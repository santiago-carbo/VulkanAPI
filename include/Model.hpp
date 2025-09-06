/*
 * Project: VulkanAPI
 * File: Model.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>

 /// \brief Malla renderizable: encapsula buffers de vértices/índices en Vulkan.
 /// \details Carga/crea los datos de geometría (posiciones, normales, color y UV),
 /// construye \c VkBuffer para vértices e índices y expone \c bind()/\c draw()
 /// para emitir los comandos de dibujo. Ofrece un \c Builder para cargar desde archivo.
class Model
{
public:
    /// \brief Formato de vértice usado por la tubería gráfica.
    /// \details Incluye atributos típicos: posición, color, normal y UV.
    /// Proporciona descriptores de binding y atributos para el pipeline.
    struct Vertex
    {
        /// Posición del vértice (x,y,z).
        glm::vec3 position {};
        /// Color del vértice (r,g,b) en [0,1].
        glm::vec3 color {};
        /// Normal del vértice (x,y,z).
        glm::vec3 normal {};
        /// Coordenadas de textura (u,v).
        glm::vec2 uv {};

        /// \brief Descriptores de binding para el pipeline.
        /// \return Vector con la única entrada de binding usada por este formato.
        static std::vector<VkVertexInputBindingDescription> bindingDescriptions();

        /// \brief Descriptores de atributos para el pipeline.
        /// \return Vector con las descripciones de posición, color, normal y UV.
        static std::vector<VkVertexInputAttributeDescription> attributeDescriptions();

        /// \brief Comparación exacta de vértices (para eliminar duplicados).
        bool operator==(const Vertex& other) const
        {
            return ((position == other.position) && (color == other.color) &&
                (normal == other.normal) && (uv == other.uv));
        }
    };

    /// \brief Constructor de datos de malla antes de crear los buffers GPU.
    /// \details Acumula vértices e índices en CPU. \c loadFromFile() permite
    /// cargar formatos soportados y rellenar \c vertices/\c indices.
    struct Builder
    {
        /// Lista de vértices en CPU.
        std::vector<Vertex> vertices {};
        /// Lista de índices (de los triángulos).
        std::vector<uint32_t> indices {};

        /// \brief Carga la malla desde un fichero en disco.
        /// \param filepath Ruta del fichero.
        /// \post \c vertices y \c indices quedan poblados.
        void loadFromFile(const std::string& filepath);
    };

    /// \brief Crea la malla en GPU a partir de los datos del \c Builder.
    /// \param device Dispositivo lógico Vulkan.
    /// \param builder Datos de vértices/índices ya cargados en CPU.
    Model(VulkanDevice& device, const Builder& builder);

    /// \brief Libera los buffers de GPU asociados a la malla.
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    /// \brief Carga desde archivo y crea la malla directamente.
    /// \param device Dispositivo lógico Vulkan.
    /// \param filepath Ruta del fichero.
    /// \return \c unique_ptr a \c Model ya inicializado.
    static std::unique_ptr<Model> fromFile(VulkanDevice& device, const std::string& filepath);

    /// \brief Enlaza los vertex/index buffers al \c commandBuffer.
    /// \param commandBuffer Command buffer en el que se están grabando comandos.
    void bind(VkCommandBuffer commandBuffer);

    /// \brief Emite la orden de dibujo (indexed o no) sobre el \c commandBuffer.
    /// \param commandBuffer Command buffer en el que se están grabando comandos.
    void draw(VkCommandBuffer commandBuffer);

private:
    /// \brief Crea el \c VkBuffer de vértices y transfiere los datos desde CPU.
    /// \param vertices Vector de vértices.
    void createVertexBuffer(const std::vector<Vertex>& vertices);

    /// \brief Crea el \c VkBuffer de índices y transfiere los datos desde CPU.
    /// \param indices Vector de índices (triángulos).
    void createIndexBuffer(const std::vector<uint32_t>& indices);

    /// Dispositivo lógico para crear/destruir buffers.
    VulkanDevice& device;
    /// Buffer de vértices en GPU.
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    /// Número de vértices.
    uint32_t vertexCount = 0;
    /// Indica si hay índice.
    bool useIndexBuffer = false;
    /// Buffer de índices en GPU.
    std::unique_ptr<VulkanBuffer> indexBuffer;
    /// Número de índices (múltiplo de 3 si son triángulos).
    uint32_t indexCount = 0;
};

