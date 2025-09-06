/*
 * Project: VulkanAPI
 * File: VulkanApplication.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "DescriptorPool.hpp"
#include "VulkanDevice.hpp"
#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "EditorUI.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

 /// \brief Punto de entrada de alto nivel de la aplicación Vulkan.
 /// \details Orquesta la inicialización de los subsistemas principales
 /// (dispositivo, renderizador, pool de descriptores y UI), gestiona la
 /// creación/carga de objetos de escena y ejecuta el bucle principal de
 /// render hasta el cierre de la ventana. Se encarga también de la
 /// liberación ordenada de recursos al finalizar.
class VulkanApplication
{
public:
    /// \brief Construye la aplicación y prepara los componentes básicos.
    /// \details No inicia el bucle de ejecución. La inicialización
    /// completa se realiza en \c run .
    VulkanApplication();

    /// \brief Libera los recursos administrados por la aplicación.
    ~VulkanApplication();

    VulkanApplication(const VulkanApplication&) = delete;
    VulkanApplication& operator=(const VulkanApplication&) = delete;

    /// \brief Ejecuta la aplicación.
    /// \details Lleva a cabo la inicialización restante, entra en el
    /// bucle principal de render y procesa eventos hasta que el usuario
    /// cierra la ventana. Al salir, sincroniza y limpia recursos.
    void run();

private:
    /// \brief Carga y registra los \c GameObject de la escena.
    /// \details Construye la geometría y las luces necesarias para
    /// validar el motor, añadiéndolas al contenedor \c gameObjects .
    void loadGameObjects();

    /// \brief Interfaz de usuario basada en Dear ImGui.
    EditorUI editorUI;

    /// \brief Dispositivo Vulkan y recursos asociados.
    std::unique_ptr<VulkanDevice> vulkanDevice;

    /// \brief Encapsula la lógica de render y la swapchain.
    std::unique_ptr<Renderer> renderer;

    /// \brief Pool de descriptores global para conjuntos recurrentes.
    std::unique_ptr<DescriptorPool> globalPool;

    /// \brief Contenedor de objetos de escena indexados por identificador.
    std::unordered_map<unsigned int, GameObject> gameObjects;
};


