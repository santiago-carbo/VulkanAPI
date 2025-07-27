#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include "GameObject.hpp"

class EditorUI 
{
    public:
        static constexpr int WIDTH = 1600;
        static constexpr int HEIGHT = 900;

        EditorUI();
        ~EditorUI();

        void init(VkInstance instance,
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            uint32_t graphicsQueueFamily,
            VkQueue graphicsQueue,
            VkRenderPass renderPass,
            uint32_t imageCount);

        void beginFrame();
        void drawGameObjects(std::unordered_map<unsigned int, GameObject>& gameObjects);
        void endFrame(VkCommandBuffer commandBuffer);
        void cleanup(VkDevice device);

        Window& getWindow()
        {    
            return window; 
        }

    private:
        void createDescriptorPool(VkDevice device);

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        Window window{WIDTH, HEIGHT, "Vulkan API"};
};

