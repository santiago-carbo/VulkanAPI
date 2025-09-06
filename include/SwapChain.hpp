/*
 * Project: VulkanAPI
 * File: SwapChain.hpp
 * Author: Santiago Carb� Garc�a
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

 /// \brief Encapsula la gesti�n de la swapchain y sus recursos asociados.
 /// \details Crea y administra la cadena de intercambio, sus im�genes y vistas,
 /// el render pass, los framebuffers y los objetos de sincronizaci�n. Expone
 /// utilidades para adquirir la siguiente imagen y presentar el frame, as� como
 /// getters de formato y dimensiones. Soporta recreaci�n segura preservando
 /// formatos entre versiones.
class SwapChain
{
public:
    /// \brief N�mero m�ximo de frames en vuelo.
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    /// \brief Construye la swapchain y los recursos asociados.
    /// \param deviceRef Dispositivo l�gico y f�sico de Vulkan.
    /// \param windowExtent Extensi�n del framebuffer de la ventana.
    SwapChain(VulkanDevice& deviceRef, VkExtent2D windowExtent);

    /// \brief Construye una nueva swapchain a partir de otra anterior.
    /// \param deviceRef Dispositivo l�gico y f�sico de Vulkan.
    /// \param windowExtent Extensi�n del framebuffer de la ventana.
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
    /// \param index �ndice de imagen.
    /// \return Framebuffer para el �ndice solicitado.
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
    /// \param index �ndice de imagen.
    /// \return \c VkImageView correspondiente.
    VkImageView getImageView(int index)
    {
        return (swapChainImageViews[index]);
    }

    /// \brief N�mero de im�genes en la swapchain.
    size_t imageCount()
    {
        return (swapChainImages.size());
    }

    /// \brief Formato de las im�genes de color de la swapchain.
    VkFormat getSwapChainImageFormat()
    {
        return (swapChainImageFormat);
    }

    /// \brief Extensi�n actual del �rea de presentaci�n.
    VkExtent2D getSwapChainExtent()
    {
        return (swapChainExtent);
    }

    /// \brief Anchura de la swapchain en p�xeles.
    uint32_t width()
    {
        return (swapChainExtent.width);
    }

    /// \brief Altura de la swapchain en p�xeles.
    uint32_t height()
    {
        return (swapChainExtent.height);
    }

    /// \brief Relaci�n de aspecto ancho entre alto.
    float extentAspectRatio()
    {
        return
            (static_cast<float>(swapChainExtent.width) /
                static_cast<float>(swapChainExtent.height));
    }

    /// \brief Busca un formato de profundidad compatible con el dispositivo.
    /// \return Formato de profundidad seleccionado.
    VkFormat findDepthFormat();

    /// \brief Adquiere el �ndice de la siguiente imagen disponible.
    /// \param imageIndex Salida con el �ndice de imagen adquirido.
    /// \return Resultado de la operaci�n de adquisici�n.
    VkResult acquireNextImage(uint32_t* imageIndex);

    /// \brief Env�a los command buffers para su ejecuci�n y presenta la imagen.
    /// \param buffers Puntero al command buffer grabado del frame.
    /// \param imageIndex �ndice de la imagen que se va a presentar.
    /// \return Resultado de la operaci�n de presentaci�n.
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

    /// \brief Crea la \c VkSwapchainKHR seg�n las capacidades de la superficie.
    void createSwapChain();

    /// \brief Crea las vistas de imagen para cada imagen de la swapchain.
    void createImageViews();

    /// \brief Crea las im�genes y vistas de profundidad.
    void createDepthResources();

    /// \brief Crea el render pass principal de presentaci�n.
    void createRenderPass();

    /// \brief Crea un framebuffer por cada imagen de la swapchain.
    void createFramebuffers();

    /// \brief Crea sem�foros y fences para la sincronizaci�n por frame.
    void createSyncObjects();

    /// \brief Selecciona formato de superficie entre los disponibles.
    /// \param availableFormats Lista de formatos soportados por la superficie.
    /// \return Formato elegido para color.
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /// \brief Selecciona el modo de presentaci�n entre los disponibles.
    /// \param availablePresentModes Lista de modos soportados.
    /// \return Modo de presentaci�n elegido.
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);

    /// \brief Determina la extensi�n final de la swapchain.
    /// \param capabilities Capacidades de la superficie de presentaci�n.
    /// \return Extensi�n elegida.
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    /// Formato de color de la swapchain.
    VkFormat swapChainImageFormat;

    /// Formato del buffer de profundidad.
    VkFormat swapChainDepthFormat;

    /// Extensi�n actual de la swapchain.
    VkExtent2D swapChainExtent;

    /// Framebuffers creados para cada imagen de la swapchain.
    std::vector<VkFramebuffer> swapChainFramebuffers;

    /// Render pass principal de presentaci�n.
    VkRenderPass renderPass;

    /// Im�genes de profundidad por imagen de la swapchain.
    std::vector<VkImage> depthImages;

    /// Memorias asociadas a las im�genes de profundidad.
    std::vector<VkDeviceMemory> depthImageMemorys;

    /// Vistas de las im�genes de profundidad.
    std::vector<VkImageView> depthImageViews;

    /// Im�genes de color de la swapchain.
    std::vector<VkImage> swapChainImages;

    /// Vistas de las im�genes de color de la swapchain.
    std::vector<VkImageView> swapChainImageViews;

    /// Referencia al dispositivo Vulkan.
    VulkanDevice& device;

    /// Extensi�n objetivo basada en el tama�o de la ventana.
    VkExtent2D windowExtent;

    /// Objeto swapchain de Vulkan.
    VkSwapchainKHR swapChain;

    /// Puntero a la swapchain previa para recreaciones.
    std::shared_ptr<SwapChain> oldSwapChain;

    /// Sem�foros que indican imagen disponible por frame.
    std::vector<VkSemaphore> imageAvailableSemaphores;

    /// Sem�foros que indican render finalizado por frame.
    std::vector<VkSemaphore> renderFinishedSemaphores;

    /// Fences para sincronizaci�n CPU GPU por frame en vuelo.
    std::vector<VkFence> inFlightFences;

    /// Fences asociadas a cada imagen de la swapchain.
    std::vector<VkFence> imagesInFlight;

    /// �ndice del frame actual en vuelo.
    size_t currentFrame = 0;
};

