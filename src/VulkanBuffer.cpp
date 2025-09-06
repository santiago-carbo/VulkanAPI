/*
 * Project: VulkanAPI
 * File: VulkanBuffer.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */


#include "VulkanBuffer.hpp"

#include <cassert>
#include <cstring>

 /// \brief Calcula el tamaño alineado por instancia.
 /// \details Si \c minOffsetAlignment es mayor que cero, redondea \c instanceSize
 /// al múltiplo más cercano requerido por el dispositivo.
 /// \param instanceSize Tamaño base de la instancia.
 /// \param minOffsetAlignment Alineación mínima exigida por el dispositivo.
 /// \return Tamaño alineado por instancia.
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

/// \brief Crea un búfer y reserva su memoria en el dispositivo.
/// \details Configura tamaño total en función de \c instanceSize , \c instanceCount
/// y la alineación mínima requerida. No mapea la memoria de forma automática.
/// \param device Dispositivo lógico Vulkan.
/// \param instanceSize Tamaño de una instancia lógica almacenada en el búfer.
/// \param instanceCount Número de instancias almacenadas consecutivamente.
/// \param usageFlags Uso del búfer (por ejemplo \c VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT).
/// \param memoryPropertyFlags Propiedades de la memoria (por ejemplo \c HOST_VISIBLE).
/// \param minOffsetAlignment Alineación mínima por instancia. Use 1 si no aplica.
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

/// \brief Libera el búfer y su memoria.
VulkanBuffer::~VulkanBuffer()
{
    unmap();
    vkDestroyBuffer(vulkanDevice.getDevice(), buffer, nullptr);
    vkFreeMemory(vulkanDevice.getDevice(), memory, nullptr);
}

/// \brief Mapea la memoria del búfer para acceso CPU.
/// \details Si \c size es \c VK_WHOLE_SIZE se mapea el rango completo.
/// \param size Tamaño a mapear en bytes.
/// \param offset Desplazamiento inicial del mapeo.
/// \return Resultado Vulkan de la operación de mapeo.
VkResult VulkanBuffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    assert(buffer && memory && "💥[Vulkan API] Called map on buffer before create.");

    return (vkMapMemory(vulkanDevice.getDevice(), memory, offset, size, 0, &mapped));
}

/// \brief Desmapea la memoria previamente mapeada.
void VulkanBuffer::unmap()
{
    if (mapped)
    {
        vkUnmapMemory(vulkanDevice.getDevice(), memory);
        mapped = nullptr;
    }
}

/// \brief Copia datos desde CPU al rango mapeado del búfer.
/// \details Requiere el búfer mapeado. No realiza \c vkFlushMappedMemoryRanges.
/// \param data Puntero al bloque de memoria origen.
/// \param size Tamaño a escribir. Use \c VK_WHOLE_SIZE para el total.
/// \param offset Desplazamiento dentro del búfer de destino.
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

/// \brief Sincroniza escrituras de host hacia la GPU.
/// \details Necesario para memorias no coherentes. Para coherentes no es obligatorio.
/// \param size Tamaño del rango a sincronizar.
/// \param offset Desplazamiento inicial del rango.
/// \return Resultado Vulkan de la operación de \c flush.
VkResult VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return (vkFlushMappedMemoryRanges(vulkanDevice.getDevice(), 1, &mappedRange));
}

/// \brief Sincroniza lecturas de host desde la GPU.
/// \details Necesario para memorias no coherentes antes de leer cambios desde CPU.
/// \param size Tamaño del rango a invalidar.
/// \param offset Desplazamiento inicial del rango.
/// \return Resultado Vulkan de la operación de \c invalidate.
VkResult VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;

    return (vkInvalidateMappedMemoryRanges(vulkanDevice.getDevice(), 1, &mappedRange));
}

/// \brief Devuelve la información de descriptor para enlazar el búfer.
/// \details Útil para \c VkWriteDescriptorSet con UBOs o SSBOs.
/// \param size Tamaño del rango expuesto al shader.
/// \param offset Desplazamiento del rango expuesto.
/// \return Estructura \c VkDescriptorBufferInfo lista para usar.
VkDescriptorBufferInfo VulkanBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
    return (VkDescriptorBufferInfo{buffer, offset, size});
}

/// \brief Escribe una instancia concreta en el búfer alineado.
/// \details Calcula el desplazamiento como \c index * alignmentSize.
/// \param data Puntero a los datos de la instancia.
/// \param index Índice de instancia destino.
void VulkanBuffer::writeToIndex(void* data, int index)
{
    writeToBuffer(data, instanceSize, index * alignmentSize);
}

/// \brief Realiza \c flush del rango correspondiente a una instancia.
/// \param index Índice de instancia.
/// \return Resultado Vulkan de la operación.
VkResult VulkanBuffer::flushIndex(int index)
{
    return (flush(alignmentSize, index * alignmentSize));
}

/// \brief Devuelve \c VkDescriptorBufferInfo para una instancia concreta.
/// \param index Índice de instancia.
/// \return Descriptor limitado al subrango de la instancia.
VkDescriptorBufferInfo VulkanBuffer::descriptorInfoForIndex(int index)
{
    return (descriptorInfo(alignmentSize, index * alignmentSize));
}

/// \brief Invalida el rango correspondiente a una instancia.
/// \param index Índice de instancia.
/// \return Resultado Vulkan de la operación.
VkResult VulkanBuffer::invalidateIndex(int index)
{
    return (invalidate(alignmentSize, index * alignmentSize));
}
