/*
 * Project: VulkanAPI
 * File: DescriptorWriter.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "DescriptorWriter.hpp"

#include <cassert>

 /// \brief Crea un escritor asociado a un layout y a un pool.
 /// \param layout Layout de descriptores que define los bindings válidos.
 /// \param pool Pool desde el que se asignarán los \c VkDescriptorSet.
DescriptorWriter::DescriptorWriter(DescriptorSetLayout& layout, DescriptorPool& pool)
    : layout{layout}, pool{pool} {}

/// \brief Añade una escritura pendiente de tipo buffer al binding indicado.
/// \details No realiza la escritura inmediata; la acumula en \c pendingWrites
///  para aplicarla en \c build o \c overwrite.
/// \param binding Índice de binding declarado en el layout.
/// \param bufferInfo Descriptor del buffer (rango, offset, handle).
/// \return Referencia a \c *this para encadenado fluido.
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

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingInfo.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    pendingWrites.push_back(write);

    return (*this);
}

/// \brief Añade una escritura pendiente de tipo imagen al binding indicado.
/// \details No realiza la escritura inmediata; la acumula en \c pendingWrites
/// para aplicarla en \c build o \c overwrite.
/// \param binding Índice de binding declarado en el layout.
/// \param imageInfo Descriptor de imagen (sampler, view y layout).
/// \return Referencia a \c *this para encadenado fluido.
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

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingInfo.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    pendingWrites.push_back(write);

    return (*this);
}

/// \brief Construye (asigna) un \c VkDescriptorSet y aplica las escrituras acumuladas.
/// \details Llama internamente a \c DescriptorPool::allocate con el layout asociado;
///  si la reserva falla, devuelve \c false y no modifica \c set.
/// \param set Referencia de salida con el set recién asignado y escrito.
/// \return \c true si la asignación y actualización han sido correctas.
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

/// \brief Reescribe un \c VkDescriptorSet ya existente con las escrituras acumuladas.
/// \details Útil cuando no se desea reasignar un set sino actualizar sus bindings.
/// \param set Descriptor set objetivo a actualizar.
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
