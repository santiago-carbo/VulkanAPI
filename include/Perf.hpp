/*
 * Project: VulkanAPI
 * File: Perf.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <vulkan/vulkan.h>
#include <array>
#include <chrono>

 /// \brief Buffer circular de valores flotantes para series temporales (FPS/ms).
 /// \details Mantiene una ventana fija de \c Count muestras y permite añadir
 /// nuevas con \c push(). El índice \c head rota de forma circular.
struct PerfRing
{
    ///< Capacidad fija del anillo.
    static constexpr size_t Count = 240;
    /// Almacenamiento de muestras.
    std::array<float, Count> data {};
    /// Próxima posición de escritura.
    size_t head = 0;

    /// \brief Inserta una nueva muestra y avanza el puntero circular.
    /// \param v Valor a registrar.
    void push(float v)
    {
        data[head] = v;
        head = (head + 1) % Count;
    }

    /// \brief Acceso directo al bloque contiguo para graficado (ImGui).
    /// \return Puntero al primer elemento del array subyacente.
    const float* raw() const
    {
        return data.data();
    }

    /// \brief Devuelve el tamaño lógico del anillo (siempre \c Count).
    /// \return Número de elementos del anillo.
    int size() const
    {
        return static_cast<int>(Count);
    }
};

/// \brief Conjunto de métricas de rendimiento acumuladas y en vivo.
/// \details Incluye tiempos de frame CPU/GPU (ms), FPS medio, uso de CPU
/// y anillos históricos para graficado.
struct PerfStats
{
    /// Duración del frame en CPU (ms) del último ciclo.
    double cpuFrameMs = 0.0;
    /// FPS instantáneo estimado.
    double fps = 0.0;
    /// FPS suavizado/promedio para visualización.
    double fpsAvg = 0.0;
    /// % CPU del sistema.
    float  cpuUsageSystem = 0.0f;
    /// % CPU del proceso actual.
    float  cpuUsageProcess = 0.0f;
    /// Promedio suavizado de ms/frame CPU.
    double cpuFrameMsAvg = 0.0;
    /// Duración del frame en GPU (ms) del último ciclo resuelto.
    double gpuFrameMs = 0.0;
    /// Promedio suavizado de ms/frame GPU.
    double gpuFrameMsAvg = 0.0;

    /// Serie temporal de FPS.
    PerfRing fpsHistory;
    /// Serie temporal de ms CPU por frame.
    PerfRing cpuMsHistory;
    /// Serie temporal de ms GPU por frame.
    PerfRing gpuMsHistory;

    /// Inicio del frame en CPU.
    std::chrono::high_resolution_clock::time_point cpuTick {};
};

/// \brief Temporizador de GPU basado en consultas de marca de tiempo.
/// \details Gestiona un \c VkQueryPool y emite pares de timestamps por frame.
/// Con \c resolve() convierte ticks a milisegundos usando \c timestampPeriodNs.
class GpuTimer
{
public:
    /// \brief Inicializa el temporizador de GPU.
    /// \param device Dispositivo lógico.
    /// \param framesInFlight Nº de frames en vuelo.
    /// \param timestampPeriodNs Periodo de timestamp del dispositivo.
    void init(VkDevice device, uint32_t framesInFlight, double timestampPeriodNs);

    /// \brief Libera el \c VkQueryPool y recursos asociados.
    void destroy();

    /// \brief Registra timestamps de inicio y fin en el \c commandBuffer.
    /// \param cb Command buffer donde insertar las consultas.
    /// \param frameIndex Índice de frame en vuelo.
    void record(VkCommandBuffer cb, uint32_t frameIndex);

    /// \brief Lee y resuelve las consultas de \c frameIndex a milisegundos.
    /// \param frameIndex Índice de frame en vuelo a resolver.
    /// \param outGpuMs Salida: duración en ms.
    /// \return \c true si los resultados estaban listos y \c outGpuMs es válido.
    bool resolve(uint32_t frameIndex, double& outGpuMs);

private:
    /// Dispositivo lógico.
    VkDevice device = VK_NULL_HANDLE;
    /// Pool de consultas de timestamps.
    VkQueryPool pool = VK_NULL_HANDLE;
    /// Dos por frame (inicio/fin).
    uint32_t queriesPerFrame = 2;
    /// Conversión de ticks a nanosegundos.
    double timestampPeriodNs = 1.0;
};

/// \brief Monitoriza el uso de CPU del sistema y del proceso actual.
/// \details En Windows utiliza \c GetSystemTimes y \c GetProcessTimes; en otras
/// plataformas puede quedar como no operativo según la implementación.
class CpuUsageMonitor
{
public:
    /// \brief Inicializa el monitor.
    void init();

    /// \brief Acumula \c frameCpuMs y actualiza los porcentajes de uso periódicamente.
    /// \param frameCpuMs Duración del frame en CPU (ms) para temporización de muestreo.
    void tick(double frameCpuMs);

    /// \brief Porcentaje de uso de CPU del sistema.
    float systemPercent()  const
    {
        return sysPct;
    }

    /// \brief Porcentaje de uso de CPU del proceso actual.
    float processPercent() const
    {
        return procPct;
    }

private:
    /// Acumulador para decidir cuándo refrescar medición.
    double accMs = 0.0;
    /// Última lectura de % CPU del sistema.
    float sysPct = 0.0f;
    /// Última lectura de % CPU del proceso.
    float procPct = 0.0f;

#ifdef _WIN32
    /// Estado interno para cálculo de % CPU en Windows
    unsigned long long lastIdle = 0, lastKernel = 0, lastUser = 0;
    bool sysInited = false;
    unsigned long long lastKProc = 0, lastUProc = 0, lastNow = 0;
    unsigned long numProcs = 1;
    bool procInited = false;
#endif
};

/// \brief Facade de rendimiento: mide CPU/GPU, mantiene históricos y dibuja overlay ImGui.
/// \details Orquesta \c GpuTimer y \c CpuUsageMonitor, expone \c stats() para
/// consulta (lectura) y \c drawImGui() para representar panel de rendimiento.
class Perf
{
public:
    /// \brief Inicializa el subsistema de rendimiento.
    /// \param device Dispositivo lógico Vulkan.
    /// \param framesInFlight Nº de frames en vuelo.
    /// \param timestampPeriodNs Periodo de timestamp del dispositivo.
    void init(VkDevice device, uint32_t framesInFlight, double timestampPeriodNs);

    /// \brief Libera recursos asociados.
    void shutdown();

    /// \brief Marca el inicio del frame en CPU.
    void beginCpuFrame();

    /// \brief Marca el fin del frame en CPU y actualiza métricas instantáneas.
    void endCpuFrame();

    /// \brief Inserta consultas GPU en el \c commandBuffer del frame.
    /// \param cb Command buffer actual.
    /// \param frameIndex Índice de frame en vuelo.
    void recordGpu(VkCommandBuffer cb, uint32_t frameIndex);

    /// \brief Resuelve las consultas GPU del frame y actualiza medias/históricos.
    /// \param frameIndex Índice de frame en vuelo.
    void resolveGpu(uint32_t frameIndex);

    /// \brief Actualiza monitores según periodo de muestreo.
    void tickMonitors();

    /// \brief Acceso de sólo lectura a las estadísticas actuales.
    /// \return Referencia constante a \c PerfStats interno.
    const PerfStats& stats() const 
    { 
        return statsRef; 
    }

    /// \brief Dibuja el panel de rendimiento en ImGui.
    /// \param pOpen Puntero opcional a flag de visibilidad del panel.
    void drawImGui(bool* pOpen = nullptr);

private:
    /// Métricas en vivo e históricos.
    PerfStats statsRef;
    /// Temporizador de GPU por timestamps.
    GpuTimer  gpuTimer;
    /// Monitor de uso de CPU.
    CpuUsageMonitor cpuMonitor;

    /// Acumulador para decidir cuándo refrescar números mostrados.
    double uiAccumMs = 0.0;
    /// Periodo de refresco de valores mostrados (ms).
    int uiPeriodMs = 300;

    /// Copias suavizadas para mostrar en UI y evitar parpadeos
    double dispFps = 0.0;
    double dispFpsAvg = 0.0;
    double dispCpuMs = 0.0;
    double dispCpuMsAvg = 0.0;
    double dispGpuMs = 0.0;
    double dispGpuMsAvg = 0.0;
    float  dispCpuSys = 0.0f;
    float  dispCpuProc = 0.0f;
};
