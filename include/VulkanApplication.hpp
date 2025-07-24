#pragma once

#include "DescriptorPool.hpp"
#include "VulkanDevice.hpp"
#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

#include <memory>
#include <vector>
#include <unordered_map>


class VulkanApplication 
{
	public:
		static constexpr int WIDTH = 1600;
		static constexpr int HEIGHT = 900;

		VulkanApplication();
		~VulkanApplication();

		VulkanApplication(const VulkanApplication&) = delete;
		VulkanApplication& operator=(const VulkanApplication&) = delete;

		void run();

	private:
		void loadGameObjects();

		Window window {WIDTH, HEIGHT, "Vulkan API"};
		VulkanDevice vulkanDevice {window};
		Renderer renderer {window, vulkanDevice};
		std::unique_ptr<DescriptorPool> globalPool;
		std::unordered_map<unsigned int, GameObject> gameObjects;
};

