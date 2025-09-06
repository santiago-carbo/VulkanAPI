/*
 * Project: VulkanAPI
 * File: GraphicsPipeline.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

 /// \brief Estructura de configuración para construir una \c VkPipeline gráfica.
 /// \details Agrega todos los bloques de estado necesarios para inicializar
 /// una tubería en Vulkan. Debe rellenarse antes de llamar al
 /// constructor de \c GraphicsPipeline. \c defaultConfig() provee
 /// valores razonables por defecto.
struct PipelineConfig
{
    PipelineConfig() = default;
    PipelineConfig(const PipelineConfig&) = delete;
    PipelineConfig& operator=(const PipelineConfig&) = delete;

    /// Descriptores de binding.
    std::vector<VkVertexInputBindingDescription> bindings;
    /// Atributos por ubicación.
    std::vector<VkVertexInputAttributeDescription> attributes;

    /// Viewport y scissor.
    VkPipelineViewportStateCreateInfo viewport;
    /// Tipo de primitivas y primitiva de reinicio.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    /// Relleno, culling, front face, profundidad, etc.
    VkPipelineRasterizationStateCreateInfo rasterizer;
    /// MSAA.
    VkPipelineMultisampleStateCreateInfo multisampling;
    /// Mezcla por attachment.
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    /// Estado global de color blending.
    VkPipelineColorBlendStateCreateInfo colorBlending;
    /// Pruebas de profundidad/plantilla.
    VkPipelineDepthStencilStateCreateInfo depthStencil;

    /// Lista de estados dinámicos.
    std::vector<VkDynamicState> dynamicStates;
    /// Bloque de estado dinámico.
    VkPipelineDynamicStateCreateInfo dynamicState;

    /// Pipeline layout.
    VkPipelineLayout layout = nullptr;
    /// Render pass destino.
    VkRenderPass renderPass = nullptr;
    /// Índice de subpass dentro del render pass.
    uint32_t subpass = 0;
};

/// \brief Encapsula la creación y uso de una \c VkPipeline gráfica + shaders.
/// \details Carga módulos de shader (SPIR-V), fija estados del pipeline a
/// partir de \c PipelineConfig y expone \c bind() para enlazar la
/// tubería durante el registro de comandos.
class GraphicsPipeline
{
public:
    /// \brief Construye la tubería gráfica a partir de rutas de shaders y configuración.
    /// \param device Dispositivo lógico Vulkan.
    /// \param vertexPath Ruta del shader de vértices en SPIR-V (.spv).
    /// \param fragmentPath Ruta del shader de fragmentos en SPIR-V (.spv).
    /// \param config Estructura \c PipelineConfig completamente rellenada.
    GraphicsPipeline(
        VulkanDevice& device,
        const std::string& vertexPath,
        const std::string& fragmentPath,
        const PipelineConfig& config);

    /// \brief Destruye la \c VkPipeline y los módulos de shader.
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    /// \brief Enlaza la tubería al \c commandBuffer activo.
    /// \param commandBuffer Command buffer.
    void bind(VkCommandBuffer commandBuffer);

    /// \brief Rellena \c config con valores por defecto razonables.
    /// \details Incluye estado de rasterización, profundidad, mezcla desactivada,
    /// viewport/scissor como dinámicos, input assembly en \c TRIANGLE_LIST, etc.
    /// \param config Referencia a configurar.
    static void defaultConfig(PipelineConfig& config);

    /// \brief Activa la mezcla alfa predecible para el primer color attachment.
    /// \details Ajusta \c colorBlendAttachment y \c colorBlending para
    /// composiciones con transparencia estándar.
    /// \param config Referencia a \c PipelineConfig a modificar.
    static void enableAlphaBlending(PipelineConfig& config);

private:
    /// \brief Carga un archivo binario (SPIR-V) a memoria.
    /// \param path Ruta del archivo.
    /// \return Vector de bytes con el contenido del fichero.
    static std::vector<char> readFile(const std::string& path);

    /// \brief Crea la \c VkPipeline con los módulos de shader y el \c PipelineConfig.
    /// \param vertexPath Ruta del shader de vértices.
    /// \param fragmentPath Ruta del shader de fragmentos.
    /// \param config Configuración completa del pipeline.
    void create(const std::string& vertexPath,
        const std::string& fragmentPath,
        const PipelineConfig& config);

    /// \brief Crea un \c VkShaderModule desde el código SPIR-V dado.
    /// \param code Bytecode SPIR-V.
    /// \param module Salida: handle del módulo creado.
    void createShader(const std::vector<char>& code, VkShaderModule* module);

    /// Dispositivo lógico usado para crear la tubería.
    VulkanDevice& device;
    /// Handle de la \c VkPipeline creada.
    VkPipeline pipeline;
    /// Módulo de shader de vértices.
    VkShaderModule vertexModule;
    /// Módulo de shader de fragmentos.
    VkShaderModule fragmentModule;
};

