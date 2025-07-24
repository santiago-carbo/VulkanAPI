#pragma once

#include "GameObject.hpp"
#include "Window.hpp"

class KeyboardMovementController 
{
    public:
        struct KeyBindings 
        {
            int strafeLeft = GLFW_KEY_A;
            int strafeRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int ascend = GLFW_KEY_E;
            int descend = GLFW_KEY_Q;
            int turnLeft = GLFW_KEY_LEFT;
            int turnRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void update(GLFWwindow* window, float deltaTime, GameObject& object);

        KeyBindings keys {};
        float moveSpeed = 4.0f;
        float lookSpeed = 2.0f;
    };