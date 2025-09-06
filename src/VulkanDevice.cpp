/*
 * Project: VulkanAPI
 * File: VulkanDevice.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "VulkanDevice.hpp"

#include <iostream>
#include <unordered_set>
#include <set>

 /// \brief Callback de validación de Vulkan (Debug Utils).
 /// \details Recibe los mensajes del validador de Vulkan y los escribe en stderr.
 /// Devuelve \c VK_FALSE para indicar que la llamada no debe interrumpir la ejecución.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData) 
{
    std::cerr << "[Vulkan API] Validation Layer: " << callbackData->pMessage << std::endl;
    
    return (VK_FALSE);
}

/// \brief Crea el mensajero de depuración (Debug Utils Messenger) en tiempo de ejecución.
/// \details Carga la función de creación mediante \c vkGetInstanceProcAddr para no
/// depender estáticamente de la extensión. Si está disponible, crea el mensajero.
VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debugMessenger) 
{
    PFN_vkCreateDebugUtilsMessengerEXT func = 
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    if (func) 
    {
        return (func(instance, createInfo, allocator, debugMessenger));
    }

    return (VK_ERROR_EXTENSION_NOT_PRESENT);
}

/// \brief Destruye el mensajero de depuración (Debug Utils Messenger).
/// \details Carga la función de destrucción mediante \c vkGetInstanceProcAddr.
/// Si la función existe, destruye el recurso asociado al mensajero.
void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* allocator) 
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = 
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (func) 
    {
        func(instance, debugMessenger, allocator);
    }
}

/// \brief Construye el dispositivo asociado a una ventana y crea los recursos base.
/// \param window Referencia a la ventana usada para crear la surface.
VulkanDevice::VulkanDevice(Window& window) 
    : window(window) 
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    selectPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
}

/// \brief Libera recursos del dispositivo y objetos dependientes.
VulkanDevice::~VulkanDevice() 
{
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);

    if (enableValidationLayers) 
    {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

/// \brief Crea la instancia de Vulkan con las extensiones y capas requeridas.
void VulkanDevice::createInstance() 
{
    if (enableValidationLayers && !checkValidationLayerSupport()) 
    {
        throw std::runtime_error("💥[Vulkan API] Validation layers requested but not available.");
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan API";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};

    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create Vulkan instance.");
    }

    verifyGLFWRequiredExtensions();
}

/// \brief Inicializa el mensajero de depuración si está habilitado.
void VulkanDevice::setupDebugMessenger()
{
    if (!enableValidationLayers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerInfo(createInfo);

    if (createDebugUtilsMessengerEXT(
        instance,
        &createInfo,
        nullptr,
        &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("💥[Vulkan API] Failed to set up debug messenger.");
    }
}

/// \brief Crea la surface asociada a la ventana.
void VulkanDevice::createSurface()
{
    window.createWindowSurface(instance, &surface);
}

/// \brief Selecciona el dispositivo físico adecuado.
void VulkanDevice::selectPhysicalDevice() 
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to find GPUs with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const VkPhysicalDevice& device : devices) 
    {
        if (isDeviceSuitable(device)) 
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to find a suitable GPU.");
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    std::cout << "[Vulkan API] Selected GPU: " << deviceProperties.deviceName << std::endl;
}

/// \brief Crea el dispositivo lógico y obtiene colas de gráficos y presentación.
void VulkanDevice::createLogicalDevice() 
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) 
    {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create logical device.");
    }

    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);
}

/// \brief Crea el command pool principal.
void VulkanDevice::createCommandPool() 
{
    QueueFamilyIndices indices = getQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | 
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create command pool.");
    }
}

/// \brief Verifica si un dispositivo físico cumple los requisitos.
/// \param device Dispositivo físico candidato.
/// \return Verdadero si cumple.
bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) const 
{
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;

    if (extensionsSupported) 
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && 
            !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return (indices.isComplete() &&
        extensionsSupported &&
        swapChainAdequate &&
        supportedFeatures.samplerAnisotropy);
}

/// \brief Extensiones requeridas por la plataforma.
std::vector<const char*> VulkanDevice::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return (extensions);
}

/// \brief Comprueba si los validation layers solicitados están disponibles.
bool VulkanDevice::checkValidationLayerSupport() const
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const VkLayerProperties& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return (false);
        }
    }

    return (true);
}

/// \brief Busca familias de colas necesarias en un dispositivo físico.
/// \param device Dispositivo físico candidato.
/// \return Estructura con los índices localizados.
QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) const
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        const VkQueueFamilyProperties& queueFamily = queueFamilies[i];

        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            indices.hasGraphicsFamily = true;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport)
        {
            indices.presentFamily = i;
            indices.hasPresentFamily = true;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    return (indices);
}

/// \brief Rellena la estructura de creación del mensajero de depuración.
/// \param createInfo Estructura a completar.
void VulkanDevice::populateDebugMessengerInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo) const
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

/// \brief Verifica que las extensiones requeridas por GLFW están disponibles.
void VulkanDevice::verifyGLFWRequiredExtensions() const
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    std::unordered_set<std::string> available;

    for (const VkExtensionProperties& extension : availableExtensions) 
    {
        available.insert(extension.extensionName);
    }

    std::vector<const char*> requiredExtensions = getRequiredExtensions();

    for (const char* required : requiredExtensions)
    {
        if (available.find(required) == available.end()) 
        {
            throw std::runtime_error(
                "💥[Vulkan API] Missing required GLFW extension: " + 
                std::string(required) + ".");
        }
    }
}

/// \brief Comprueba soporte de extensiones de dispositivo.
/// \param device Dispositivo físico candidato.
/// \return Verdadero si se soportan.
bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) const 
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device, 
        nullptr, 
        &extensionCount, 
        availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const VkExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return (requiredExtensions.empty());
}

/// \brief Consulta soporte de swapchain para un dispositivo físico.
/// \param device Dispositivo físico a consultar.
/// \return Detalles de capacidades, formatos y modos.
SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice device) const 
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) 
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, 
            surface, 
            &formatCount, 
            details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) 
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, 
            surface, 
            &presentModeCount, 
            details.presentModes.data());
    }

    return (details);
}

/// \brief Busca un tipo de memoria de dispositivo que cumpla las propiedades requeridas.
/// \param typeFilter Máscara de tipos aceptables.
/// \param properties Propiedades de memoria requeridas.
/// \return Índice de tipo de memoria válido.
uint32_t VulkanDevice::findMemoryType(
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return (i);
        }
    }

    throw std::runtime_error("💥[Vulkan API] Failed to find suitable memory type.");
}

/// \brief Elige un formato soportado a partir de candidatos y características requeridas.
/// \param candidates Lista de formatos candidatos.
/// \param tiling Tipeado de imagen requerido.
/// \param features Conjunto de características requeridas.
/// \return Formato compatible.
VkFormat VulkanDevice::findSupportedFormat(
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features) const 
{
    for (VkFormat format : candidates) 
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if ((tiling == VK_IMAGE_TILING_LINEAR) && 
            (props.linearTilingFeatures & features) == features) 
        {
            return (format);
        }

        if ((tiling == VK_IMAGE_TILING_OPTIMAL) && 
            (props.optimalTilingFeatures & features) == features) 
        {
            return (format);
        }
    }

    throw std::runtime_error("💥[Vulkan API] Failed to find supported format.");
}

/// \brief Crea un VkBuffer y asigna su memoria en el dispositivo.
/// \param size Tamaño del búfer.
/// \param usage Flags de uso del búfer.
/// \param properties Propiedades de la memoria requerida.
/// \param buffer Salida con el manejador del búfer.
/// \param bufferMemory Salida con la memoria asignada.
void VulkanDevice::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory) 
{

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to allocate buffer memory.");
    }

    vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
}

/// \brief Comienza un comando de un solo uso en un command buffer temporal.
/// \return Command buffer listo para grabar.
VkCommandBuffer VulkanDevice::beginSingleUseCommands() 
{
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return (commandBuffer);
}

/// \brief Finaliza y envía el command buffer de un solo uso y libera recursos.
/// \param commandBuffer El command buffer temporal a finalizar.
void VulkanDevice::endSingleUseCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

/// \brief Copia datos entre búferes con un comando inmediato.
/// \param srcBuffer Origen.
/// \param dstBuffer Destino.
/// \param size Tamaño de la copia.
void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
{
    VkCommandBuffer commandBuffer = beginSingleUseCommands();

    VkBufferCopy copyRegion {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleUseCommands(commandBuffer);
}

/// \brief Copia datos desde un búfer a una imagen.
/// \param buffer Búfer origen.
/// \param image Imagen destino.
/// \param width Anchura de la región a copiar.
/// \param height Altura de la región a copiar.
/// \param layerCount Número de capas de la imagen a copiar.
void VulkanDevice::copyBufferToImage(
    VkBuffer buffer, 
    VkImage image, 
    uint32_t width, 
    uint32_t height, 
    uint32_t layerCount) 
{
    VkCommandBuffer commandBuffer = beginSingleUseCommands();

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    endSingleUseCommands(commandBuffer);
}

/// \brief Crea una imagen con los parámetros indicados y asigna su memoria.
/// \param imageInfo Estructura de creación de imagen.
/// \param properties Propiedades de la memoria requerida.
/// \param image Salida con la imagen creada.
/// \param imageMemory Salida con la memoria asignada.
void VulkanDevice::createImageWithInfo(
    const VkImageCreateInfo& imageInfo,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory) 
{

    if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to allocate image memory.");
    }

    if (vkBindImageMemory(logicalDevice, image, imageMemory, 0) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to bind image memory.");
    }
}
