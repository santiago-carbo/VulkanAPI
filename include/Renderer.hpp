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


class Renderer 
{
    public:
        Renderer(Window& window, VulkanDevice& device);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        VkRenderPass getSwapChainRenderPass() const 
        { 
            return (swapChain->getRenderPass()); 
        }

        size_t getSwapChainImageCount() const
        {
            return (swapChain->imageCount());
        }

        float getAspectRatio() const 
        { 
            return (swapChain->extentAspectRatio()); 
        }

        bool isFrameInProgress() const 
        { 
            return (isFrameStarted); 
        }

        VkCommandBuffer getCurrentCommandBuffer() const 
        {
            assert(isFrameStarted && 
                "💥[Vulkan API] Cannot get command buffer when frame not in progress");

            return (commandBuffers[currentFrameIndex]);
        }

        int getFrameIndex() const 
        {
            assert(isFrameStarted && 
                "💥[Vulkan API] Cannot get frame index when frame not in progress");

            return (currentFrameIndex);
        }

        Perf& getPerf() 
        { 
            return perf; 
        }

        const Perf& getPerf() const 
        { 
            return perf; 
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        Window& window;
        VulkanDevice& vulkanDevice;
        std::unique_ptr<SwapChain> swapChain;
        std::vector<VkCommandBuffer> commandBuffers;
        uint32_t currentImageIndex;
        int currentFrameIndex {0};
        bool isFrameStarted {false};
        Perf perf;
};
