/*
 * Project: VulkanAPI
 * File: Perf.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "Perf.hpp"
#include <algorithm>
#include <cmath> 

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

void GpuTimer::init(VkDevice dev, uint32_t framesInFlight, double tsPeriodNs) 
{
    device = dev;
    timestampPeriodNs = tsPeriodNs;

    VkQueryPoolCreateInfo ci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    ci.queryCount = framesInFlight *queriesPerFrame;
    vkCreateQueryPool(device, &ci, nullptr, &pool);
}

void GpuTimer::destroy() 
{
    if (pool) 
    {
        vkDestroyQueryPool(device, pool, nullptr);
        pool = VK_NULL_HANDLE;
    }
}

void GpuTimer::record(VkCommandBuffer cb, uint32_t frameIndex) 
{
    const uint32_t base = frameIndex * queriesPerFrame;
    vkResetQueryPool(device, pool, base, queriesPerFrame);
    vkCmdWriteTimestamp(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, base + 0);
    vkCmdWriteTimestamp(cb, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, pool, base + 1);
}

bool GpuTimer::resolve(uint32_t frameIndex, double& outGpuMs) 
{
    const uint32_t base = frameIndex * queriesPerFrame;
    uint64_t ts[2] = {};
    VkResult r = vkGetQueryPoolResults(
        device, pool, base, 2,
        sizeof(ts), ts, sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
    );

    if (r != VK_SUCCESS)
    {
        return (false);
    }

    const double dtNs = double(ts[1] - ts[0]) * timestampPeriodNs;
    outGpuMs = dtNs / 1.0e6;

    return (true);
}

static inline unsigned long long toULL(const FILETIME& ft) 
{
    ULARGE_INTEGER u; 
    u.LowPart = ft.dwLowDateTime; 
    u.HighPart = ft.dwHighDateTime; 
    
    return (u.QuadPart);
}

void CpuUsageMonitor::init() 
{
    accMs = 0.0;

#ifdef _WIN32
    SYSTEM_INFO si; GetSystemInfo(&si);
    numProcs = si.dwNumberOfProcessors ? si.dwNumberOfProcessors : 1;
#endif
}

void CpuUsageMonitor::tick(double frameCpuMs) 
{
    accMs += frameCpuMs;

    if (accMs < 250.0)
    {
        return;
    }

    accMs = 0.0;

#ifdef _WIN32
    FILETIME idleFT {}, kernelFT {}, userFT {};

    if (GetSystemTimes(&idleFT, &kernelFT, &userFT)) 
    {
        const auto idle = toULL(idleFT), kernel = toULL(kernelFT), user = toULL(userFT);
        
        if (!sysInited) 
        {
            lastIdle = idle; 
            lastKernel = kernel; 
            lastUser = user; 
            sysInited = true;
        }
        else 
        {
            const auto idleDiff = idle - lastIdle;
            const auto kernDiff = kernel - lastKernel;
            const auto userDiff = user - lastUser;
            const auto total = kernDiff + userDiff;

            sysPct = (total > 0) ? float((total - idleDiff) * 100.0 / double(total)) : 0.0f;

            lastIdle = idle; 
            lastKernel = kernel; 
            lastUser = user;
        }
    }

    FILETIME nowFT {}, cFT {}, eFT {}, kFT {}, uFT {};
    GetSystemTimeAsFileTime(&nowFT);

    if (GetProcessTimes(GetCurrentProcess(), &cFT, &eFT, &kFT, &uFT)) 
    {
        const auto now = toULL(nowFT);
        const auto k = toULL(kFT);
        const auto u = toULL(uFT);

        if (!procInited) 
        {
            lastKProc = k; 
            lastUProc = u; 
            lastNow = now; 
            procInited = true;
        }
        else 
        {
            const auto kDiff = k - lastKProc;
            const auto uDiff = u - lastUProc;
            const auto wDiff = now - lastNow;
            float pct = 0.0f;

            if (wDiff > 0)
            {
                pct = float((double(kDiff + uDiff) / double(wDiff)) * 100.0 / double(numProcs));
            }

            procPct = std::clamp(pct, 0.0f, 100.0f);
            lastKProc = k; 
            lastUProc = u; 
            lastNow = now;
        }
    }
#else
    sysPct = 0.0f;
    procPct = 0.0f;
#endif
}

#include "imgui.h"

void Perf::init(VkDevice device, uint32_t framesInFlight, double timestampPeriodNs) 
{
    statsRef = PerfStats {};
    gpuTimer.init(device, framesInFlight, timestampPeriodNs);
    cpuMonitor.init();
}

void Perf::shutdown() 
{
    gpuTimer.destroy();
}

void Perf::beginCpuFrame() 
{
    statsRef.cpuTick = std::chrono::high_resolution_clock::now();
}

void Perf::endCpuFrame() 
{
    std::chrono::high_resolution_clock::time_point now = 
        std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(now - statsRef.cpuTick).count();

    statsRef.cpuFrameMs = ms;

    if (statsRef.cpuFrameMsAvg <= 0.0)
    {
        statsRef.cpuFrameMsAvg = ms;
    }

    statsRef.cpuFrameMsAvg = 0.9 * statsRef.cpuFrameMsAvg + 0.1 * ms;

    statsRef.fps = (ms > 0.0) ? (1000.0 / ms) : 0.0;

    if (statsRef.fpsAvg <= 0.0)
    {
        statsRef.fpsAvg = statsRef.fps;
    }

    statsRef.fpsAvg = 0.9 * statsRef.fpsAvg + 0.1 * statsRef.fps;

    statsRef.fpsHistory.push(static_cast<float>(statsRef.fps));
    statsRef.cpuMsHistory.push(static_cast<float>(statsRef.cpuFrameMs));

    uiAccumMs += ms;
}

void Perf::recordGpu(VkCommandBuffer cb, uint32_t frameIndex) 
{
    gpuTimer.record(cb, frameIndex);
}

void Perf::resolveGpu(uint32_t frameIndex) 
{
    double gpuMs = 0.0;

    if (gpuTimer.resolve(frameIndex, gpuMs)) 
    {
        statsRef.gpuFrameMs = gpuMs;

        if (statsRef.gpuFrameMsAvg <= 0.0)
        {
            statsRef.gpuFrameMsAvg = gpuMs;
        }

        statsRef.gpuFrameMsAvg = 0.9 * statsRef.gpuFrameMsAvg + 0.1 * gpuMs;
        statsRef.gpuMsHistory.push(static_cast<float>(gpuMs));
    }
}

void Perf::tickMonitors() 
{
    cpuMonitor.tick(statsRef.cpuFrameMs);
    statsRef.cpuUsageSystem = cpuMonitor.systemPercent();
    statsRef.cpuUsageProcess = cpuMonitor.processPercent();

    if (uiAccumMs >= static_cast<double>(uiPeriodMs)) 
    {
        uiAccumMs = 0.0;

        dispFps = std::round(statsRef.fps);
        dispFpsAvg = std::round(statsRef.fpsAvg);
        dispCpuMs = std::round(statsRef.cpuFrameMs * 100.0) / 100.0;
        dispCpuMsAvg = std::round(statsRef.cpuFrameMsAvg * 100.0) / 100.0;
        dispGpuMs = std::round(statsRef.gpuFrameMs * 100.0) / 100.0;
        dispGpuMsAvg = std::round(statsRef.gpuFrameMsAvg * 100.0) / 100.0;
        dispCpuSys = std::round(statsRef.cpuUsageSystem * 10.0f) / 10.0f;
        dispCpuProc = std::round(statsRef.cpuUsageProcess * 10.0f) / 10.0f;
    }
}

void Perf::drawImGui(bool* pOpen) {
    if (ImGui::Begin("Performance", pOpen, ImGuiWindowFlags_AlwaysAutoResize)) 
    {
        ImGui::Text("FPS: %.0f  (avg %.0f)", dispFps, dispFpsAvg);
        ImGui::Text("CPU frame: %.2f ms   (avg %.2f ms)", dispCpuMs, dispCpuMsAvg);
        ImGui::Text("GPU frame: %.2f ms   (avg %.2f ms)", dispGpuMs, dispGpuMsAvg);
        ImGui::Text("CPU usage: system %.1f%%   process %.1f%%", dispCpuSys, dispCpuProc);

        ImGui::Separator();
        ImGui::PlotLines("FPS", statsRef.fpsHistory.raw(), statsRef.fpsHistory.size(),
            statsRef.fpsHistory.size() - 1, nullptr, 0.0f, 240.0f, ImVec2(300, 80));
        ImGui::PlotLines("CPU ms", statsRef.cpuMsHistory.raw(), statsRef.cpuMsHistory.size(),
            statsRef.cpuMsHistory.size() - 1, nullptr, 0.0f, 33.0f, ImVec2(300, 80));
        ImGui::PlotLines("GPU ms", statsRef.gpuMsHistory.raw(), statsRef.gpuMsHistory.size(),
            statsRef.gpuMsHistory.size() - 1, nullptr, 0.0f, 33.0f, ImVec2(300, 80));

        ImGui::Separator();
        ImGui::TextDisabled("UI refresh: %d ms (suavizado EMA 0.1).", uiPeriodMs);
        ImGui::SliderInt("UI period (ms)", &uiPeriodMs, 100, 1000);
    }
    ImGui::End();
}
