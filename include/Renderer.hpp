/*
 * Project: VulkanAPI
 * File: Renderer.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "SwapChain.hpp"
#include "Perf.hpp"

 /// \brief Gestor del ciclo de renderizado.
 /// \details Administra la swapchain, los command buffers, el inicio y fin de frame
 /// y la apertura y cierre del render pass. Expone utilidades para consultar
 /// el render pass activo, la relación de aspecto y el índice de frame.
 /// Integra el subsistema de métricas de rendimiento.
class Renderer
{
public:
    /// \brief Construye el renderer asociado a una ventana y a un dispositivo Vulkan.
    /// \param window Ventana donde se presenta la imagen.
    /// \param device Dispositivo lógico Vulkan.
    Renderer(Window& window, VulkanDevice& device);

    /// \brief Libera recursos asociados y destruye la swapchain.
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /// \brief Devuelve el render pass de la swapchain.
    VkRenderPass getSwapChainRenderPass() const
    {
        return (swapChain->getRenderPass());
    }

    /// \brief Devuelve el número de imágenes de la swapchain.
    size_t getSwapChainImageCount() const
    {
        return (swapChain->imageCount());
    }

    /// \brief Devuelve la relación de aspecto del framebuffer actual.
    float getAspectRatio() const
    {
        return (swapChain->extentAspectRatio());
    }

    /// \brief Indica si hay un frame en curso.
    bool isFrameInProgress() const
    {
        return (isFrameStarted);
    }

    /// \brief Devuelve el command buffer del frame en curso.
    /// Debe llamarse entre beginFrame y endFrame.
    VkCommandBuffer getCurrentCommandBuffer() const
    {
        assert(isFrameStarted &&
            "💥[Vulkan API] Cannot get command buffer when frame not in progress");

        return (commandBuffers[currentFrameIndex]);
    }

    /// \brief Devuelve el índice del frame en curso.
    /// Debe llamarse entre beginFrame y endFrame.
    int getFrameIndex() const
    {
        assert(isFrameStarted &&
            "💥[Vulkan API] Cannot get frame index when frame not in progress");

        return (currentFrameIndex);
    }

    /// \brief Acceso al subsistema de métricas.
    Perf& getPerf()
    {
        return perf;
    }

    /// \brief Acceso constante al subsistema de métricas.
    const Perf& getPerf() const
    {
        return perf;
    }

    VkFramebuffer Renderer::getCurrentFrameBuffer() const
    {
        return swapChain->getFrameBuffer(currentFrameIndex);
    }


    /// \brief Comienza un frame. Adquiere imagen y prepara el command buffer.
    /// \return Command buffer listo para grabación o nullptr si se recreó la swapchain.
    VkCommandBuffer beginFrame();

    /// \brief Finaliza el frame. Cierra el command buffer y presenta.
    void endFrame();

    /// \brief Inicia el render pass principal sobre el command buffer indicado.
    /// \param commandBuffer Command buffer devuelto por beginFrame.
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);

    /// \brief Finaliza el render pass principal sobre el command buffer indicado.
    /// \param commandBuffer Command buffer devuelto por beginFrame.
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
    /// \brief Crea los command buffers para los frames en vuelo.
    void createCommandBuffers();

    /// \brief Libera los command buffers creados.
    void freeCommandBuffers();

    /// \brief Recrea la swapchain cuando cambia el tamaño de la ventana o queda obsoleta.
    void recreateSwapChain();

    /// Ventana asociada al renderer.
    Window& window;

    /// Dispositivo lógico Vulkan.
    VulkanDevice& vulkanDevice;

    /// Swapchain activa y recursos asociados.
    std::unique_ptr<SwapChain> swapChain;

    /// Conjunto de command buffers, uno por frame en vuelo.
    std::vector<VkCommandBuffer> commandBuffers;

    /// Índice de la imagen actual de la swapchain.
    uint32_t currentImageIndex;

    /// Índice del frame actual.
    int currentFrameIndex{0};

    /// Indica si hay un frame en curso.
    bool isFrameStarted{ false };

    /// Subsistema de métricas de rendimiento.
    Perf perf;
};

