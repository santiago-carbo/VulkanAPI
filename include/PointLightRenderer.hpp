/*
 * Project: VulkanAPI
 * File: PointLightRenderer.hpp
 * Author: Santiago Carb� Garc�a
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "FrameContext.hpp"
#include "GraphicsPipeline.hpp"

 /// \brief Sistema de renderizado de luces puntuales.
 /// \details Se encarga de actualizar los datos de luces en el \c GlobalUbo
 /// y de emitir los comandos de dibujo asociados usando una tuber�a gr�fica dedicada.
 /// La actualizaci�n \c update prepara la informaci�n de iluminaci�n por frame;
 /// \c render graba los comandos de render sobre el
 /// \c VkCommandBuffer del \c FrameInfo.
class PointLightSystem
{
public:
    /// \brief Construye el sistema de luces puntuales y crea los recursos de pipeline.
    /// \param device Dispositivo l�gico Vulkan.
    /// \param renderPass Render pass donde se ejecutar� esta tuber�a.
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
    /// ajustando \c ubo.numLights y par�metros de iluminaci�n global si procede.
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

    /// \brief Crea la \c GraphicsPipeline espec�fica para el pass de luces puntuales.
    /// \param renderPass Render pass objetivo donde se ejecutar� la tuber�a.
    void createPipeline(VkRenderPass renderPass);

    /// Dispositivo l�gico para crear recursos.
    VulkanDevice& vulkanDevice;
    /// Pipeline gr�fica para el pass de luces.
    std::unique_ptr<GraphicsPipeline> pointLightPipeline;
    /// Layout de pipeline (descriptores/constantes).
    VkPipelineLayout pipelineLayout;
};

