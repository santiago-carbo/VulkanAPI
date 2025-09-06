/*
 * Project: VulkanAPI
 * File: DescriptorWriter.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "DescriptorSetLayout.hpp"
#include "DescriptorPool.hpp"

#include <vector>

 /// \brief Utilidad para construir y actualizar \c VkDescriptorSet.
 /// \details Permite encadenar escrituras de buffers e imágenes (UBO, samplers,
 /// storage, etc.) contra un \c DescriptorSetLayout concreto. Con \c build
 /// se solicita un set nuevo al \c DescriptorPool y se aplican las
 /// escrituras; con \c overwrite se reescribe un set ya existente.
class DescriptorWriter
{
	public:
		/// \brief Crea un escritor asociado a un layout y a un pool.
		/// \param layout Layout de descriptores que define los bindings válidos.
		/// \param pool Pool desde el que se asignarán los \c VkDescriptorSet.
		DescriptorWriter(DescriptorSetLayout& layout, DescriptorPool& pool);

		/// \brief Añade una escritura pendiente de tipo buffer al binding indicado.
		/// \details No realiza la escritura inmediata; la acumula en \c pendingWrites
		///  para aplicarla en \c build o \c overwrite.
		/// \param binding Índice de binding declarado en el layout.
		/// \param bufferInfo Descriptor del buffer (rango, offset, handle).
		/// \return Referencia a \c *this para encadenado fluido.
		DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);

		/// \brief Añade una escritura pendiente de tipo imagen al binding indicado.
		/// \details No realiza la escritura inmediata; la acumula en \c pendingWrites
		/// para aplicarla en \c build o \c overwrite.
		/// \param binding Índice de binding declarado en el layout.
		/// \param imageInfo Descriptor de imagen (sampler, view y layout).
		/// \return Referencia a \c *this para encadenado fluido.
		DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		/// \brief Construye (asigna) un \c VkDescriptorSet y aplica las escrituras acumuladas.
		/// \details Llama internamente a \c DescriptorPool::allocate con el layout asociado;
		///  si la reserva falla, devuelve \c false y no modifica \c set.
		/// \param set Referencia de salida con el set recién asignado y escrito.
		/// \return \c true si la asignación y actualización han sido correctas.
		bool build(VkDescriptorSet& set);

		/// \brief Reescribe un \c VkDescriptorSet ya existente con las escrituras acumuladas.
		/// \details Útil cuando no se desea reasignar un set sino actualizar sus bindings.
		/// \param set Descriptor set objetivo a actualizar.
		void overwrite(VkDescriptorSet& set);

	private:
		/// Layout que valida los \c bindings utilizados.
		DescriptorSetLayout& layout;

		/// Pool desde el que se asignan los descriptor sets.
		DescriptorPool& pool;

		/// Escrituras acumuladas a aplicar en bloque.
		std::vector<VkWriteDescriptorSet> pendingWrites;
};
