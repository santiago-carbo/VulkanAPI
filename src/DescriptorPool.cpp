#include "DescriptorPool.hpp"

#include <stdexcept>

DescriptorPool::DescriptorPool(
    VulkanDevice& device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags flags,
    const std::vector<VkDescriptorPoolSize>& sizes)
    : device{ device } 
{
    VkDescriptorPoolCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = static_cast<uint32_t>(sizes.size());
    info.pPoolSizes = sizes.data();
    info.maxSets = maxSets;
    info.flags = flags;

    if (vkCreateDescriptorPool(device.getDevice(), &info, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("💥[Vulkan API] Could not create descriptor pool.");
    }
}

DescriptorPool::~DescriptorPool() 
{
    vkDestroyDescriptorPool(device.getDevice(), pool, nullptr);
}

bool DescriptorPool::allocate(
    const VkDescriptorSetLayout layout, 
    VkDescriptorSet& descriptor) const 
{
    VkDescriptorSetAllocateInfo alloc {};
    alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc.descriptorPool = pool;
    alloc.pSetLayouts = &layout;
    alloc.descriptorSetCount = 1;

    return (vkAllocateDescriptorSets(device.getDevice(), &alloc, &descriptor) == VK_SUCCESS);
}

void DescriptorPool::free(std::vector<VkDescriptorSet>& descriptors) const 
{
    vkFreeDescriptorSets(
        device.getDevice(),
        pool, 
        static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::reset() 
{
    vkResetDescriptorPool(device.getDevice(), pool, 0);
}