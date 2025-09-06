/*
 * Project: VulkanAPI
 * File: VulkanDevice.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "Window.hpp"

#include <vector>

 /// \brief Capacidades y formatos de la swapchain para un dispositivo físico.
 /// \details Recoge límites de la superficie, formatos soportados y modos de presentación.
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/// \brief Índices de familias de colas relevantes para la aplicación.
/// \details Se almacenan los índices de cola de gráficos y de presentación 
/// y si han sido localizados.
struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool hasGraphicsFamily = false;
    bool hasPresentFamily = false;

    /// \brief Devuelve el índice de la familia de colas de gráficos.
    uint32_t GetGraphicsFamily() const
    {
        return graphicsFamily;
    }

    /// \brief Devuelve el índice de la familia de colas de presentación.
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

/// \brief Encapsula la creación y gestión del dispositivo Vulkan y recursos asociados.
/// \details Responsable de la instancia, surface, selección de dispositivo físico,
/// dispositivo lógico, colas, command pool y utilidades de creación y copia de recursos.
class VulkanDevice
{
public:
#ifdef NDEBUG
    /// \brief Controla la activación de validation layers según modo de compilación.
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

    /// \brief Devuelve el dispositivo físico seleccionado.
    VkPhysicalDevice getPhysicalDevice() const
    {
        return physicalDevice;
    }

    /// \brief Devuelve el dispositivo lógico.
    VkDevice getDevice() const
    {
        return logicalDevice;
    }

    /// \brief Devuelve la surface asociada a la ventana.
    VkSurfaceKHR getSurface() const
    {
        return surface;
    }

    /// \brief Devuelve la cola de gráficos.
    VkQueue getGraphicsQueue() const
    {
        return graphicsQueue;
    }

    /// \brief Devuelve la cola de presentación.
    VkQueue getPresentQueue() const
    {
        return presentQueue;
    }

    /// \brief Consulta y devuelve el soporte de swapchain del dispositivo físico actual.
    SwapChainSupportDetails getSwapChainSupportDetails() const
    {
        return querySwapChainSupport(physicalDevice);
    }

    /// \brief Busca un tipo de memoria de dispositivo que cumpla las propiedades requeridas.
    /// \param typeFilter Máscara de tipos aceptables.
    /// \param properties Propiedades de memoria requeridas.
    /// \return Índice de tipo de memoria válido.
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    /// \brief Devuelve los índices de familias de colas relevantes
    /// para el dispositivo físico actual.
    QueueFamilyIndices getQueueFamilyIndices() const
    {
        return findQueueFamilies(physicalDevice);
    }

    /// \brief Elige un formato soportado a partir de candidatos y características requeridas.
    /// \param candidates Lista de formatos candidatos.
    /// \param tiling Tipeado de imagen requerido.
    /// \param features Conjunto de características requeridas.
    /// \return Formato compatible.
    VkFormat findSupportedFormat(
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features) const;

    /// \brief Crea un VkBuffer y asigna su memoria en el dispositivo.
    /// \param size Tamaño del búfer.
    /// \param usage Flags de uso del búfer.
    /// \param properties Propiedades de la memoria requerida.
    /// \param buffer Salida con el manejador del búfer.
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

    /// \brief Finaliza y envía el command buffer de un solo uso y libera recursos.
    /// \param commandBuffer El command buffer temporal a finalizar.
    void endSingleUseCommands(VkCommandBuffer commandBuffer);

    /// \brief Copia datos entre búferes con un comando inmediato.
    /// \param srcBuffer Origen.
    /// \param dstBuffer Destino.
    /// \param size Tamaño de la copia.
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /// \brief Copia datos desde un búfer a una imagen.
    /// \param buffer Búfer origen.
    /// \param image Imagen destino.
    /// \param width Anchura de la región a copiar.
    /// \param height Altura de la región a copiar.
    /// \param layerCount Número de capas de la imagen a copiar.
    void copyBufferToImage(
        VkBuffer buffer,
        VkImage image,
        uint32_t width,
        uint32_t height,
        uint32_t layerCount);

    /// \brief Crea una imagen con los parámetros indicados y asigna su memoria.
    /// \param imageInfo Estructura de creación de imagen.
    /// \param properties Propiedades de la memoria requerida.
    /// \param image Salida con la imagen creada.
    /// \param imageMemory Salida con la memoria asignada.
    void createImageWithInfo(
        const VkImageCreateInfo& imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory);

    /// \brief Propiedades del dispositivo físico seleccionado.
    VkPhysicalDeviceProperties deviceProperties;

private:
    /// \brief Crea la instancia de Vulkan con las extensiones y capas requeridas.
    void createInstance();

    /// \brief Inicializa el mensajero de depuración si está habilitado.
    void setupDebugMessenger();

    /// \brief Crea la surface asociada a la ventana.
    void createSurface();

    /// \brief Selecciona el dispositivo físico adecuado.
    void selectPhysicalDevice();

    /// \brief Crea el dispositivo lógico y obtiene colas de gráficos y presentación.
    void createLogicalDevice();

    /// \brief Crea el command pool principal.
    void createCommandPool();

    /// \brief Verifica si un dispositivo físico cumple los requisitos.
    /// \param device Dispositivo físico candidato.
    /// \return Verdadero si cumple.
    bool isDeviceSuitable(VkPhysicalDevice device) const;

    /// \brief Extensiones requeridas por la plataforma.
    std::vector<const char*> getRequiredExtensions() const;

    /// \brief Comprueba si los validation layers solicitados están disponibles.
    bool checkValidationLayerSupport() const;

    /// \brief Busca familias de colas necesarias en un dispositivo físico.
    /// \param device Dispositivo físico candidato.
    /// \return Estructura con los índices localizados.
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

    /// \brief Rellena la estructura de creación del mensajero de depuración.
    /// \param createInfo Estructura a completar.
    void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;

    /// \brief Verifica que las extensiones requeridas por GLFW están disponibles.
    void verifyGLFWRequiredExtensions() const;

    /// \brief Comprueba soporte de extensiones de dispositivo.
    /// \param device Dispositivo físico candidato.
    /// \return Verdadero si se soportan.
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

    /// \brief Consulta soporte de swapchain para un dispositivo físico.
    /// \param device Dispositivo físico a consultar.
    /// \return Detalles de capacidades, formatos y modos.
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    /// Instancia de Vulkan.
    VkInstance instance;

    /// Mensajero de depuración para callback de validación.
    VkDebugUtilsMessengerEXT debugMessenger;

    /// Dispositivo físico seleccionado.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    /// Ventana asociada usada para crear la surface.
    Window& window;

    /// Command pool principal para asignar command buffers.
    VkCommandPool commandPool;

    /// Dispositivo lógico.
    VkDevice logicalDevice;

    /// Superficie de presentación.
    VkSurfaceKHR surface;

    /// Cola de gráficos.
    VkQueue graphicsQueue;

    /// Cola de presentación.
    VkQueue presentQueue;

    /// Lista de validation layers solicitados.
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    /// Extensiones de dispositivo requeridas.
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
