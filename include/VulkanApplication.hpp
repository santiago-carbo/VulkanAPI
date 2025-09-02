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


class VulkanApplication 
{
	public:
		VulkanApplication();
		~VulkanApplication();

		VulkanApplication(const VulkanApplication&) = delete;
		VulkanApplication& operator=(const VulkanApplication&) = delete;

		void run();

	private:
		void loadGameObjects();

		EditorUI editorUI;

		std::unique_ptr<VulkanDevice> vulkanDevice;
		std::unique_ptr<Renderer> renderer;

		std::unique_ptr<DescriptorPool> globalPool;
		std::unordered_map<unsigned int, GameObject> gameObjects;
};

