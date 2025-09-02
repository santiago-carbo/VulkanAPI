/*
 * Project: VulkanAPI
 * File: PointLightRenderer.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "FrameContext.hpp"
#include "GraphicsPipeline.hpp"


class PointLightSystem 
{
    public:
        PointLightSystem(
            VulkanDevice& device,
            VkRenderPass renderPass,
            VkDescriptorSetLayout globalSetLayout);

        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void update(FrameInfo& frameInfo, GlobalUbo& ubo);

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        VulkanDevice& vulkanDevice;
        std::unique_ptr<GraphicsPipeline> pointLightPipeline;
        VkPipelineLayout pipelineLayout;
};
