/*
 * Project: VulkanAPI
 * File: BasicRenderer.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "BasicRenderer.hpp"

#include <stdexcept>

 /// \brief Datos enviados por push constants a los shaders.
 /// \details Contiene la matriz de modelo y la matriz de normales del objeto actual.
struct PushConstantData
{
    glm::mat4 modelMatrix {1.0f};
    glm::mat4 normalMatrix {1.0f};
};

/// \brief Construye el renderizador básico y prepara los objetos de estado de pipeline.
/// \details Inicializa el \c VkPipelineLayout y el \c GraphicsPipeline necesarios para
/// dibujar en el \c VkRenderPass indicado. No toma propiedad de \c device.
/// \param device Dispositivo Vulkan sobre el que se crean los recursos.
/// \param renderPass Render pass destino donde se adjuntará el pipeline.
/// \param globalDescriptorSetLayout Layout del descriptor set global (p.ej., UBO).
BasicRenderer::BasicRenderer(
    VulkanDevice& device,
    VkRenderPass renderPass,
    VkDescriptorSetLayout globalDescriptorSetLayout)
    : device{device}
{
    createPipelineLayout(globalDescriptorSetLayout);
    createGraphicsPipeline(renderPass);
}

/// \brief Libera los recursos asociados al pipeline gráfico y su layout.
BasicRenderer::~BasicRenderer()
{
    vkDestroyPipelineLayout(device.getDevice(), pipelineLayout, nullptr);
}

/// \brief Crea el \c VkPipelineLayout en función del layout de descriptores global.
/// \param globalDescriptorSetLayout Layout del descriptor set global 
/// (binding de UBO, texturas, ...).
/// \note Debe invocarse antes de \c createGraphicsPipeline().
void BasicRenderer::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
{
    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantData);

    std::vector<VkDescriptorSetLayout> setLayouts{ globalDescriptorSetLayout };

    VkPipelineLayoutCreateInfo layoutInfo{};
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

/// \brief Crea el \c GraphicsPipeline asociado al \c renderPass.
/// \param renderPass Render pass donde se usará este pipeline.
/// \pre Requiere que \c pipelineLayout haya sido creado.
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

/// \brief Renderiza la escena para el frame actual.
/// \details Enlaza el pipeline, configura los descriptor sets globales
/// y emite las draw calls sobre los objetos disponibles en \c frameInfo.
/// \param frameInfo Contexto del frame (command buffer, descriptor set, cámara, etc.).
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

        PushConstantData push {};
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

/// \brief Renderiza la escena para el frame actual.
/// \details Enlaza el pipeline, configura los descriptor sets globales
/// y emite las draw calls sobre los objetos disponibles en \c frameInfo.
/// \param frameInfo Contexto del frame (command buffer, descriptor set, cámara, etc.).
void BasicRenderer::recordRange(
    FrameInfo& frameInfo, 
    VkCommandBuffer cbSec, 
    size_t begin, 
    size_t end)
{
    pipeline->bind(cbSec);

    vkCmdBindDescriptorSets(
        cbSec, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

    std::vector<std::pair<unsigned, GameObject*>> view; view.reserve(frameInfo.gameObjects.size());
    
    for (auto& go : frameInfo.gameObjects)
    {
        view.push_back({go.first, &go.second});
    }

    if (begin > view.size())
    {
        begin = view.size();
    }

    if (end > view.size()) 
    {
        end = view.size();
    }

    for (size_t i = begin; i < end; ++i)
    {
        GameObject& object = *view[i].second;

        if (!object.model)
        {
            continue;
        }

        PushConstantData push {};
        push.modelMatrix = object.transform.matrix();
        push.normalMatrix = object.transform.normalMatrix();

        vkCmdPushConstants(
            cbSec, pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0, sizeof(PushConstantData), &push);

        object.model->bind(cbSec);
        object.model->draw(cbSec);
    }
}


