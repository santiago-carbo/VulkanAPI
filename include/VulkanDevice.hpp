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

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices 
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool hasGraphicsFamily = false;
    bool hasPresentFamily = false;
    
    uint32_t GetGraphicsFamily() const 
    { 
        return graphicsFamily; 
    }

    uint32_t GetPresentFamily() const 
    { 
        return presentFamily; 
    }

    bool isComplete() const 
    { 
        return (hasGraphicsFamily && hasPresentFamily); 
    }
};

class VulkanDevice 
{
    public:
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

        VulkanDevice(Window& window);
        ~VulkanDevice();

        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice& operator=(const VulkanDevice&) = delete;
        VulkanDevice(VulkanDevice&&) = delete;
        VulkanDevice& operator=(VulkanDevice&&) = delete;

        VkInstance getInstance() const
        {
            return instance;
        }

        VkCommandPool getCommandPool() const 
        { 
            return (commandPool); 
        }

        VkPhysicalDevice getPhysicalDevice() const
        {
            return (physicalDevice);
        }

        VkDevice getDevice() const 
        { 
            return (logicalDevice); 
        }

        VkSurfaceKHR getSurface() const 
        { 
            return (surface); 
        }

        VkQueue getGraphicsQueue() const 
        { 
            return (graphicsQueue); 
        }

        VkQueue getPresentQueue() const 
        { 
            return (presentQueue); 
        }

        SwapChainSupportDetails getSwapChainSupportDetails() const 
        { 
            return (querySwapChainSupport(physicalDevice)); 
        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        QueueFamilyIndices getQueueFamilyIndices() const 
        { 
            return (findQueueFamilies(physicalDevice)); 
        }

        VkFormat findSupportedFormat(
        const std::vector<VkFormat>& candidates, 
            VkImageTiling tiling, 
            VkFormatFeatureFlags features) const;

        void createBuffer(
            VkDeviceSize size, 
            VkBufferUsageFlags usage, 
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer, 
            VkDeviceMemory& bufferMemory);

        VkCommandBuffer beginSingleUseCommands();
        void endSingleUseCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBufferToImage(
            VkBuffer buffer, 
            VkImage image, 
            uint32_t width, 
            uint32_t height, 
            uint32_t layerCount);

        void createImageWithInfo(
            const VkImageCreateInfo& imageInfo, 
            VkMemoryPropertyFlags properties,
            VkImage& image, 
            VkDeviceMemory& imageMemory);

        VkPhysicalDeviceProperties deviceProperties;

    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void selectPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        bool isDeviceSuitable(VkPhysicalDevice device) const;
        std::vector<const char*> getRequiredExtensions() const;
        bool checkValidationLayerSupport() const;
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
        void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
        void verifyGLFWRequiredExtensions() const;
        bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        Window& window;
        VkCommandPool commandPool;

        VkDevice logicalDevice;
        VkSurfaceKHR surface;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
