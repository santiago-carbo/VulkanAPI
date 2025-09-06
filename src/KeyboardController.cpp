/*
 * Project: VulkanAPI
 * File: KeyboardController.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "KeyboardController.hpp"

 /// \brief Aplica entrada de teclado al \c GameObject.
 /// \param window Puntero a la ventana GLFW para consultar el estado de teclas.
 /// \param deltaTime Tiempo transcurrido desde el frame anterior (en segundos).
 /// \param object Objeto a mover/rotar (se modifica su \c Transform).
 /// \post \c object.transform se actualiza en función de \c keys, \c moveSpeed y \c lookSpeed.
void KeyboardMovementController::update(GLFWwindow* window, float deltaTime, GameObject& object)
{
    // Bloque de rotación: acumula intención de giro en yaw (izquierda/derecha)
    // y pitch (arriba/abajo)
    glm::vec3 rotate {};

    if (glfwGetKey(window, keys.turnLeft) == GLFW_PRESS)
    {
        rotate.y -= 1.0f;
    }
    if (glfwGetKey(window, keys.turnRight) == GLFW_PRESS)
    {
        rotate.y += 1.0f;
    }
    if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
    {
        rotate.x += 1.0f;
    }
    if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
    {
        rotate.x -= 1.0f;
    }

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
    {
        object.transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
    }

    object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
    object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

    float yaw = object.transform.rotation.y;
    glm::vec3 forward {sin(yaw), 0.0f, cos(yaw)};
    glm::vec3 right {forward.z, 0.0f, -forward.x};
    glm::vec3 up {0.0f, -1.0f, 0.0f};

    glm::vec3 moveDir {};

    if (glfwGetKey(window, keys.strafeLeft) == GLFW_PRESS)
    {
        moveDir -= right;
    }
    if (glfwGetKey(window, keys.strafeRight) == GLFW_PRESS)
    {
        moveDir += right;
    }
    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
    {
        moveDir += forward;
    }
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
    {
        moveDir -= forward;
    }
    if (glfwGetKey(window, keys.ascend) == GLFW_PRESS)
    {
        moveDir += up;
    }
    if (glfwGetKey(window, keys.descend) == GLFW_PRESS)
    {
        moveDir -= up;
    }

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
    {
        object.transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
    }
}

