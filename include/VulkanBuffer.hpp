/*
 * Project: VulkanAPI
 * File: VulkanBuffer.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

 /// \brief Encapsula un VkBuffer y su memoria asociada.
 /// \details Administra creación, mapeo, escritura y sincronización de un búfer
 /// Vulkan con soporte para instancias múltiples. Calcula alineación por instancia,
 /// ofrece utilidades para UBOs dinámicos y expone descriptores listos para usar.
class VulkanBuffer
{
public:
    /// \brief Crea un búfer y reserva su memoria en el dispositivo.
    /// \details Configura tamaño total en función de \c instanceSize , \c instanceCount
    /// y la alineación mínima requerida. No mapea la memoria de forma automática.
    /// \param device Dispositivo lógico Vulkan.
    /// \param instanceSize Tamaño de una instancia lógica almacenada en el búfer.
    /// \param instanceCount Número de instancias almacenadas consecutivamente.
    /// \param usageFlags Uso del búfer (por ejemplo \c VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT).
    /// \param memoryPropertyFlags Propiedades de la memoria (por ejemplo \c HOST_VISIBLE).
    /// \param minOffsetAlignment Alineación mínima por instancia. Use 1 si no aplica.
    VulkanBuffer(
        VulkanDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment = 1);

    /// \brief Libera el búfer y su memoria.
    ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    /// \brief Mapea la memoria del búfer para acceso CPU.
    /// \details Si \c size es \c VK_WHOLE_SIZE se mapea el rango completo.
    /// \param size Tamaño a mapear en bytes.
    /// \param offset Desplazamiento inicial del mapeo.
    /// \return Resultado Vulkan de la operación de mapeo.
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /// \brief Desmapea la memoria previamente mapeada.
    void unmap();

    /// \brief Copia datos desde CPU al rango mapeado del búfer.
    /// \details Requiere el búfer mapeado. No realiza \c vkFlushMappedMemoryRanges.
    /// \param data Puntero al bloque de memoria origen.
    /// \param size Tamaño a escribir. Use \c VK_WHOLE_SIZE para el total.
    /// \param offset Desplazamiento dentro del búfer de destino.
    void writeToBuffer(
        void* data,
        VkDeviceSize size = VK_WHOLE_SIZE,
        VkDeviceSize offset = 0);

    /// \brief Sincroniza escrituras de host hacia la GPU.
    /// \details Necesario para memorias no coherentes. Para coherentes no es obligatorio.
    /// \param size Tamaño del rango a sincronizar.
    /// \param offset Desplazamiento inicial del rango.
    /// \return Resultado Vulkan de la operación de \c flush.
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /// \brief Devuelve la información de descriptor para enlazar el búfer.
    /// \details Útil para \c VkWriteDescriptorSet con UBOs o SSBOs.
    /// \param size Tamaño del rango expuesto al shader.
    /// \param offset Desplazamiento del rango expuesto.
    /// \return Estructura \c VkDescriptorBufferInfo lista para usar.
    VkDescriptorBufferInfo descriptorInfo(
        VkDeviceSize size = VK_WHOLE_SIZE,
        VkDeviceSize offset = 0);

    /// \brief Sincroniza lecturas de host desde la GPU.
    /// \details Necesario para memorias no coherentes antes de leer cambios desde CPU.
    /// \param size Tamaño del rango a invalidar.
    /// \param offset Desplazamiento inicial del rango.
    /// \return Resultado Vulkan de la operación de \c invalidate.
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /// \brief Escribe una instancia concreta en el búfer alineado.
    /// \details Calcula el desplazamiento como \c index * alignmentSize.
    /// \param data Puntero a los datos de la instancia.
    /// \param index Índice de instancia destino.
    void writeToIndex(void* data, int index);

    /// \brief Realiza \c flush del rango correspondiente a una instancia.
    /// \param index Índice de instancia.
    /// \return Resultado Vulkan de la operación.
    VkResult flushIndex(int index);

    /// \brief Devuelve \c VkDescriptorBufferInfo para una instancia concreta.
    /// \param index Índice de instancia.
    /// \return Descriptor limitado al subrango de la instancia.
    VkDescriptorBufferInfo descriptorInfoForIndex(int index);

    /// \brief Invalida el rango correspondiente a una instancia.
    /// \param index Índice de instancia.
    /// \return Resultado Vulkan de la operación.
    VkResult invalidateIndex(int index);

    /// \brief Acceso al manejador \c VkBuffer .
    VkBuffer getBuffer() const
    {
        return buffer;
    }

    /// \brief Devuelve el puntero CPU al área mapeada.
    void* getMappedMemory() const
    {
        return mapped;
    }

    /// \brief Número de instancias almacenadas.
    uint32_t getInstanceCount() const
    {
        return instanceCount;
    }

    /// \brief Tamaño por instancia en bytes.
    VkDeviceSize getInstanceSize() const
    {
        return instanceSize;
    }

    /// \brief Tamaño alineado por instancia.
    VkDeviceSize getAlignmentSize() const
    {
        return alignmentSize;
    }

    /// \brief Flags de uso del búfer.
    VkBufferUsageFlags getUsageFlags() const
    {
        return usageFlags;
    }

    /// \brief Propiedades de la memoria asignada.
    VkMemoryPropertyFlags getMemoryPropertyFlags() const
    {
        return memoryPropertyFlags;
    }

    /// \brief Tamaño total del búfer en bytes.
    VkDeviceSize getBufferSize() const
    {
        return bufferSize;
    }

private:
    /// \brief Calcula el tamaño alineado por instancia.
    /// \details Si \c minOffsetAlignment es mayor que cero, redondea \c instanceSize
    /// al múltiplo más cercano requerido por el dispositivo.
    /// \param instanceSize Tamaño base de la instancia.
    /// \param minOffsetAlignment Alineación mínima exigida por el dispositivo.
    /// \return Tamaño alineado por instancia.
    static VkDeviceSize getAlignment(
        VkDeviceSize instanceSize,
        VkDeviceSize minOffsetAlignment);

    /// Dispositivo Vulkan para crear y administrar el búfer.
    VulkanDevice& vulkanDevice;

    /// Puntero host a la región mapeada. Nulo si no está mapeado.
    void* mapped = nullptr;

    /// Manejador del búfer Vulkan.
    VkBuffer buffer = VK_NULL_HANDLE;

    /// Memoria asociada al búfer.
    VkDeviceMemory memory = VK_NULL_HANDLE;

    /// Tamaño total del búfer en bytes.
    VkDeviceSize bufferSize;

    /// Número de instancias lógicas en el búfer.
    uint32_t instanceCount;

    /// Tamaño base de una instancia en bytes.
    VkDeviceSize instanceSize;

    /// Tamaño alineado por instancia en bytes.
    VkDeviceSize alignmentSize;

    /// Flags de uso del búfer.
    VkBufferUsageFlags usageFlags;

    /// Propiedades de la memoria asignada.
    VkMemoryPropertyFlags memoryPropertyFlags;
};


