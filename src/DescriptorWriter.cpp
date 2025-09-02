/*
 * Project: VulkanAPI
 * File: DescriptorWriter.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "DescriptorWriter.hpp"

#include <cassert>

DescriptorWriter::DescriptorWriter(DescriptorSetLayout& layout, DescriptorPool& pool)
    : layout{layout}, pool{pool} {}

DescriptorWriter& DescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo* bufferInfo) 
{
    assert(
        layout.entries.count(binding) == 1 &&
        "💥[Vulkan API] Descriptor set layout does not contain the specified binding.");

    const VkDescriptorSetLayoutBinding& bindingInfo = layout.getBinding(binding);

    assert(
        bindingInfo.descriptorCount == 1 &&
        "💥[Vulkan API] Provided only one descriptor, but binding requires an array.");

    VkWriteDescriptorSet write {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingInfo.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    pendingWrites.push_back(write);
    
    return (*this);
}

DescriptorWriter& DescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo* imageInfo) 
{
    assert(
        layout.entries.count(binding) == 1 &&
        "💥[Vulkan API] Descriptor set layout does not contain the specified binding.");

    const VkDescriptorSetLayoutBinding& bindingInfo = layout.getBinding(binding);

    assert(
        bindingInfo.descriptorCount == 1 &&
        "💥[Vulkan API] Provided only one descriptor, but binding requires an array.");

    VkWriteDescriptorSet write {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingInfo.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    pendingWrites.push_back(write);
    
    return (*this);
}

bool DescriptorWriter::build(VkDescriptorSet& set) 
{
    bool success = pool.allocate(layout.get(), set);
    
    if (!success)
    {
        return false;
    }
        
    overwrite(set);
    return (true);
}

void DescriptorWriter::overwrite(VkDescriptorSet& set) 
{
    for (VkWriteDescriptorSet& write : pendingWrites) 
    {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(
        pool.getDevice().getDevice(),
        static_cast<uint32_t>(pendingWrites.size()),
        pendingWrites.data(),
        0,
        nullptr);
}