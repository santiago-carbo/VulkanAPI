/*
 * Project: VulkanAPI
 * File: EditorUI.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "EditorUI.hpp"
#include "Perf.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

 /// \brief Constructor por defecto.
EditorUI::EditorUI()
{
}

/// \brief Destructor.
EditorUI::~EditorUI()
{
}

/// \brief Inicializa el backend de ImGui para Vulkan y GLFW.
/// \details Crea contexto ImGui, configura estilo, inicializa \c ImGui_ImplGlfw
/// y \c ImGui_ImplVulkan y crea un descriptor pool propio para UI.
/// \param instance Instancia de Vulkan.
/// \param physicalDevice Dispositivo físico empleado por la aplicación.
/// \param device Dispositivo lógico (VkDevice).
/// \param graphicsQueueFamily Índice de la cola gráfica.
/// \param graphicsQueue Cola gráfica donde se enviarán comandos de UI.
/// \param renderPass Render pass destino (de la swapchain) para dibujar la UI.
/// \param imageCount Número de imágenes en la swapchain (para configurar ImGui).
void EditorUI::init(VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t graphicsQueueFamily,
    VkQueue graphicsQueue,
    VkRenderPass renderPass,
    uint32_t imageCount)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(this->window.getGLFWwindow(), true);

    createDescriptorPool(device);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.QueueFamily = graphicsQueueFamily;
    init_info.Queue = graphicsQueue;
    init_info.DescriptorPool = descriptorPool;
    init_info.MinImageCount = imageCount;
    init_info.ImageCount = imageCount;

    ImGui_ImplVulkan_Init(&init_info);
}

/// \brief Crea el descriptor pool específico que usa ImGui internamente.
/// \param device Dispositivo lógico Vulkan.
void EditorUI::createDescriptorPool(VkDevice device)
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("No se pudo crear el Descriptor Pool para ImGui");
    }
}

/// \brief Comienza un frame de ImGui.
/// \details Llama a \c ImGui_ImplVulkan_NewFrame, \c ImGui_ImplGlfw_NewFrame
/// y \c ImGui::NewFrame(). Debe invocarse una vez por frame antes de
/// construir los paneles.
void EditorUI::beginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

/// \brief Dibuja un panel de inspección/edición para \c gameObjects.
/// \details Recorre el mapa y muestra sliders para posición/rotación/escala.
/// \param gameObjects Contenedor de objetos de escena a inspeccionar/editar.
void EditorUI::drawGameObjects(std::unordered_map<unsigned int, GameObject>& gameObjects)
{
    ImGui::Begin("Objetos de Escena");

    for (std::pair<const unsigned int, GameObject>& entry : gameObjects) {
        unsigned int id = entry.first;
        GameObject& obj = entry.second;

        ImGui::PushID(static_cast<int>(id));

        ImGui::Text("GameObject %u", id);
        auto& transform = obj.transform;

        ImGui::SliderFloat3("Posicion", glm::value_ptr(transform.translation), -10.0f, 10.0f);
        ImGui::SliderFloat3("Rotacion", glm::value_ptr(transform.rotation), 0.0f, 360.0f);
        ImGui::SliderFloat3("Escala", glm::value_ptr(transform.scale), 0.1f, 5.0f);

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::End();

    if (perf)
    {
        perf->drawImGui();
    }
}

/// \brief Finaliza y emite los draw calls de ImGui al \c commandBuffer.
/// \details Llama a \c ImGui::Render() y \c ImGui_ImplVulkan_RenderDrawData()
/// para grabar los comandos de UI en el command buffer del frame.
/// \param commandBuffer Command buffer actual (en un render pass activo).
void EditorUI::endFrame(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

/// \brief Libera los recursos del backend de ImGui y su descriptor pool.
/// \param device VkDevice con el que se creó el descriptor pool.
void EditorUI::cleanup(VkDevice device)
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
}

