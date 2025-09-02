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

EditorUI::EditorUI() 
{
}

EditorUI::~EditorUI() 
{
}

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

void EditorUI::beginFrame() 
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

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

void EditorUI::endFrame(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

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
