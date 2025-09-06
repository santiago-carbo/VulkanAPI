/*
 * Project: VulkanAPI
 * File: DescriptorSetLayout.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "DescriptorSetLayout.hpp"

#include <cassert>
#include <stdexcept>

 /// \brief Crea el \c VkDescriptorSetLayout a partir de un mapa de bindings.
 /// \param device Dispositivo Vulkan sobre el que se crea el layout.
 /// \param entries Mapa {binding -> \c VkDescriptorSetLayoutBinding} que define el layout.
 /// \post \c layout queda válido hasta la destrucción del objeto.
DescriptorSetLayout::DescriptorSetLayout(
    VulkanDevice& device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> entries)
    : device{device}, entries{entries}
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(entries.size());

    for (const std::pair<const uint32_t, VkDescriptorSetLayoutBinding>& entry : entries)
    {
        layoutBindings.push_back(entry.second);
    }

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    info.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(device.getDevice(), &info, nullptr, &layout) != VK_SUCCESS)
    {
        throw std::runtime_error("💥[Vulkan API] Could not create descriptor set layout.");
    }
}

/// \brief Destruye el \c VkDescriptorSetLayout asociado.
DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(device.getDevice(), layout, nullptr);
}



