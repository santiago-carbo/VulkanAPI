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

 /// \brief Sistema de renderizado de luces puntuales.
 /// \details Se encarga de actualizar los datos de luces en el \c GlobalUbo
 /// y de emitir los comandos de dibujo asociados usando una tubería gráfica dedicada.
 /// La actualización \c update prepara la información de iluminación por frame;
 /// \c render graba los comandos de render sobre el
 /// \c VkCommandBuffer del \c FrameInfo.
class PointLightSystem
{
public:
    /// \brief Construye el sistema de luces puntuales y crea los recursos de pipeline.
    /// \param device Dispositivo lógico Vulkan.
    /// \param renderPass Render pass donde se ejecutará esta tubería.
    /// \param globalSetLayout Layout de descriptores global compatible con shaders.
    PointLightSystem(
        VulkanDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout);

    /// \brief Libera recursos asociados.
    ~PointLightSystem();

    PointLightSystem(const PointLightSystem&) = delete;
    PointLightSystem& operator=(const PointLightSystem&) = delete;

    /// \brief Actualiza el bloque UBO global con las luces activas de la escena.
    /// \details Recorre \c frameInfo.gameObjects y compacta luces puntuales en \c ubo.pointLights,
    /// ajustando \c ubo.numLights y parámetros de iluminación global si procede.
    /// \param frameInfo Contexto del frame.
    /// \param ubo Estructura \c GlobalUbo a rellenar/actualizar antes del render.
    void update(FrameInfo& frameInfo, GlobalUbo& ubo);

    /// \brief Emite los comandos de dibujo de las luces puntuales.
    /// \details Enlaza pipeline, descriptor set global y registra las draw calls necesarias
    /// para representar las luces.
    /// \param frameInfo Contexto del frame con \c commandBuffer activo.
    void render(FrameInfo& frameInfo);

private:
    /// \brief Crea el \c VkPipelineLayout compatible con el \c globalSetLayout.
    /// \param globalSetLayout Layout de descriptores global que usan los shaders de luz.
    void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

    /// \brief Crea la \c GraphicsPipeline específica para el pass de luces puntuales.
    /// \param renderPass Render pass objetivo donde se ejecutará la tubería.
    void createPipeline(VkRenderPass renderPass);

    /// Dispositivo lógico para crear recursos.
    VulkanDevice& vulkanDevice;
    /// Pipeline gráfica para el pass de luces.
    std::unique_ptr<GraphicsPipeline> pointLightPipeline;
    /// Layout de pipeline (descriptores/constantes).
    VkPipelineLayout pipelineLayout;
};

