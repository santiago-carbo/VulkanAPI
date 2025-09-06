/*
 * Project: VulkanAPI
 * File: Model.hpp
 * Author: Santiago Carb� Garc�a
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

 /// \brief Malla renderizable: encapsula buffers de v�rtices/�ndices en Vulkan.
 /// \details Carga/crea los datos de geometr�a (posiciones, normales, color y UV),
 /// construye \c VkBuffer para v�rtices e �ndices y expone \c bind()/\c draw()
 /// para emitir los comandos de dibujo. Ofrece un \c Builder para cargar desde archivo.
class Model
{
public:
    /// \brief Formato de v�rtice usado por la tuber�a gr�fica.
    /// \details Incluye atributos t�picos: posici�n, color, normal y UV.
    /// Proporciona descriptores de binding y atributos para el pipeline.
    struct Vertex
    {
        /// Posici�n del v�rtice (x,y,z).
        glm::vec3 position {};
        /// Color del v�rtice (r,g,b) en [0,1].
        glm::vec3 color {};
        /// Normal del v�rtice (x,y,z).
        glm::vec3 normal {};
        /// Coordenadas de textura (u,v).
        glm::vec2 uv {};

        /// \brief Descriptores de binding para el pipeline.
        /// \return Vector con la �nica entrada de binding usada por este formato.
        static std::vector<VkVertexInputBindingDescription> bindingDescriptions();

        /// \brief Descriptores de atributos para el pipeline.
        /// \return Vector con las descripciones de posici�n, color, normal y UV.
        static std::vector<VkVertexInputAttributeDescription> attributeDescriptions();

        /// \brief Comparaci�n exacta de v�rtices (para eliminar duplicados).
        bool operator==(const Vertex& other) const
        {
            return ((position == other.position) && (color == other.color) &&
                (normal == other.normal) && (uv == other.uv));
        }
    };

    /// \brief Constructor de datos de malla antes de crear los buffers GPU.
    /// \details Acumula v�rtices e �ndices en CPU. \c loadFromFile() permite
    /// cargar formatos soportados y rellenar \c vertices/\c indices.
    struct Builder
    {
        /// Lista de v�rtices en CPU.
        std::vector<Vertex> vertices {};
        /// Lista de �ndices (de los tri�ngulos).
        std::vector<uint32_t> indices {};

        /// \brief Carga la malla desde un fichero en disco.
        /// \param filepath Ruta del fichero.
        /// \post \c vertices y \c indices quedan poblados.
        void loadFromFile(const std::string& filepath);
    };

    /// \brief Crea la malla en GPU a partir de los datos del \c Builder.
    /// \param device Dispositivo l�gico Vulkan.
    /// \param builder Datos de v�rtices/�ndices ya cargados en CPU.
    Model(VulkanDevice& device, const Builder& builder);

    /// \brief Libera los buffers de GPU asociados a la malla.
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    /// \brief Carga desde archivo y crea la malla directamente.
    /// \param device Dispositivo l�gico Vulkan.
    /// \param filepath Ruta del fichero.
    /// \return \c unique_ptr a \c Model ya inicializado.
    static std::unique_ptr<Model> fromFile(VulkanDevice& device, const std::string& filepath);

    /// \brief Enlaza los vertex/index buffers al \c commandBuffer.
    /// \param commandBuffer Command buffer en el que se est�n grabando comandos.
    void bind(VkCommandBuffer commandBuffer);

    /// \brief Emite la orden de dibujo (indexed o no) sobre el \c commandBuffer.
    /// \param commandBuffer Command buffer en el que se est�n grabando comandos.
    void draw(VkCommandBuffer commandBuffer);

private:
    /// \brief Crea el \c VkBuffer de v�rtices y transfiere los datos desde CPU.
    /// \param vertices Vector de v�rtices.
    void createVertexBuffer(const std::vector<Vertex>& vertices);

    /// \brief Crea el \c VkBuffer de �ndices y transfiere los datos desde CPU.
    /// \param indices Vector de �ndices (tri�ngulos).
    void createIndexBuffer(const std::vector<uint32_t>& indices);

    /// Dispositivo l�gico para crear/destruir buffers.
    VulkanDevice& device;
    /// Buffer de v�rtices en GPU.
    std::unique_ptr<VulkanBuffer> vertexBuffer;
    /// N�mero de v�rtices.
    uint32_t vertexCount = 0;
    /// Indica si hay �ndice.
    bool useIndexBuffer = false;
    /// Buffer de �ndices en GPU.
    std::unique_ptr<VulkanBuffer> indexBuffer;
    /// N�mero de �ndices (m�ltiplo de 3 si son tri�ngulos).
    uint32_t indexCount = 0;
};

