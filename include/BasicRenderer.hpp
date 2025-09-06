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

 /// \brief Sistema de renderizado básico para geometría opaca.
 /// \details Encapsula la creación del \c VkPipelineLayout y del \c GraphicsPipeline
 /// asociado al \c VkRenderPass principal. Proporciona el método \c render
 /// para emitir las draw calls de la escena empleando los recursos globales
 /// (descriptor set con UBO, etc.) definidos en \c FrameInfo.
class BasicRenderer
{
    public:
        /// \brief Construye el renderizador básico y prepara los objetos de estado de pipeline.
        /// \details Inicializa el \c VkPipelineLayout y el \c GraphicsPipeline necesarios para
        /// dibujar en el \c VkRenderPass indicado. No toma propiedad de \c device.
        /// \param device Dispositivo Vulkan sobre el que se crean los recursos.
        /// \param renderPass Render pass destino donde se adjuntará el pipeline.
        /// \param globalDescriptorSetLayout Layout del descriptor set global (p.ej., UBO).
        BasicRenderer(
        VulkanDevice& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalDescriptorSetLayout);

        /// \brief Libera los recursos asociados al pipeline gráfico y su layout.
        ~BasicRenderer();

        BasicRenderer(const BasicRenderer&) = delete;
        BasicRenderer& operator=(const BasicRenderer&) = delete;

        /// \brief Renderiza la escena para el frame actual.
        /// \details Enlaza el pipeline, configura los descriptor sets globales
        /// y emite las draw calls sobre los objetos disponibles en \c frameInfo.
        /// \param frameInfo Contexto del frame (command buffer, descriptor set, cámara, etc.).
        void render(FrameInfo& frameInfo);

        /// \brief Graba draw calls de un rango [begin, end) de gameObjects en un command buffer 
        /// ya iniciado.
        /// \param frameInfo Contexto del frame.
        /// \param cbSec Command buffer secundario ya comenzado con inheritance correcto.
        /// \param begin Índice inicial (incluido) dentro de la vista de objetos a dibujar.
        /// \param end   Índice final (excluido).
        void recordRange(FrameInfo& frameInfo, VkCommandBuffer cbSec, size_t begin, size_t end);


    private:
        /// \brief Crea el \c VkPipelineLayout en función del layout de descriptores global.
        /// \param globalDescriptorSetLayout Layout del descriptor set global 
        /// (binding de UBO, texturas, ...).
        /// \note Debe invocarse antes de \c createGraphicsPipeline().
        void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);

        /// \brief Crea el \c GraphicsPipeline asociado al \c renderPass.
        /// \param renderPass Render pass donde se usará este pipeline.
        /// \pre Requiere que \c pipelineLayout haya sido creado.
        void createGraphicsPipeline(VkRenderPass renderPass);

        /// Dispositivo Vulkan usado para crear/gestionar recursos.
        VulkanDevice& device;

        /// Pipeline gráfico para el pass de geometría básica.
        std::unique_ptr<GraphicsPipeline> pipeline;

        /// Layout del pipeline (sets, push constants, estados fijos).
        VkPipelineLayout pipelineLayout;
};

