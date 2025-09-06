/*
 * Project: VulkanAPI
 * File: Window.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

 /// \brief Abstracción ligera de la ventana y eventos de GLFW.
 /// \details Crea y gestiona una ventana con contexto para Vulkan, expone
 /// utilidades para consultar tamaño, detectar redimensionados y crear la
 /// \c VkSurfaceKHR necesaria para la presentación.
class Window
{
public:
    /// \brief Construye la ventana de la aplicación.
    /// \details Inicializa GLFW si es necesario y registra el callback de redimensionado.
    /// \param w Anchura inicial en píxeles.
    /// \param h Altura inicial en píxeles.
    /// \param name Título de la ventana.
    Window(int w, int h, std::string name);

    /// \brief Destruye la ventana y libera los recursos asociados de GLFW.
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /// \brief Indica si el usuario ha solicitado cerrar la ventana.
    /// \return \c true si se ha activado la orden de cierre.
    bool shouldClose()
    {
        return glfwWindowShouldClose(window);
    }

    /// \brief Devuelve la extensión actual de la ventana en formato Vulkan.
    /// \details El valor reflejará el último tamaño del framebuffer.
    /// \return Estructura \c VkExtent2D con anchura y altura.
    VkExtent2D getExtent()
    {
        return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }

    /// \brief Indica si el framebuffer ha sido redimensionado desde el último frame.
    /// \details Útil para invalidar y recrear la swapchain de forma controlada.
    /// \return \c true si hubo redimensionado.
    bool wasWindowResized()
    {
        return framebufferResized;
    }

    /// \brief Reinicia la marca de redimensionado tras procesarla en el render.
    void resetWindowResizedFlag()
    {
        framebufferResized = false;
    }

    /// \brief Acceso directo al puntero \c GLFWwindow para integraciones externas.
    /// \return Puntero a \c GLFWwindow.
    GLFWwindow* getGLFWwindow() const
    {
        return window;
    }

    /// \brief Crea la \c VkSurfaceKHR asociada a esta ventana.
    /// \details Debe llamarse tras crear la instancia de Vulkan. La surface
    /// es necesaria para crear la swapchain.
    /// \param instance Instancia de Vulkan.
    /// \param surface Salida con la surface creada.
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
    /// \brief Callback estático de GLFW para actualizar tamaño y marcar redimensionado.
    /// \param window Puntero a la ventana GLFW.
    /// \param width Nueva anchura del framebuffer.
    /// \param height Nueva altura del framebuffer.
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    /// \brief Inicializa la ventana, el contexto y las hints de GLFW.
    void initWindow();

    /// Anchura actual de la ventana en píxeles.
    int width;

    /// Altura actual de la ventana en píxeles.
    int height;

    /// Indica si el framebuffer ha sido redimensionado desde el último frame.
    bool framebufferResized = false;

    /// Título de la ventana mostrado por el sistema.
    std::string windowName;

    /// Puntero nativo a la ventana de GLFW.
    GLFWwindow* window;
};

