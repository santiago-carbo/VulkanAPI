/*
 * Project: VulkanAPI
 * File: Window.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "Window.hpp"

#include <stdexcept>

 /// \brief Construye la ventana de la aplicación.
 /// \details Inicializa GLFW si es necesario y registra el callback de redimensionado.
 /// \param w Anchura inicial en píxeles.
 /// \param h Altura inicial en píxeles.
 /// \param name Título de la ventana.
Window::Window(int w, int h, std::string name) : width{w}, height{h}, windowName{name} 
{
  initWindow();
}

/// \brief Destruye la ventana y libera los recursos asociados de GLFW.
Window::~Window() 
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

/// \brief Inicializa la ventana, el contexto y las hints de GLFW.
void Window::initWindow() 
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

/// \brief Crea la \c VkSurfaceKHR asociada a esta ventana.
/// \details Debe llamarse tras crear la instancia de Vulkan. La surface
/// es necesaria para crear la swapchain.
/// \param instance Instancia de Vulkan.
/// \param surface Salida con la surface creada.
void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) 
{
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) 
  {
    throw std::runtime_error("💥[Vulkan API] Failed to create window surface.");
  }
}

/// \brief Callback estático de GLFW para actualizar tamaño y marcar redimensionado.
/// \param window Puntero a la ventana GLFW.
/// \param width Nueva anchura del framebuffer.
/// \param height Nueva altura del framebuffer.
void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) 
{
  Window* activeWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  activeWindow->framebufferResized = true;
  activeWindow->width = width;
  activeWindow->height = height;
}
