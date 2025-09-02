/*
 * Project: VulkanAPI
 * File: Window.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "Window.hpp"

#include <stdexcept>

Window::Window(int w, int h, std::string name) : width{w}, height{h}, windowName{name} 
{
  initWindow();
}

Window::~Window() 
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::initWindow() 
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) 
{
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) 
  {
    throw std::runtime_error("💥[Vulkan API] Failed to create window surface.");
  }
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) 
{
  Window* activeWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  activeWindow->framebufferResized = true;
  activeWindow->width = width;
  activeWindow->height = height;
}
