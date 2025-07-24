#include "PointLightRenderer.hpp"

#include <map>
#include <stdexcept>

struct PointLightPushConstants 
{
    glm::vec4 position {};
    glm::vec4 color {};
    float radius;
};

PointLightSystem::PointLightSystem(
    VulkanDevice& device, 
    VkRenderPass renderPass, 
    VkDescriptorSetLayout globalSetLayout)
    : vulkanDevice{device} 
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() 
{
    vkDestroyPipelineLayout(vulkanDevice.getDevice(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
{
    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PointLightPushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(
        vulkanDevice.getDevice(),
        &pipelineLayoutInfo,
        nullptr,
        &pipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create pipeline layout.");
    }
}

void PointLightSystem::createPipeline(VkRenderPass renderPass) 
{
    assert(pipelineLayout != nullptr && 
        "💥[Vulkan API] Cannot create pipeline before pipeline layout.");

    PipelineConfig pipelineConfig {};
    GraphicsPipeline::defaultConfig(pipelineConfig);
    GraphicsPipeline::enableAlphaBlending(pipelineConfig);

    pipelineConfig.attributes.clear();
    pipelineConfig.bindings.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.layout = pipelineLayout;

    pointLightPipeline = std::make_unique<GraphicsPipeline>(
        vulkanDevice,
        "shaders/point_light.vert.spv",
        "shaders/point_light.frag.spv",
        pipelineConfig);
}

void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) 
{
    glm::mat4 rotateLight =
        glm::rotate(glm::mat4(1.0f), 0.5f * frameInfo.frameTime, {0.0f, -1.0f, 0.0f});

    int lightIndex = 0;

    for (std::pair<const unsigned int, GameObject>& kv : frameInfo.gameObjects) 
    {
        GameObject& obj = kv.second;

        if (obj.light == nullptr) 
        {
            continue;
        }

        assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

        obj.transform.translation =
            glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.0f));

        ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.0f);
        ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.light->intensity);

        lightIndex += 1;
    }

    ubo.numLights = lightIndex;
}

void PointLightSystem::render(FrameInfo& frameInfo) 
{
    std::map<float, unsigned int> sorted;

    for (std::pair<const unsigned int, GameObject>& kv : frameInfo.gameObjects) 
    {
        GameObject& obj = kv.second;

        if (obj.light == nullptr) 
        {
            continue;
        }

        auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
        float disSquared = glm::dot(offset, offset);
        sorted[disSquared] = obj.getId();
    }

    pointLightPipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr);

    for (std::map<float, unsigned int>::reverse_iterator it = sorted.rbegin();
        it != sorted.rend(); ++it) 
    {
        GameObject& obj = frameInfo.gameObjects.at(it->second);

        PointLightPushConstants push{};
        push.position = glm::vec4(obj.transform.translation, 1.0f);
        push.color = glm::vec4(obj.color, obj.light->intensity);
        push.radius = obj.transform.scale.x;

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PointLightPushConstants),
            &push);

        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
}
