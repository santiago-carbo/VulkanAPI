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

 /// \brief Encapsula la gestión de un \c VkDescriptorPool.
 /// \details Proporciona operaciones de reserva (\c allocate), liberación (\c free)
 /// y reinicio (\c reset) de descriptores. No toma propiedad del
 /// \c VulkanDevice, solo lo referencia para crear/gestionar el pool.
class DescriptorPool
{
    public:
        /// \brief Construye el pool de descriptores con una configuración dada.
        /// \param device Dispositivo Vulkan sobre el que se crea el \c VkDescriptorPool.
        /// \param maxSets Número máximo de descriptor sets que podrá emitir el pool.
        /// \param flags \c VkDescriptorPoolCreateFlags (p.ej., 
        /// \c VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT).
        /// \param sizes Lista de tamaños por tipo de descriptor (\c VkDescriptorPoolSize) 
        /// que dimensiona el pool.
        /// \post Se crea internamente un \c VkDescriptorPool listo para asignar sets.
        DescriptorPool(
            VulkanDevice& device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags flags,
            const std::vector<VkDescriptorPoolSize>& sizes);

        /// \brief Destruye el \c VkDescriptorPool asociado.
        /// \details Libera el recurso del dispositivo. No destruye \c device.
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        /// \brief Reserva un \c VkDescriptorSet a partir de un \c VkDescriptorSetLayout.
        /// \details Intenta asignar un set desde el pool actual. Si no hay espacio
        /// suficiente, devuelve \c false sin modificar \c descriptor.
        /// \param layout Layout del descriptor set a instanciar.
        /// \param descriptor Referencia de salida donde se almacena el set asignado.
        /// \return \c true si la asignación ha tenido éxito; \c false en caso contrario.
        bool allocate(const VkDescriptorSetLayout layout, VkDescriptorSet& descriptor) const;

        /// \brief Libera en bloque un conjunto de \c VkDescriptorSet al pool.
        /// \details Requiere que el pool se haya creado con el flag adecuado
        /// (\c VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT).
        /// \param descriptors Vector de sets previamente emitidos por este pool.
        void free(std::vector<VkDescriptorSet>& descriptors) const;

        /// \brief Restablece el \c VkDescriptorPool a su estado inicial.
        /// \details Invalida todos los sets emitidos por el pool.
        void reset();

        /// \brief Acceso de solo lectura al dispositivo propietario.
        /// \return Referencia constante al \c VulkanDevice asociado.
        const VulkanDevice& getDevice() const
        {
            return device;
        }

    private:
        /// Dispositivo Vulkan en el que reside el pool.
        VulkanDevice& device;

        /// Handle del \c VkDescriptorPool administrado.
        VkDescriptorPool pool;
};
