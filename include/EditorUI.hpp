/*
 * Project: VulkanAPI
 * File: EditorUI.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include "GameObject.hpp"

class Perf;

/// \brief Capa de interfaz de usuario basada en Dear ImGui.
/// \details Inicializa ImGui para GLFW+Vulkan, gestiona el descriptor pool propio
/// de ImGui, abre/cierra el frame de UI y dibuja paneles básicos de
/// inspección/edición (por ejemplo, transformaciones de GameObjects).
/// Tiene integrada \c Perf para mostrar métricas.
class EditorUI
{
    public:
        /// \brief Dimensiones iniciales de la ventana del editor.
        static constexpr int WIDTH = 1600;
        static constexpr int HEIGHT = 900;

        /// \brief Constructor por defecto.
        EditorUI();

        /// \brief Destructor.
        ~EditorUI();

        /// \brief Inicializa el backend de ImGui para Vulkan y GLFW.
        /// \details Crea contexto ImGui, configura estilo, inicializa \c ImGui_ImplGlfw
        /// y \c ImGui_ImplVulkan y crea un descriptor pool propio para UI.
        /// \param instance Instancia de Vulkan.
        /// \param physicalDevice Dispositivo físico empleado por la aplicación.
        /// \param device Dispositivo lógico (VkDevice).
        /// \param graphicsQueueFamily Índice de la cola gráfica.
        /// \param graphicsQueue Cola gráfica donde se enviarán comandos de UI.
        /// \param renderPass Render pass destino (de la swapchain) para dibujar la UI.
        /// \param imageCount Número de imágenes en la swapchain (para configurar ImGui).
        void init(VkInstance instance,
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            uint32_t graphicsQueueFamily,
            VkQueue graphicsQueue,
            VkRenderPass renderPass,
            uint32_t imageCount);

        /// \brief Comienza un frame de ImGui.
        /// \details Llama a \c ImGui_ImplVulkan_NewFrame, \c ImGui_ImplGlfw_NewFrame
        /// y \c ImGui::NewFrame(). Debe invocarse una vez por frame antes de
        /// construir los paneles.
        void beginFrame();

        /// \brief Dibuja un panel de inspección/edición para \c gameObjects.
        /// \details Recorre el mapa y muestra sliders para posición/rotación/escala.
        /// \param gameObjects Contenedor de objetos de escena a inspeccionar/editar.
        void drawGameObjects(std::unordered_map<unsigned int, GameObject>& gameObjects);

        /// \brief Finaliza y emite los draw calls de ImGui al \c commandBuffer.
        /// \details Llama a \c ImGui::Render() y \c ImGui_ImplVulkan_RenderDrawData()
        /// para grabar los comandos de UI en el command buffer del frame.
        /// \param commandBuffer Command buffer actual (en un render pass activo).
        void endFrame(VkCommandBuffer commandBuffer);

        /// \brief Libera los recursos del backend de ImGui y su descriptor pool.
        /// \param device VkDevice con el que se creó el descriptor pool.
        void cleanup(VkDevice device);

        /// \brief Inyecta el recolector de métricas de rendimiento.
        /// \details Permite a la UI consultar/mostrar estadísticas (CPU/GPU/FPS).
        /// \param p Puntero a \c Perf válido mientras se use la UI.
        void setPerf(Perf* p)
        {
            perf = p;
        }

        /// \brief Acceso a la ventana principal (GLFW).
        /// \return Referencia a la ventana propietaria de la UI.
        Window& getWindow()
        {
            return window;
        }

    private:
        /// \brief Crea el descriptor pool específico que usa ImGui internamente.
        /// \param device Dispositivo lógico Vulkan.
        void createDescriptorPool(VkDevice device);

        /// Pool de descriptores para ImGui.
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

        /// Ventana GLFW de la aplicación/editor.
        Window window{WIDTH, HEIGHT, "Vulkan API"};

        /// Métricas de rendimiento.
        Perf* perf = nullptr;
};


