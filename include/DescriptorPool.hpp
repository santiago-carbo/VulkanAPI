/*
 * Project: VulkanAPI
 * File: DescriptorPool.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

#include <memory>


class DescriptorPool 
{
    public:
        DescriptorPool(
            VulkanDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags flags,
            const std::vector<VkDescriptorPoolSize>& sizes);

        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        bool allocate(const VkDescriptorSetLayout layout, VkDescriptorSet& descriptor) const;
        void free(std::vector<VkDescriptorSet>& descriptors) const;
        void reset();

        const VulkanDevice& getDevice() const 
        { 
            return device; 
        }

    private:
        VulkanDevice& device;
        VkDescriptorPool pool;
};