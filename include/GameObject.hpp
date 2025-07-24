#pragma once

#include "Model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>


struct Transform
{
    glm::vec3 translation {};
    glm::vec3 scale {1.0f, 1.0f, 1.0f};
    glm::vec3 rotation {};

    glm::mat4 matrix() const;
    glm::mat3 normalMatrix() const;
};

struct PointLight
{
    float intensity = 1.0f;
};

class GameObject
{
    public:
        static GameObject create();

        static GameObject makePointLight(
            float intensity = 10.f,
            float radius = 0.1f,
            glm::vec3 color = glm::vec3(1.f));

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        unsigned int getId() const 
        { 
            return (id); 
        }

        glm::vec3 color {};
        Transform transform {};
        std::shared_ptr<Model> model{};
        std::unique_ptr<PointLight> light = nullptr;

    private:
        explicit GameObject(unsigned int objId) : id{ objId } {}

        unsigned int id;
    };
