/*
 * Project: VulkanAPI
 * File: VulkanBuffer.hpp
 * Author: Santiago Carb� Garc�a
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "VulkanDevice.hpp"

 /// \brief Encapsula un VkBuffer y su memoria asociada.
 /// \details Administra creaci�n, mapeo, escritura y sincronizaci�n de un b�fer
 /// Vulkan con soporte para instancias m�ltiples. Calcula alineaci�n por instancia,
 /// ofrece utilidades para UBOs din�micos y expone descriptores listos para usar.
class VulkanBuffer
{
public:
    /// \brief Crea un b�fer y reserva su memoria en el dispositivo.
    /// \details Configura tama�o total en funci�n de \c instanceSize , \c instanceCount
    /// y la alineaci�n m�nima requerida. No mapea la memoria de forma autom�tica.
    /// \param device Dispositivo l�gico Vulkan.
    /// \param instanceSize Tama�o de una instancia l�gica almacenada en el b�fer.
    /// \param instanceCount N�mero de instancias almacenadas consecutivamente.
    /// \param usageFlags Uso del b�fer (por ejemplo \c VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT).
    /// \param memoryPropertyFlags Propiedades de la memoria (por ejemplo \c HOST_VISIBLE).
    /// \param minOffsetAlignment Alineaci�n m�nima por instancia. Use 1 si no aplica.
    VulkanBuffer(
        VulkanDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment = 1);

    /// \brief Libera el b�fer y su memoria.
    ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    /// \brief Mapea la memoria del b�fer para acceso CPU.
    /// \details Si \c size es \c VK_WHOLE_SIZE se mapea el rango completo.
    /// \param size Tama�o a mapear en bytes.
    /// \param offset Desplazamiento inicial del mapeo.
    /// \return Resultado Vulkan de la operaci�n de mapeo.
    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /// \brief Desmapea la memoria previamente mapeada.
    void unmap();

    /// \brief Copia datos desde CPU al rango mapeado del b�fer.
    /// \details Requiere el b�fer mapeado. No realiza \c vkFlushMappedMemoryRanges.
    /// \param data Puntero al bloque de memoria origen.
    /// \param size Tama�o a escribir. Use \c VK_WHOLE_SIZE para el total.
    /// \param offset Desplazamiento dentro del b�fer de destino.
    void writeToBuffer(
        void* data,
        VkDeviceSize size = VK_WHOLE_SIZE,
        VkDeviceSize offset = 0);

    /// \brief Sincroniza escrituras de host hacia la GPU.
    /// \details Necesario para memorias no coherentes. Para coherentes no es obligatorio.
    /// \param size Tama�o del rango a sincronizar.
    /// \param offset Desplazamiento inicial del rango.
    /// \return Resultado Vulkan de la operaci�n de \c flush.
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /// \brief Devuelve la informaci�n de descriptor para enlazar el b�fer.
    /// \details �til para \c VkWriteDescriptorSet con UBOs o SSBOs.
    /// \param size Tama�o del rango expuesto al shader.
    /// \param offset Desplazamiento del rango expuesto.
    /// \return Estructura \c VkDescriptorBufferInfo lista para usar.
    VkDescriptorBufferInfo descriptorInfo(
        VkDeviceSize size = VK_WHOLE_SIZE,
        VkDeviceSize offset = 0);

    /// \brief Sincroniza lecturas de host desde la GPU.
    /// \details Necesario para memorias no coherentes antes de leer cambios desde CPU.
    /// \param size Tama�o del rango a invalidar.
    /// \param offset Desplazamiento inicial del rango.
    /// \return Resultado Vulkan de la operaci�n de \c invalidate.
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    /// \brief Escribe una instancia concreta en el b�fer alineado.
    /// \details Calcula el desplazamiento como \c index * alignmentSize.
    /// \param data Puntero a los datos de la instancia.
    /// \param index �ndice de instancia destino.
    void writeToIndex(void* data, int index);

    /// \brief Realiza \c flush del rango correspondiente a una instancia.
    /// \param index �ndice de instancia.
    /// \return Resultado Vulkan de la operaci�n.
    VkResult flushIndex(int index);

    /// \brief Devuelve \c VkDescriptorBufferInfo para una instancia concreta.
    /// \param index �ndice de instancia.
    /// \return Descriptor limitado al subrango de la instancia.
    VkDescriptorBufferInfo descriptorInfoForIndex(int index);

    /// \brief Invalida el rango correspondiente a una instancia.
    /// \param index �ndice de instancia.
    /// \return Resultado Vulkan de la operaci�n.
    VkResult invalidateIndex(int index);

    /// \brief Acceso al manejador \c VkBuffer .
    VkBuffer getBuffer() const
    {
        return buffer;
    }

    /// \brief Devuelve el puntero CPU al �rea mapeada.
    void* getMappedMemory() const
    {
        return mapped;
    }

    /// \brief N�mero de instancias almacenadas.
    uint32_t getInstanceCount() const
    {
        return instanceCount;
    }

    /// \brief Tama�o por instancia en bytes.
    VkDeviceSize getInstanceSize() const
    {
        return instanceSize;
    }

    /// \brief Tama�o alineado por instancia.
    VkDeviceSize getAlignmentSize() const
    {
        return alignmentSize;
    }

    /// \brief Flags de uso del b�fer.
    VkBufferUsageFlags getUsageFlags() const
    {
        return usageFlags;
    }

    /// \brief Propiedades de la memoria asignada.
    VkMemoryPropertyFlags getMemoryPropertyFlags() const
    {
        return memoryPropertyFlags;
    }

    /// \brief Tama�o total del b�fer en bytes.
    VkDeviceSize getBufferSize() const
    {
        return bufferSize;
    }

private:
    /// \brief Calcula el tama�o alineado por instancia.
    /// \details Si \c minOffsetAlignment es mayor que cero, redondea \c instanceSize
    /// al m�ltiplo m�s cercano requerido por el dispositivo.
    /// \param instanceSize Tama�o base de la instancia.
    /// \param minOffsetAlignment Alineaci�n m�nima exigida por el dispositivo.
    /// \return Tama�o alineado por instancia.
    static VkDeviceSize getAlignment(
        VkDeviceSize instanceSize,
        VkDeviceSize minOffsetAlignment);

    /// Dispositivo Vulkan para crear y administrar el b�fer.
    VulkanDevice& vulkanDevice;

    /// Puntero host a la regi�n mapeada. Nulo si no est� mapeado.
    void* mapped = nullptr;

    /// Manejador del b�fer Vulkan.
    VkBuffer buffer = VK_NULL_HANDLE;

    /// Memoria asociada al b�fer.
    VkDeviceMemory memory = VK_NULL_HANDLE;

    /// Tama�o total del b�fer en bytes.
    VkDeviceSize bufferSize;

    /// N�mero de instancias l�gicas en el b�fer.
    uint32_t instanceCount;

    /// Tama�o base de una instancia en bytes.
    VkDeviceSize instanceSize;

    /// Tama�o alineado por instancia en bytes.
    VkDeviceSize alignmentSize;

    /// Flags de uso del b�fer.
    VkBufferUsageFlags usageFlags;

    /// Propiedades de la memoria asignada.
    VkMemoryPropertyFlags memoryPropertyFlags;
};


