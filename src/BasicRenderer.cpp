#include "BasicRenderer.hpp"

#include <stdexcept>

struct PushConstantData 
{
    glm::mat4 modelMatrix {1.0f};
    glm::mat4 normalMatrix {1.0f};
};

BasicRenderer::BasicRenderer(
    VulkanDevice& device,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalDescriptorSetLayout)
    : device{device} 
{
    createPipelineLayout(globalDescriptorSetLayout);
    createGraphicsPipeline(renderPass);
}

BasicRenderer::~BasicRenderer() 
{
    vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}

void BasicRenderer::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout) 
{
    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantData);

    std::vector<VkDescriptorSetLayout> setLayouts{globalDescriptorSetLayout};

    VkPipelineLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    layoutInfo.pSetLayouts = setLayouts.data();
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(
        device.getDevice(), 
        &layoutInfo, 
        nullptr, 
        &pipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create pipeline layout.");
    }
}

void BasicRenderer::createGraphicsPipeline(VkRenderPass renderPass) 
{
    assert(pipelineLayout != nullptr &&
        "💥[Vulkan API] Cannot create pipeline without a valid layout.");

    PipelineConfig configInfo {};
    GraphicsPipeline::defaultConfig(configInfo);
    configInfo.renderPass = renderPass;
    configInfo.layout = pipelineLayout;

    pipeline = std::make_unique<GraphicsPipeline>(
        device,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        configInfo);
}

void BasicRenderer::render(FrameInfo& frameInfo) 
{
    pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    for (std::pair<const unsigned int, GameObject>& entry : frameInfo.gameObjects) 
    {
        GameObject& object = entry.second;

        if (object.model == nullptr) 
        {
            continue;
        }

        PushConstantData push{};
        push.modelMatrix = object.transform.matrix();
        push.normalMatrix = object.transform.normalMatrix();

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData),
            &push);

        object.model->bind(frameInfo.commandBuffer);
        object.model->draw(frameInfo.commandBuffer);
    }
}
