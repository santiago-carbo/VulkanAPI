/*
 * Project: VulkanAPI
 * File: DescriptorSetLayout.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

#include <memory>
#include <unordered_map>

 /// \brief Wrapper de \c VkDescriptorSetLayout con acceso a bindings por índice.
 /// \details Recibe un mapa \c binding->VkDescriptorSetLayoutBinding y crea el
 /// \c VkDescriptorSetLayout correspondiente. Expone el handle y
 /// permite consultar la descripción de cada binding.
class DescriptorSetLayout
{
    public:
        /// \brief Crea el \c VkDescriptorSetLayout a partir de un mapa de bindings.
        /// \param device Dispositivo Vulkan sobre el que se crea el layout.
        /// \param entries Mapa {binding -> \c VkDescriptorSetLayoutBinding} que define el layout.
        /// \post \c layout queda válido hasta la destrucción del objeto.
        DescriptorSetLayout(
            VulkanDevice& device,
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> entries);

        /// \brief Destruye el \c VkDescriptorSetLayout asociado.
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        /// \brief Devuelve el handle de \c VkDescriptorSetLayout.
        /// \return \c VkDescriptorSetLayout válido creado en el constructor.
        VkDescriptorSetLayout get() const
        {
            return layout;
        }

        /// \brief Accede a la definición de un binding concreto.
        /// \param binding Índice de binding a consultar.
        /// \return Referencia constante al \c VkDescriptorSetLayoutBinding almacenado.
        /// \throws std::out_of_range si el binding no existe en \c entries.
        const VkDescriptorSetLayoutBinding& getBinding(uint32_t binding) const
        {
            return entries.at(binding);
        }

    private:
        /// Dispositivo sobre el que se creó el layout.
        VulkanDevice& device;

        /// Handle del \c VkDescriptorSetLayout.
        VkDescriptorSetLayout layout;

        /// Mapa local de bindings.
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> entries;
};
