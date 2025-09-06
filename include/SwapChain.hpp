/*
 * Project: VulkanAPI
 * File: SwapChain.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <assert.h>
#include <memory>
#include <string>
#include <vector>

 /// \brief Encapsula la gestión de la swapchain y sus recursos asociados.
 /// \details Crea y administra la cadena de intercambio, sus imágenes y vistas,
 /// el render pass, los framebuffers y los objetos de sincronización. Expone
 /// utilidades para adquirir la siguiente imagen y presentar el frame, así como
 /// getters de formato y dimensiones. Soporta recreación segura preservando
 /// formatos entre versiones.
class SwapChain
{
public:
    /// \brief Número máximo de frames en vuelo.
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    /// \brief Construye la swapchain y los recursos asociados.
    /// \param deviceRef Dispositivo lógico y físico de Vulkan.
    /// \param windowExtent Extensión del framebuffer de la ventana.
    SwapChain(VulkanDevice& deviceRef, VkExtent2D windowExtent);

    /// \brief Construye una nueva swapchain a partir de otra anterior.
    /// \param deviceRef Dispositivo lógico y físico de Vulkan.
    /// \param windowExtent Extensión del framebuffer de la ventana.
    /// \param previous Puntero compartido a la swapchain anterior para migrar recursos.
    SwapChain(
        VulkanDevice& deviceRef,
        VkExtent2D windowExtent,
        std::shared_ptr<SwapChain> previous);

    /// \brief Libera todos los recursos propiedad de la swapchain.
    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    /// \brief Devuelve el framebuffer asociado a una imagen de la swapchain.
    /// \param index Índice de imagen.
    /// \return Framebuffer para el índice solicitado.
    VkFramebuffer getFrameBuffer(int index)
    {
        return (swapChainFramebuffers[index]);
    }

    /// \brief Devuelve el render pass usado por la swapchain.
    VkRenderPass getRenderPass()
    {
        return (renderPass);
    }

    /// \brief Devuelve la vista de imagen de la swapchain.
    /// \param index Índice de imagen.
    /// \return \c VkImageView correspondiente.
    VkImageView getImageView(int index)
    {
        return (swapChainImageViews[index]);
    }

    /// \brief Número de imágenes en la swapchain.
    size_t imageCount()
    {
        return (swapChainImages.size());
    }

    /// \brief Formato de las imágenes de color de la swapchain.
    VkFormat getSwapChainImageFormat()
    {
        return (swapChainImageFormat);
    }

    /// \brief Extensión actual del área de presentación.
    VkExtent2D getSwapChainExtent()
    {
        return (swapChainExtent);
    }

    /// \brief Anchura de la swapchain en píxeles.
    uint32_t width()
    {
        return (swapChainExtent.width);
    }

    /// \brief Altura de la swapchain en píxeles.
    uint32_t height()
    {
        return (swapChainExtent.height);
    }

    /// \brief Relación de aspecto ancho entre alto.
    float extentAspectRatio()
    {
        return
            (static_cast<float>(swapChainExtent.width) /
                static_cast<float>(swapChainExtent.height));
    }

    /// \brief Busca un formato de profundidad compatible con el dispositivo.
    /// \return Formato de profundidad seleccionado.
    VkFormat findDepthFormat();

    /// \brief Adquiere el índice de la siguiente imagen disponible.
    /// \param imageIndex Salida con el índice de imagen adquirido.
    /// \return Resultado de la operación de adquisición.
    VkResult acquireNextImage(uint32_t* imageIndex);

    /// \brief Envía los command buffers para su ejecución y presenta la imagen.
    /// \param buffers Puntero al command buffer grabado del frame.
    /// \param imageIndex Índice de la imagen que se va a presentar.
    /// \return Resultado de la operación de presentación.
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

    /// \brief Compara los formatos de color y profundidad con otra swapchain.
    /// \param swapChain Otra instancia contra la que se compara.
    /// \return \c true si ambos formatos coinciden.
    bool compareSwapFormats(const SwapChain& swapChain) const
    {
        return
            ((swapChain.swapChainDepthFormat == swapChainDepthFormat) &&
                (swapChain.swapChainImageFormat == swapChainImageFormat));
    }

private:
    /// \brief Inicializa la cadena de intercambio y los recursos derivados.
    void init();

    /// \brief Crea la \c VkSwapchainKHR según las capacidades de la superficie.
    void createSwapChain();

    /// \brief Crea las vistas de imagen para cada imagen de la swapchain.
    void createImageViews();

    /// \brief Crea las imágenes y vistas de profundidad.
    void createDepthResources();

    /// \brief Crea el render pass principal de presentación.
    void createRenderPass();

    /// \brief Crea un framebuffer por cada imagen de la swapchain.
    void createFramebuffers();

    /// \brief Crea semáforos y fences para la sincronización por frame.
    void createSyncObjects();

    /// \brief Selecciona formato de superficie entre los disponibles.
    /// \param availableFormats Lista de formatos soportados por la superficie.
    /// \return Formato elegido para color.
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /// \brief Selecciona el modo de presentación entre los disponibles.
    /// \param availablePresentModes Lista de modos soportados.
    /// \return Modo de presentación elegido.
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);

    /// \brief Determina la extensión final de la swapchain.
    /// \param capabilities Capacidades de la superficie de presentación.
    /// \return Extensión elegida.
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    /// Formato de color de la swapchain.
    VkFormat swapChainImageFormat;

    /// Formato del buffer de profundidad.
    VkFormat swapChainDepthFormat;

    /// Extensión actual de la swapchain.
    VkExtent2D swapChainExtent;

    /// Framebuffers creados para cada imagen de la swapchain.
    std::vector<VkFramebuffer> swapChainFramebuffers;

    /// Render pass principal de presentación.
    VkRenderPass renderPass;

    /// Imágenes de profundidad por imagen de la swapchain.
    std::vector<VkImage> depthImages;

    /// Memorias asociadas a las imágenes de profundidad.
    std::vector<VkDeviceMemory> depthImageMemorys;

    /// Vistas de las imágenes de profundidad.
    std::vector<VkImageView> depthImageViews;

    /// Imágenes de color de la swapchain.
    std::vector<VkImage> swapChainImages;

    /// Vistas de las imágenes de color de la swapchain.
    std::vector<VkImageView> swapChainImageViews;

    /// Referencia al dispositivo Vulkan.
    VulkanDevice& device;

    /// Extensión objetivo basada en el tamaño de la ventana.
    VkExtent2D windowExtent;

    /// Objeto swapchain de Vulkan.
    VkSwapchainKHR swapChain;

    /// Puntero a la swapchain previa para recreaciones.
    std::shared_ptr<SwapChain> oldSwapChain;

    /// Semáforos que indican imagen disponible por frame.
    std::vector<VkSemaphore> imageAvailableSemaphores;

    /// Semáforos que indican render finalizado por frame.
    std::vector<VkSemaphore> renderFinishedSemaphores;

    /// Fences para sincronización CPU GPU por frame en vuelo.
    std::vector<VkFence> inFlightFences;

    /// Fences asociadas a cada imagen de la swapchain.
    std::vector<VkFence> imagesInFlight;

    /// Índice del frame actual en vuelo.
    size_t currentFrame = 0;
};

