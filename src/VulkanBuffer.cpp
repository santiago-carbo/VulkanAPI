/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "VulkanBuffer.hpp"

#include <cassert>
#include <cstring>

 /**
  * Calculates the aligned size for an instance given a required minimum alignment.
  *
  * @param instanceSize Size of a single instance in bytes.
  * @param minOffsetAlignment Required minimum alignment in bytes.
  * @return Aligned size that satisfies the given alignment constraint.
  */
VkDeviceSize VulkanBuffer::getAlignment(
    VkDeviceSize instanceSize,
    VkDeviceSize minOffsetAlignment)
{
    if (minOffsetAlignment > 0)
    {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    }

    return (instanceSize);
}

VulkanBuffer::VulkanBuffer(
    VulkanDevice& device,
    VkDeviceSize instanceSize,
    uint32_t instanceCount,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkDeviceSize minOffsetAlignment)
    : vulkanDevice{device},
    instanceSize{instanceSize},
    instanceCount{instanceCount},
    usageFlags{usageFlags},
    memoryPropertyFlags{memoryPropertyFlags}
{
    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    bufferSize = alignmentSize * instanceCount;
    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
}

VulkanBuffer::~VulkanBuffer()
{
    unmap();
    vkDestroyBuffer(vulkanDevice.getDevice(), buffer, nullptr);
    vkFreeMemory(vulkanDevice.getDevice(), memory, nullptr);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
VkResult VulkanBuffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    assert(buffer && memory && "💥[Vulkan API] Called map on buffer before create.");

    return (vkMapMemory(vulkanDevice.getDevice(), memory, offset, size, 0, &mapped));
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void VulkanBuffer::unmap()
{
    if (mapped)
    {
        vkUnmapMemory(vulkanDevice.getDevice(), memory);
        mapped = nullptr;
    }
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void VulkanBuffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
{
    assert(mapped && "💥[Vulkan API] Cannot copy to unmapped buffer.");

    if (size == VK_WHOLE_SIZE)
    {
        memcpy(mapped, data, bufferSize);
    }
    else
    {
        char* memOffset = (char*) mapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return (vkFlushMappedMemoryRanges(vulkanDevice.getDevice(), 1, &mappedRange));
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return (vkInvalidateMappedMemoryRanges(vulkanDevice.getDevice(), 1, &mappedRange));
}

/**
 * Generates a VkDescriptorBufferInfo object to describe this buffer for descriptor binding.
 *
 * @param size Size of the buffer region. Defaults to VK_WHOLE_SIZE.
 * @param offset Offset from the start of the buffer. Defaults to 0.
 * @return A filled VkDescriptorBufferInfo struct.
 */
VkDescriptorBufferInfo VulkanBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
    return (VkDescriptorBufferInfo{buffer, offset, size});
}

/**
 * Copies one instance (with size equal to instanceSize) into the mapped buffer at a given index.
 *
 * @param data Pointer to the data to copy.
 * @param index Target index to copy the data to.
 */
void VulkanBuffer::writeToIndex(void* data, int index)
{
    writeToBuffer(data, instanceSize, index * alignmentSize);
}

/**
 * Flushes the memory region associated with the specified index.
 *
 * @param index Index of the instance to flush.
 * @return VkResult of the flush call.
 */
VkResult VulkanBuffer::flushIndex(int index)
{
    return (flush(alignmentSize, index * alignmentSize));
}

/**
 * Returns a descriptor buffer info pointing to the memory region of a specific index.
 *
 * @param index Index of the instance.
 * @return A VkDescriptorBufferInfo struct for the selected instance.
 */
VkDescriptorBufferInfo VulkanBuffer::descriptorInfoForIndex(int index)
{
    return (descriptorInfo(alignmentSize, index * alignmentSize));
}

/**
 * Invalidates the memory range of a specific index.
 *
 * @param index Index of the instance to invalidate.
 * @return VkResult of the invalidate call.
 */
VkResult VulkanBuffer::invalidateIndex(int index)
{
    return (invalidate(alignmentSize, index * alignmentSize));
}
