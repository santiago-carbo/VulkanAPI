#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <chrono>

struct PerfRing 
{
    static constexpr size_t Count = 240;
    std::array<float, Count> data{};
    size_t head = 0;

    void push(float v) 
    { 
        data[head] = v; head = (head + 1) % Count; 
    }

    const float* raw() const 
    { 
        return data.data(); 
    }

    int size() const 
    { 
        return static_cast<int>(Count); 
    }
};

struct PerfStats 
{
    double cpuFrameMs = 0.0;
    double fps = 0.0;
    double fpsAvg = 0.0;
    float  cpuUsageSystem = 0.0f;
    float  cpuUsageProcess = 0.0f;
    double cpuFrameMsAvg = 0.0;
    double gpuFrameMs = 0.0;
    double gpuFrameMsAvg = 0.0;

    PerfRing fpsHistory;
    PerfRing cpuMsHistory;
    PerfRing gpuMsHistory;

    std::chrono::high_resolution_clock::time_point cpuTick{};
};

class GpuTimer 
{
    public:
        void init(VkDevice device, uint32_t framesInFlight, double timestampPeriodNs);
        void destroy();
        void record(VkCommandBuffer cb, uint32_t frameIndex);
        bool resolve(uint32_t frameIndex, double& outGpuMs);

    private:
        VkDevice device = VK_NULL_HANDLE;
        VkQueryPool pool = VK_NULL_HANDLE;
        uint32_t queriesPerFrame = 2;
        double timestampPeriodNs = 1.0;
};

class CpuUsageMonitor 
{
    public:
        void init();
        void tick(double frameCpuMs);

        float systemPercent()  const 
        { 
            return sysPct; 
        }

        float processPercent() const 
        { 
            return procPct; 
        }

    private:
        double accMs = 0.0;
        float sysPct = 0.0f;
        float procPct = 0.0f;

#ifdef _WIN32
    unsigned long long lastIdle = 0, lastKernel = 0, lastUser = 0;
    bool sysInited = false;
    unsigned long long lastKProc = 0, lastUProc = 0, lastNow = 0;
    unsigned long numProcs = 1;
    bool procInited = false;
#endif
};

class Perf 
{
    public:
        void init(VkDevice device, uint32_t framesInFlight, double timestampPeriodNs);
        void shutdown();

        void beginCpuFrame();
        void endCpuFrame();
        void recordGpu(VkCommandBuffer cb, uint32_t frameIndex);
        void resolveGpu(uint32_t frameIndex);
        void tickMonitors();

        const PerfStats& stats() const { return statsRef; }

        void drawImGui(bool* pOpen = nullptr);

    private:
        PerfStats statsRef;
        GpuTimer  gpuTimer;
        CpuUsageMonitor cpuMonitor;

        double uiAccumMs = 0.0;
        int uiPeriodMs = 300;

        double dispFps = 0.0;
        double dispFpsAvg = 0.0;
        double dispCpuMs = 0.0;
        double dispCpuMsAvg = 0.0;
        double dispGpuMs = 0.0;
        double dispGpuMsAvg = 0.0;
        float  dispCpuSys = 0.0f;
        float  dispCpuProc = 0.0f;
};
