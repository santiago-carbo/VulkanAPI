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

struct GpuPointLight 
{
	glm::vec4 position {};
	glm::vec4 color {};
};

struct GlobalUbo 
{
	glm::mat4 projection {1.0f};
	glm::mat4 view {1.0f};
	glm::mat4 inverseView {1.0f};
	glm::vec4 ambientLightColor {1.0f, 1.0f, 1.0f, 0.05f};
	GpuPointLight pointLights[10];
	uint32_t numLights = 0;

	alignas(16) int _padding[3] {};
};

struct FrameInfo 
{
	int frameIndex = 0;
	float frameTime = 0.0f;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	Camera& camera;
	VkDescriptorSet globalDescriptorSet = VK_NULL_HANDLE;
	std::unordered_map<unsigned int, GameObject>& gameObjects;
};
