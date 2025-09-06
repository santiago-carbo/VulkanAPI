/*
 * Project: VulkanAPI
 * File: Renderer.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "Renderer.hpp"

#include <array>
#include <stdexcept>
#include <cassert>

/// \brief Construye el renderer asociado a una ventana y a un dispositivo Vulkan.
/// \param window Ventana donde se presenta la imagen.
/// \param device Dispositivo lógico Vulkan.
Renderer::Renderer(Window& window, VulkanDevice& device)
    : window{window}, vulkanDevice{device} 
{
    recreateSwapChain();
    createCommandBuffers();

    VkPhysicalDeviceProperties props;
    std::memset(&props, 0, sizeof(props));
    vkGetPhysicalDeviceProperties(vulkanDevice.getPhysicalDevice(), &props);
    perf.init(vulkanDevice.getDevice(), SwapChain::MAX_FRAMES_IN_FLIGHT, 
        props.limits.timestampPeriod);
}

/// \brief Libera recursos asociados y destruye la swapchain.
Renderer::~Renderer() 
{ 
    freeCommandBuffers(); 
}

/// \brief Recrea la swapchain cuando cambia el tamaño de la ventana o queda obsoleta.
void Renderer::recreateSwapChain() 
{
    VkExtent2D extent = window.getExtent();

    while (extent.width == 0 || extent.height == 0) 
    {
        extent = window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vulkanDevice.getDevice());

    if (swapChain == nullptr) 
    {
        swapChain = std::make_unique<SwapChain>(vulkanDevice, extent);
    }
    else 
    {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
        swapChain = std::make_unique<SwapChain>(vulkanDevice, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormats(*swapChain.get())) 
        {
            throw std::runtime_error(
                "💥[Vulkan API] Swap chain image or depth format has changed.");
        }
    }
}

/// \brief Crea los command buffers para los frames en vuelo.
void Renderer::createCommandBuffers() 
{
    commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(
        vulkanDevice.getDevice(),
        &allocInfo, 
        commandBuffers.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to allocate command buffers.");
    }
}

/// \brief Libera los command buffers creados.
void Renderer::freeCommandBuffers() {

    vkFreeCommandBuffers(
        vulkanDevice.getDevice(),
        vulkanDevice.getCommandPool(),
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    commandBuffers.clear();
}

/// \brief Comienza un frame. Adquiere imagen y prepara el command buffer.
/// \return Command buffer listo para grabación o nullptr si se recreó la swapchain.
VkCommandBuffer Renderer::beginFrame() 
{
    assert(!isFrameStarted && "💥[Vulkan API] Can't call beginFrame while already in progress.");

    VkResult result = swapChain->acquireNextImage(&currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to acquire swap chain image.");
    }

    isFrameStarted = true;

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to begin recording command buffer.");
    }

    perf.beginCpuFrame();
    perf.recordGpu(commandBuffer, currentFrameIndex);

    return commandBuffer;
}

/// \brief Finaliza el frame. Cierra el command buffer y presenta.
void Renderer::endFrame() 
{
    assert(isFrameStarted && "💥[Vulkan API] Can't call endFrame while frame is not in progress.");

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

    perf.recordGpu(commandBuffer, currentFrameIndex);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to record command buffer.");
    }

    VkResult result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        window.wasWindowResized()) {
        window.resetWindowResizedFlag();
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to present swap chain image.");
    }

    isFrameStarted = false;

    perf.endCpuFrame();
    perf.resolveGpu(currentFrameIndex);
    perf.tickMonitors();

    currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

/// \brief Inicia el render pass principal sobre el command buffer indicado.
/// \param commandBuffer Command buffer devuelto por beginFrame.
void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) 
{
    assert(isFrameStarted && 
        "💥[Vulkan API] Can't call beginSwapChainRenderPass if frame is not in progress.");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "💥[Vulkan API] Can't begin render pass on command buffer from a different frame.");

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

/// \brief Finaliza el render pass principal sobre el command buffer indicado.
/// \param commandBuffer Command buffer devuelto por beginFrame.
void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) 
{
    assert(isFrameStarted && 
        "💥[Vulkan API] Can't call endSwapChainRenderPass if frame is not in progress.");
    assert(
        commandBuffer == getCurrentCommandBuffer() &&
        "💥[Vulkan API] Can't end render pass on command buffer from a different frame.");

    vkCmdEndRenderPass(commandBuffer);
}
