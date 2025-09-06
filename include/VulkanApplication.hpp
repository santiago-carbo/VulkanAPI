/*
 * Project: VulkanAPI
 * File: VulkanApplication.hpp
 * Author: Santiago Carb� Garc�a
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

 /// \brief Punto de entrada de alto nivel de la aplicaci�n Vulkan.
 /// \details Orquesta la inicializaci�n de los subsistemas principales
 /// (dispositivo, renderizador, pool de descriptores y UI), gestiona la
 /// creaci�n/carga de objetos de escena y ejecuta el bucle principal de
 /// render hasta el cierre de la ventana. Se encarga tambi�n de la
 /// liberaci�n ordenada de recursos al finalizar.
class VulkanApplication
{
public:
    /// \brief Construye la aplicaci�n y prepara los componentes b�sicos.
    /// \details No inicia el bucle de ejecuci�n. La inicializaci�n
    /// completa se realiza en \c run .
    VulkanApplication();

    /// \brief Libera los recursos administrados por la aplicaci�n.
    ~VulkanApplication();

    VulkanApplication(const VulkanApplication&) = delete;
    VulkanApplication& operator=(const VulkanApplication&) = delete;

    /// \brief Ejecuta la aplicaci�n.
    /// \details Lleva a cabo la inicializaci�n restante, entra en el
    /// bucle principal de render y procesa eventos hasta que el usuario
    /// cierra la ventana. Al salir, sincroniza y limpia recursos.
    void run();

private:
    /// \brief Carga y registra los \c GameObject de la escena.
    /// \details Construye la geometr�a y las luces necesarias para
    /// validar el motor, a�adi�ndolas al contenedor \c gameObjects .
    void loadGameObjects();

    /// \brief Interfaz de usuario basada en Dear ImGui.
    EditorUI editorUI;

    /// \brief Dispositivo Vulkan y recursos asociados.
    std::unique_ptr<VulkanDevice> vulkanDevice;

    /// \brief Encapsula la l�gica de render y la swapchain.
    std::unique_ptr<Renderer> renderer;

    /// \brief Pool de descriptores global para conjuntos recurrentes.
    std::unique_ptr<DescriptorPool> globalPool;

    /// \brief Contenedor de objetos de escena indexados por identificador.
    std::unordered_map<unsigned int, GameObject> gameObjects;
};


