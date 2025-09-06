/*
 * Project: VulkanAPI
 * File: VulkanDevice.hpp
 * Author: Santiago Carb� Garc�a
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "Window.hpp"

#include <vector>

 /// \brief Capacidades y formatos de la swapchain para un dispositivo f�sico.
 /// \details Recoge l�mites de la superficie, formatos soportados y modos de presentaci�n.
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/// \brief �ndices de familias de colas relevantes para la aplicaci�n.
/// \details Se almacenan los �ndices de cola de gr�ficos y de presentaci�n 
/// y si han sido localizados.
struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool hasGraphicsFamily = false;
    bool hasPresentFamily = false;

    /// \brief Devuelve el �ndice de la familia de colas de gr�ficos.
    uint32_t GetGraphicsFamily() const
    {
        return graphicsFamily;
    }

    /// \brief Devuelve el �ndice de la familia de colas de presentaci�n.
    uint32_t GetPresentFamily() const
    {
        return presentFamily;
    }

    /// \brief Indica si se han encontrado ambas familias necesarias.
    bool isComplete() const
    {
        return hasGraphicsFamily && hasPresentFamily;
    }
};

/// \brief Encapsula la creaci�n y gesti�n del dispositivo Vulkan y recursos asociados.
/// \details Responsable de la instancia, surface, selecci�n de dispositivo f�sico,
/// dispositivo l�gico, colas, command pool y utilidades de creaci�n y copia de recursos.
class VulkanDevice
{
public:
#ifdef NDEBUG
    /// \brief Controla la activaci�n de validation layers seg�n modo de compilaci�n.
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    /// \brief Construye el dispositivo asociado a una ventana y crea los recursos base.
    /// \param window Referencia a la ventana usada para crear la surface.
    VulkanDevice(Window& window);

    /// \brief Libera recursos del dispositivo y objetos dependientes.
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
    VulkanDevice(VulkanDevice&&) = delete;
    VulkanDevice& operator=(VulkanDevice&&) = delete;

    /// \brief Devuelve la instancia de Vulkan.
    VkInstance getInstance() const
    {
        return instance;
    }

    /// \brief Devuelve el command pool general para comandos de uso frecuente.
    VkCommandPool getCommandPool() const
    {
        return commandPool;
    }

    /// \brief Devuelve el dispositivo f�sico seleccionado.
    VkPhysicalDevice getPhysicalDevice() const
    {
        return physicalDevice;
    }

    /// \brief Devuelve el dispositivo l�gico.
    VkDevice getDevice() const
    {
        return logicalDevice;
    }

    /// \brief Devuelve la surface asociada a la ventana.
    VkSurfaceKHR getSurface() const
    {
        return surface;
    }

    /// \brief Devuelve la cola de gr�ficos.
    VkQueue getGraphicsQueue() const
    {
        return graphicsQueue;
    }

    /// \brief Devuelve la cola de presentaci�n.
    VkQueue getPresentQueue() const
    {
        return presentQueue;
    }

    /// \brief Consulta y devuelve el soporte de swapchain del dispositivo f�sico actual.
    SwapChainSupportDetails getSwapChainSupportDetails() const
    {
        return querySwapChainSupport(physicalDevice);
    }

    /// \brief Busca un tipo de memoria de dispositivo que cumpla las propiedades requeridas.
    /// \param typeFilter M�scara de tipos aceptables.
    /// \param properties Propiedades de memoria requeridas.
    /// \return �ndice de tipo de memoria v�lido.
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    /// \brief Devuelve los �ndices de familias de colas relevantes
    /// para el dispositivo f�sico actual.
    QueueFamilyIndices getQueueFamilyIndices() const
    {
        return findQueueFamilies(physicalDevice);
    }

    /// \brief Elige un formato soportado a partir de candidatos y caracter�sticas requeridas.
    /// \param candidates Lista de formatos candidatos.
    /// \param tiling Tipeado de imagen requerido.
    /// \param features Conjunto de caracter�sticas requeridas.
    /// \return Formato compatible.
    VkFormat findSupportedFormat(
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features) const;

    /// \brief Crea un VkBuffer y asigna su memoria en el dispositivo.
    /// \param size Tama�o del b�fer.
    /// \param usage Flags de uso del b�fer.
    /// \param properties Propiedades de la memoria requerida.
    /// \param buffer Salida con el manejador del b�fer.
    /// \param bufferMemory Salida con la memoria asignada.
    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    /// \brief Comienza un comando de un solo uso en un command buffer temporal.
    /// \return Command buffer listo para grabar.
    VkCommandBuffer beginSingleUseCommands();

    /// \brief Finaliza y env�a el command buffer de un solo uso y libera recursos.
    /// \param commandBuffer El command buffer temporal a finalizar.
    void endSingleUseCommands(VkCommandBuffer commandBuffer);

    /// \brief Copia datos entre b�feres con un comando inmediato.
    /// \param srcBuffer Origen.
    /// \param dstBuffer Destino.
    /// \param size Tama�o de la copia.
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /// \brief Copia datos desde un b�fer a una imagen.
    /// \param buffer B�fer origen.
    /// \param image Imagen destino.
    /// \param width Anchura de la regi�n a copiar.
    /// \param height Altura de la regi�n a copiar.
    /// \param layerCount N�mero de capas de la imagen a copiar.
    void copyBufferToImage(
        VkBuffer buffer,
        VkImage image,
        uint32_t width,
        uint32_t height,
        uint32_t layerCount);

    /// \brief Crea una imagen con los par�metros indicados y asigna su memoria.
    /// \param imageInfo Estructura de creaci�n de imagen.
    /// \param properties Propiedades de la memoria requerida.
    /// \param image Salida con la imagen creada.
    /// \param imageMemory Salida con la memoria asignada.
    void createImageWithInfo(
        const VkImageCreateInfo& imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory);

    /// \brief Propiedades del dispositivo f�sico seleccionado.
    VkPhysicalDeviceProperties deviceProperties;

private:
    /// \brief Crea la instancia de Vulkan con las extensiones y capas requeridas.
    void createInstance();

    /// \brief Inicializa el mensajero de depuraci�n si est� habilitado.
    void setupDebugMessenger();

    /// \brief Crea la surface asociada a la ventana.
    void createSurface();

    /// \brief Selecciona el dispositivo f�sico adecuado.
    void selectPhysicalDevice();

    /// \brief Crea el dispositivo l�gico y obtiene colas de gr�ficos y presentaci�n.
    void createLogicalDevice();

    /// \brief Crea el command pool principal.
    void createCommandPool();

    /// \brief Verifica si un dispositivo f�sico cumple los requisitos.
    /// \param device Dispositivo f�sico candidato.
    /// \return Verdadero si cumple.
    bool isDeviceSuitable(VkPhysicalDevice device) const;

    /// \brief Extensiones requeridas por la plataforma.
    std::vector<const char*> getRequiredExtensions() const;

    /// \brief Comprueba si los validation layers solicitados est�n disponibles.
    bool checkValidationLayerSupport() const;

    /// \brief Busca familias de colas necesarias en un dispositivo f�sico.
    /// \param device Dispositivo f�sico candidato.
    /// \return Estructura con los �ndices localizados.
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

    /// \brief Rellena la estructura de creaci�n del mensajero de depuraci�n.
    /// \param createInfo Estructura a completar.
    void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;

    /// \brief Verifica que las extensiones requeridas por GLFW est�n disponibles.
    void verifyGLFWRequiredExtensions() const;

    /// \brief Comprueba soporte de extensiones de dispositivo.
    /// \param device Dispositivo f�sico candidato.
    /// \return Verdadero si se soportan.
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

    /// \brief Consulta soporte de swapchain para un dispositivo f�sico.
    /// \param device Dispositivo f�sico a consultar.
    /// \return Detalles de capacidades, formatos y modos.
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    /// Instancia de Vulkan.
    VkInstance instance;

    /// Mensajero de depuraci�n para callback de validaci�n.
    VkDebugUtilsMessengerEXT debugMessenger;

    /// Dispositivo f�sico seleccionado.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    /// Ventana asociada usada para crear la surface.
    Window& window;

    /// Command pool principal para asignar command buffers.
    VkCommandPool commandPool;

    /// Dispositivo l�gico.
    VkDevice logicalDevice;

    /// Superficie de presentaci�n.
    VkSurfaceKHR surface;

    /// Cola de gr�ficos.
    VkQueue graphicsQueue;

    /// Cola de presentaci�n.
    VkQueue presentQueue;

    /// Lista de validation layers solicitados.
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    /// Extensiones de dispositivo requeridas.
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
