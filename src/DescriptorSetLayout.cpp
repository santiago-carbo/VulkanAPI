#include "DescriptorSetLayout.hpp"

#include <cassert>
#include <stdexcept>

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

    VkDescriptorSetLayoutCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    info.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(device.getDevice(), &info, nullptr, &layout) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Could not create descriptor set layout.");
    }
}

DescriptorSetLayout::~DescriptorSetLayout() 
{
    vkDestroyDescriptorSetLayout(device.getDevice(), layout, nullptr);
}


