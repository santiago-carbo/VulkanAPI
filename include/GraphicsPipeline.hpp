/*
 * Project: VulkanAPI
 * File: GraphicsPipeline.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

struct PipelineConfig 
{
    PipelineConfig() = default;
    PipelineConfig(const PipelineConfig&) = delete;
    PipelineConfig& operator=(const PipelineConfig&) = delete;

    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlending;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    std::vector<VkDynamicState> dynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicState;
    VkPipelineLayout layout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class GraphicsPipeline 
{
    public:
        GraphicsPipeline(
            VulkanDevice& device,
            const std::string& vertexPath,
            const std::string& fragmentPath,
            const PipelineConfig& config);

        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultConfig(PipelineConfig& config);

        static void enableAlphaBlending(PipelineConfig& config);

    private:
        static std::vector<char> readFile(const std::string& path);

        void create(const std::string& vertexPath, 
            const std::string& fragmentPath, 
            const PipelineConfig& config);

        void createShader(const std::vector<char>& code, VkShaderModule* module);

        VulkanDevice& device;
        VkPipeline pipeline;
        VkShaderModule vertexModule;
        VkShaderModule fragmentModule;
};
