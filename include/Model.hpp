#pragma once

#include "VulkanBuffer.hpp"
#include "VulkanDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>

class Model 
{
    public:
        struct Vertex 
        {
            glm::vec3 position {};
            glm::vec3 color {};
            glm::vec3 normal {};
            glm::vec2 uv {};

            static std::vector<VkVertexInputBindingDescription> bindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> attributeDescriptions();

            bool operator==(const Vertex& other) const 
            {
                return ((position == other.position) && (color == other.color) &&
                    (normal == other.normal) && (uv == other.uv));
            }
        };

        struct Builder 
        {
            std::vector<Vertex> vertices {};
            std::vector<uint32_t> indices {};

            void loadFromFile(const std::string& filepath);
        };

        Model(VulkanDevice& device, const Builder& builder);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        static std::unique_ptr<Model> fromFile(VulkanDevice& device, const std::string& filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffer(const std::vector<Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);

        VulkanDevice& device;
        std::unique_ptr<VulkanBuffer> vertexBuffer;
        uint32_t vertexCount = 0;
        bool useIndexBuffer = false;
        std::unique_ptr<VulkanBuffer> indexBuffer;
        uint32_t indexCount = 0;
};
