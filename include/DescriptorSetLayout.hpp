/*
 * Project: VulkanAPI
 * File: DescriptorSetLayout.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

#include <memory>
#include <unordered_map>

class DescriptorSetLayout 
{
    public:
        DescriptorSetLayout(
            VulkanDevice& device,
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> entries);

        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout&) = delete;

        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout get() const 
        { 
            return layout; 
        }

        const VkDescriptorSetLayoutBinding& getBinding(uint32_t binding) const 
        {
            return entries.at(binding);
        }

    private:
        VulkanDevice& device;
        VkDescriptorSetLayout layout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> entries;
};