#pragma once

#include <glm/glm.hpp>

class Camera 
{
    public:
        
        void setOrthographicProjection(
            float left, float right, float top, float bottom, float nearPlane, float farPlane);

        void setPerspectiveProjection(
            float verticalFov, float aspectRatio, float nearPlane, float farPlane);

        void lookAtDirection(
            glm::vec3 position, 
            glm::vec3 direction, 
            glm::vec3 up = glm::vec3{ 0.0f, 1.0f, 0.0f });

        void lookAtTarget(
            glm::vec3 position, 
            glm::vec3 target, 
            glm::vec3 up = glm::vec3{ 0.0f, 1.0f, 0.0f });

        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

        const glm::mat4& getProjectionMatrix() const 
        { 
            return (projection); 
        }

        const glm::mat4& getViewMatrix() const 
        { 
            return (view); 
        }

        const glm::mat4& getInverseViewMatrix() const 
        { 
            return (inverseView); 
        }

        const glm::vec3 getPosition() const 
        { 
            return glm::vec3(inverseView[3]); 
        }

    private:
        glm::mat4 projection {1.0f};
        glm::mat4 view {1.0f};
        glm::mat4 inverseView {1.0f};
};