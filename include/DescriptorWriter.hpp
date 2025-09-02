/*
 * Project: VulkanAPI
 * File: DescriptorWriter.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "DescriptorSetLayout.hpp"
#include "DescriptorPool.hpp"

#include <vector>

class DescriptorWriter 
{
	public:
		DescriptorWriter(DescriptorSetLayout& layout, DescriptorPool& pool);

		DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		bool build(VkDescriptorSet& set);
		void overwrite(VkDescriptorSet& set);

	private:
		DescriptorSetLayout& layout;
		DescriptorPool& pool;
		std::vector<VkWriteDescriptorSet> pendingWrites;
};