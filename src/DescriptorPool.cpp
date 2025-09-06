/*
 * Project: VulkanAPI
 * File: DescriptorPool.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "DescriptorPool.hpp"

#include <stdexcept>

 /// \brief Construye el pool de descriptores con una configuración dada.
 /// \param device Dispositivo Vulkan sobre el que se crea el \c VkDescriptorPool.
 /// \param maxSets Número máximo de descriptor sets que podrá emitir el pool.
 /// \param flags \c VkDescriptorPoolCreateFlags (p.ej., 
 /// \c VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT).
 /// \param sizes Lista de tamaños por tipo de descriptor (\c VkDescriptorPoolSize) 
 /// que dimensiona el pool.
 /// \post Se crea internamente un \c VkDescriptorPool listo para asignar sets.
DescriptorPool::DescriptorPool(
    VulkanDevice& device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags flags,
    const std::vector<VkDescriptorPoolSize>& sizes)
    : device{device}
{
    VkDescriptorPoolCreateInfo info{};
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

/// \brief Destruye el \c VkDescriptorPool asociado.
/// \details Libera el recurso del dispositivo. No destruye \c device.
DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(device.getDevice(), pool, nullptr);
}

/// \brief Reserva un \c VkDescriptorSet a partir de un \c VkDescriptorSetLayout.
/// \details Intenta asignar un set desde el pool actual. Si no hay espacio
/// suficiente, devuelve \c false sin modificar \c descriptor.
/// \param layout Layout del descriptor set a instanciar.
/// \param descriptor Referencia de salida donde se almacena el set asignado.
/// \return \c true si la asignación ha tenido éxito; \c false en caso contrario.
bool DescriptorPool::allocate(
    const VkDescriptorSetLayout layout,
    VkDescriptorSet& descriptor) const
{
    VkDescriptorSetAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc.descriptorPool = pool;
    alloc.pSetLayouts = &layout;
    alloc.descriptorSetCount = 1;

    return (vkAllocateDescriptorSets(device.getDevice(), &alloc, &descriptor) == VK_SUCCESS);
}

/// \brief Libera en bloque un conjunto de \c VkDescriptorSet al pool.
/// \details Requiere que el pool se haya creado con el flag adecuado
/// (\c VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT).
/// \param descriptors Vector de sets previamente emitidos por este pool.
void DescriptorPool::free(std::vector<VkDescriptorSet>& descriptors) const
{
    vkFreeDescriptorSets(
        device.getDevice(),
        pool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
}

/// \brief Restablece el \c VkDescriptorPool a su estado inicial.
/// \details Invalida todos los sets emitidos por el pool.
void DescriptorPool::reset()
{
    vkResetDescriptorPool(device.getDevice(), pool, 0);
}
