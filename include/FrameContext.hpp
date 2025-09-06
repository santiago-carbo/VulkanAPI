/*
 * Project: VulkanAPI
 * File: FrameContext.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "Camera.hpp"
#include "GameObject.hpp"

#include <vulkan/vulkan.h>
#include <unordered_map>

 /// \brief Representación GPU de una luz puntual.
 /// \details Estructura compacta lista para ser copiada a UBO/SSBO.
 /// \c position usa \c w para posibles extensiones (p.ej., radio).
 /// \c color usa \c w para intensidad.
struct GpuPointLight
{
    /// Posición de la luz en espacio de mundo.
    glm::vec4 position {}; 

    /// Color RGB de la luz.
    glm::vec4 color {};    
};

/// \brief UBO global compartido por los shaders.
/// \details Contiene matrices de cámara, luz ambiental y un conjunto acotado
/// de luces puntuales. La disposición y alineación siguen reglas
/// std140, de ahí el \c alignas(16) en el padding final.
struct GlobalUbo
{
    /// Matriz de proyección (perspectiva u ortográfica).
    glm::mat4 projection {1.0f};

    /// Matriz de vista.
    glm::mat4 view {1.0f};           

    /// Inversa de la vista.
    glm::mat4 inverseView {1.0f};    

    /// Luz ambiental (RGB) e intensidad en w.
    glm::vec4 ambientLightColor {1.0f, 1.0f, 1.0f, 0.05f};

    /// Array fijo de luces puntuales.
    GpuPointLight pointLights[10];

    /// Número de luces activas en \c pointLights.
    uint32_t numLights = 0;

    // Relleno para respetar alineación std140 (múltiplos de 16 bytes).
    alignas(16) int _padding[3]{};
};

/// \brief Contexto de datos inmutable por frame que comparten los sistemas de render.
/// \details Agrega índice de frame, tiempo de frame, command buffer, cámara,
/// descriptor set global y referencia a los objetos de escena.
/// Se pasa por referencia a los subsistemas durante el render.
struct FrameInfo
{
    /// Índice del frame en vuelo.
    int frameIndex = 0;

    /// Tiempo del último frame en segundos (delta time).
    float frameTime = 0.0f;

    /// Command buffer primario del frame.
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    /// Cámara activa (vista/proyección).
    Camera& camera;

    /// Descriptor set global (UBO y/o texturas generales).
    VkDescriptorSet globalDescriptorSet = VK_NULL_HANDLE;

    /// Objetos de escena direccionados por id.
    std::unordered_map<unsigned int, GameObject>& gameObjects;
};

