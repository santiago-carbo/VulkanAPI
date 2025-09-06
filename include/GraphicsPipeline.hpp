/*
 * Project: VulkanAPI
 * File: GraphicsPipeline.hpp
 * Author: Santiago Carb� Garc�a
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

 /// \brief Estructura de configuraci�n para construir una \c VkPipeline gr�fica.
 /// \details Agrega todos los bloques de estado necesarios para inicializar
 /// una tuber�a en Vulkan. Debe rellenarse antes de llamar al
 /// constructor de \c GraphicsPipeline. \c defaultConfig() provee
 /// valores razonables por defecto.
struct PipelineConfig
{
    PipelineConfig() = default;
    PipelineConfig(const PipelineConfig&) = delete;
    PipelineConfig& operator=(const PipelineConfig&) = delete;

    /// Descriptores de binding.
    std::vector<VkVertexInputBindingDescription> bindings;
    /// Atributos por ubicaci�n.
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

    /// Lista de estados din�micos.
    std::vector<VkDynamicState> dynamicStates;
    /// Bloque de estado din�mico.
    VkPipelineDynamicStateCreateInfo dynamicState;

    /// Pipeline layout.
    VkPipelineLayout layout = nullptr;
    /// Render pass destino.
    VkRenderPass renderPass = nullptr;
    /// �ndice de subpass dentro del render pass.
    uint32_t subpass = 0;
};

/// \brief Encapsula la creaci�n y uso de una \c VkPipeline gr�fica + shaders.
/// \details Carga m�dulos de shader (SPIR-V), fija estados del pipeline a
/// partir de \c PipelineConfig y expone \c bind() para enlazar la
/// tuber�a durante el registro de comandos.
class GraphicsPipeline
{
public:
    /// \brief Construye la tuber�a gr�fica a partir de rutas de shaders y configuraci�n.
    /// \param device Dispositivo l�gico Vulkan.
    /// \param vertexPath Ruta del shader de v�rtices en SPIR-V (.spv).
    /// \param fragmentPath Ruta del shader de fragmentos en SPIR-V (.spv).
    /// \param config Estructura \c PipelineConfig completamente rellenada.
    GraphicsPipeline(
        VulkanDevice& device,
        const std::string& vertexPath,
        const std::string& fragmentPath,
        const PipelineConfig& config);

    /// \brief Destruye la \c VkPipeline y los m�dulos de shader.
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    /// \brief Enlaza la tuber�a al \c commandBuffer activo.
    /// \param commandBuffer Command buffer.
    void bind(VkCommandBuffer commandBuffer);

    /// \brief Rellena \c config con valores por defecto razonables.
    /// \details Incluye estado de rasterizaci�n, profundidad, mezcla desactivada,
    /// viewport/scissor como din�micos, input assembly en \c TRIANGLE_LIST, etc.
    /// \param config Referencia a configurar.
    static void defaultConfig(PipelineConfig& config);

    /// \brief Activa la mezcla alfa predecible para el primer color attachment.
    /// \details Ajusta \c colorBlendAttachment y \c colorBlending para
    /// composiciones con transparencia est�ndar.
    /// \param config Referencia a \c PipelineConfig a modificar.
    static void enableAlphaBlending(PipelineConfig& config);

private:
    /// \brief Carga un archivo binario (SPIR-V) a memoria.
    /// \param path Ruta del archivo.
    /// \return Vector de bytes con el contenido del fichero.
    static std::vector<char> readFile(const std::string& path);

    /// \brief Crea la \c VkPipeline con los m�dulos de shader y el \c PipelineConfig.
    /// \param vertexPath Ruta del shader de v�rtices.
    /// \param fragmentPath Ruta del shader de fragmentos.
    /// \param config Configuraci�n completa del pipeline.
    void create(const std::string& vertexPath,
        const std::string& fragmentPath,
        const PipelineConfig& config);

    /// \brief Crea un \c VkShaderModule desde el c�digo SPIR-V dado.
    /// \param code Bytecode SPIR-V.
    /// \param module Salida: handle del m�dulo creado.
    void createShader(const std::vector<char>& code, VkShaderModule* module);

    /// Dispositivo l�gico usado para crear la tuber�a.
    VulkanDevice& device;
    /// Handle de la \c VkPipeline creada.
    VkPipeline pipeline;
    /// M�dulo de shader de v�rtices.
    VkShaderModule vertexModule;
    /// M�dulo de shader de fragmentos.
    VkShaderModule fragmentModule;
};

