/*
 * Project: VulkanAPI
 * File: BasicRenderer.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */


#pragma once

#include "FrameContext.hpp"
#include "GraphicsPipeline.hpp"
#include "VulkanDevice.hpp"

#include <memory>

class BasicRenderer 
{
    public:
        BasicRenderer(
            VulkanDevice& device,
            VkRenderPass renderPass,
        VkDescriptorSetLayout globalDescriptorSetLayout);

        ~BasicRenderer();

        BasicRenderer(const BasicRenderer&) = delete;
        BasicRenderer& operator=(const BasicRenderer&) = delete;

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
        void createGraphicsPipeline(VkRenderPass renderPass);

        VulkanDevice& device;
        std::unique_ptr<GraphicsPipeline> pipeline;
        VkPipelineLayout pipelineLayout;
};
