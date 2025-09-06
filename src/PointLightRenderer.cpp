/*
 * Project: VulkanAPI
 * File: PointLightRenderer.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "PointLightRenderer.hpp"

#include <map>
#include <stdexcept>

/// \brief Push constants de la luz puntual.
/// \details Datos mínimos que el shader necesita por luz: posición, color y radio de influencia.
struct PointLightPushConstants 
{
    glm::vec4 position {};
    glm::vec4 color {};
    float radius;
};

/// \brief Construye el sistema de luces puntuales y crea los recursos de pipeline.
/// \param device Dispositivo lógico Vulkan.
/// \param renderPass Render pass donde se ejecutará esta tubería.
/// \param globalSetLayout Layout de descriptores global compatible con shaders.
PointLightSystem::PointLightSystem(
    VulkanDevice& device, 
    VkRenderPass renderPass, 
    VkDescriptorSetLayout globalSetLayout)
    : vulkanDevice{device} 
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

/// \brief Libera recursos asociados.
PointLightSystem::~PointLightSystem() 
{
    vkDestroyPipelineLayout(vulkanDevice.getDevice(), pipelineLayout, nullptr);
}

/// \brief Crea el \c VkPipelineLayout compatible con el \c globalSetLayout.
/// \param globalSetLayout Layout de descriptores global que usan los shaders de luz.
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

/// \brief Crea la \c GraphicsPipeline específica para el pass de luces puntuales.
/// \param renderPass Render pass objetivo donde se ejecutará la tubería.
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

/// \brief Actualiza el bloque UBO global con las luces activas de la escena.
/// \details Recorre \c frameInfo.gameObjects y compacta luces puntuales en \c ubo.pointLights,
/// ajustando \c ubo.numLights y parámetros de iluminación global si procede.
/// \param frameInfo Contexto del frame.
/// \param ubo Estructura \c GlobalUbo a rellenar/actualizar antes del render.
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

/// \brief Emite los comandos de dibujo de las luces puntuales.
/// \details Enlaza pipeline, descriptor set global y registra las draw calls necesarias
/// para representar las luces.
/// \param frameInfo Contexto del frame con \c commandBuffer activo.
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
