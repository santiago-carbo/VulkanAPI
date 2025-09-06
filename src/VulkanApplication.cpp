/*
 * Project: VulkanAPI
 * File: VulkanApplication.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "VulkanApplication.hpp"

#include "DescriptorWriter.hpp"
#include "KeyboardController.hpp"
#include "PointLightRenderer.hpp"
#include "BasicRenderer.hpp"

#include <chrono>
#include <thread>


struct Threads
{
    VkCommandPool pool{VK_NULL_HANDLE};
    std::vector<VkCommandBuffer> sec;
};


/// \brief Construye la aplicación y prepara los componentes básicos.
/// \details No inicia el bucle de ejecución. La inicialización
/// completa se realiza en \c run .
VulkanApplication::VulkanApplication() 
{
    vulkanDevice = std::make_unique<VulkanDevice>(editorUI.getWindow());
    renderer = std::make_unique<Renderer>(editorUI.getWindow(), *vulkanDevice);

    editorUI.init(vulkanDevice->getInstance(),
        vulkanDevice->getPhysicalDevice(),
        vulkanDevice->getDevice(),
        vulkanDevice->getQueueFamilyIndices().GetGraphicsFamily(),
        vulkanDevice->getGraphicsQueue(),
        renderer->getSwapChainRenderPass(),
        renderer->getSwapChainImageCount());

    std::vector<VkDescriptorPoolSize> poolSizes = 
    {
     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT}
    };

    globalPool = std::make_unique<DescriptorPool>(
        *vulkanDevice,
        SwapChain::MAX_FRAMES_IN_FLIGHT,
        0,
        poolSizes
    );

    loadGameObjects();
}

/// \brief Libera los recursos administrados por la aplicación.
VulkanApplication::~VulkanApplication() 
{
}

/// \brief Ejecuta la aplicación.
/// \details Lleva a cabo la inicialización restante, entra en el
/// bucle principal de render y procesa eventos hasta que el usuario
/// cierra la ventana. Al salir, sincroniza y limpia recursos.
void VulkanApplication::run() 
{
    std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < uboBuffers.size(); ++i) 
    {
        uboBuffers[i] = std::make_unique<VulkanBuffer>(
            *vulkanDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        uboBuffers[i]->map();
    }

    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> globalBindings = 
    {
        {
            0,
            VkDescriptorSetLayoutBinding 
            {
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                VK_SHADER_STAGE_ALL_GRAPHICS,
                nullptr
            }
        }
    };

    std::unique_ptr<DescriptorSetLayout> globalSetLayout =
        std::make_unique<DescriptorSetLayout>(*vulkanDevice, globalBindings);

    std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();

        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    BasicRenderer basicRenderer(
        *vulkanDevice,
        renderer->getSwapChainRenderPass(),
        globalSetLayout->get()
    );

    const int M = std::max(2u, std::thread::hardware_concurrency());
    std::vector<Threads> workers(M);

    for (int t = 0; t < M; ++t) 
    {
        VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pci.queueFamilyIndex = vulkanDevice->getQueueFamilyIndices().GetGraphicsFamily();
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        vkCreateCommandPool(vulkanDevice->getDevice(), &pci, nullptr, &workers[t].pool);

        workers[t].sec.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};

        ai.commandPool = workers[t].pool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        ai.commandBufferCount = (uint32_t)workers[t].sec.size();

        vkAllocateCommandBuffers(vulkanDevice->getDevice(), &ai, workers[t].sec.data());
    }

    PointLightSystem pointLightSystem(
        *vulkanDevice,
        renderer->getSwapChainRenderPass(),
        globalSetLayout->get()
    );

    Camera camera;
    GameObject viewerObject = GameObject::create();
    viewerObject.transform.translation.z = -2.5f;

    KeyboardMovementController cameraController;
    std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = 
        std::chrono::high_resolution_clock::now();

    editorUI.setPerf(&renderer->getPerf());

    while (!editorUI.getWindow().shouldClose())
    {
        glfwPollEvents();
        std::chrono::time_point<std::chrono::high_resolution_clock> newTime = 
            std::chrono::high_resolution_clock::now();

        float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.update(editorUI.getWindow().getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = renderer->getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (VkCommandBuffer commandBuffer = renderer->beginFrame()) 
        {
            int frameIndex = renderer->getFrameIndex();

            renderer->getPerf().beginCpuFrame();
            renderer->getPerf().recordGpu(commandBuffer, static_cast<uint32_t>(frameIndex));

            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects};

            GlobalUbo ubo;
            ubo.projection = camera.getProjectionMatrix();
            ubo.view = camera.getViewMatrix();
            ubo.inverseView = camera.getInverseViewMatrix();

            pointLightSystem.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            renderer->beginSwapChainRenderPass(commandBuffer);

            editorUI.beginFrame();
            editorUI.drawGameObjects(gameObjects);

            /// Una hebra
            //basicRenderer.render(frameInfo);

            // Multi hebra
            VkCommandBufferInheritanceInfo inherit 
                {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};

            inherit.renderPass = renderer->getSwapChainRenderPass();
            inherit.subpass = 0;
            inherit.framebuffer = renderer->getCurrentFrameBuffer();

            std::vector<std::pair<unsigned, GameObject*>> view; view.reserve(gameObjects.size());

            for (auto& go : gameObjects)
            {
                view.push_back({go.first, &go.second });
            }

            const size_t N = view.size();
            const size_t chunk = (N + M - 1)/M;

            std::vector<std::thread> threads;
            threads.reserve(M);

            for (int t = 0; t < M; ++t) 
            {
                const size_t begin = std::min(N, size_t(t) * chunk);
                const size_t end = std::min(N, begin + chunk);

                if (begin >= end)
                {
                    break;
                }

                VkCommandBuffer cbSec = workers[t].sec[frameIndex];

                VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                bi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | 
                    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

                bi.pInheritanceInfo = &inherit;
                vkBeginCommandBuffer(cbSec, &bi);

                threads.emplace_back([&, cbSec, begin, end] 
                {
                    basicRenderer.recordRange(frameInfo, cbSec, begin, end);
                    vkEndCommandBuffer(cbSec);
                });
            }

            for (std::thread& th : threads)
            {
                th.join();
            }

            std::vector<VkCommandBuffer> execList;

            for (int t = 0; t < (int)threads.size(); ++t) 
            {
                execList.push_back(workers[t].sec[frameIndex]);
            }

            if (!execList.empty())
            {
                vkCmdExecuteCommands(commandBuffer, (uint32_t)execList.size(), execList.data());
            }

            pointLightSystem.render(frameInfo);

            editorUI.endFrame(commandBuffer);

            renderer->endSwapChainRenderPass(commandBuffer);
            renderer->getPerf().recordGpu(commandBuffer, static_cast<uint32_t>(frameIndex));
            renderer->endFrame();

            renderer->getPerf().endCpuFrame();
            renderer->getPerf().resolveGpu(static_cast<uint32_t>(frameIndex));
            renderer->getPerf().tickMonitors();
        }
    }

    vkDeviceWaitIdle(vulkanDevice->getDevice());

    editorUI.cleanup(vulkanDevice->getDevice());
}

/// \brief Carga y registra los \c GameObject de la escena.
/// \details Construye la geometría y las luces necesarias para
/// validar el motor, añadiéndolas al contenedor \c gameObjects .
void VulkanApplication::loadGameObjects() 
{
    std::shared_ptr<Model> model = Model::fromFile(*vulkanDevice, "models/room.obj");

    GameObject room = GameObject::create();
    room.model = model;
    room.transform.scale = {0.5f, 0.5f, 0.5f};
    room.transform.rotation =  {glm::pi<float>(), 0.0f, 0.0f};
    room.transform.translation = {0.0f, 0.5f, 0.0f};
    gameObjects.emplace(room.getId(), std::move(room));

    std::vector<glm::vec3> lightColors = 
    {
        {1.0f, 0.1f, 0.1f},
        {0.1f, 0.1f, 1.0f},
        {0.1f, 1.0f, 0.1f},
        {1.0f, 1.0f, 0.1f},
        {0.1f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f} 
    };

    int lightsPerSide = 4;
    float spacing = 1.0f;
    float yLevel = 0;
    float start = -spacing * (lightsPerSide - 1) / 2.0f;

    for (int i = 0; i < lightColors.size(); ++i) 
    {
        float radius = 0.3f + 0.2f * (i % 3);
        GameObject pointLight = GameObject::makePointLight(radius);

        pointLight.color = lightColors[i];

        int side = i / lightsPerSide;
        int index = i % lightsPerSide;

        glm::vec3 position;

        switch (side) 
        {
            case 0:
                position = 
                    glm::vec3(start + index * spacing, yLevel, -spacing * lightsPerSide / 2.0f);

                break;

            case 1:
                position = 
                    glm::vec3(spacing * lightsPerSide / 2.0f, yLevel, start + index * spacing);

                break;

            case 2:
                position = 
                    glm::vec3(start + index * spacing, yLevel, spacing * lightsPerSide / 2.0f);

                break;

            case 3:
                position = 
                    glm::vec3(-spacing * lightsPerSide / 2.0f, yLevel, start + index * spacing);

                break;

            default:
                continue;
        }

        pointLight.transform.translation = position;

        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }
}

