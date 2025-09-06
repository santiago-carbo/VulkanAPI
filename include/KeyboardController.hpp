/*
 * Project: VulkanAPI
 * File: KeyboardController.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "GameObject.hpp"
#include "Window.hpp"

 /// \brief Controlador de movimiento por teclado para un \c GameObject tipo cámara/visor.
 /// \details Interpreta teclas de desplazamiento (WASD+Q/E) y de giro (flechas)
 /// para actualizar la \c Transform del \c GameObject objetivo en cada frame.
 /// El cálculo usa \c deltaTime para garantizar movimiento dependiente del tiempo.
class KeyboardMovementController
{
public:
    /// \brief Mapeo de teclas para traslación y rotación.
    /// \details Las constantes \c GLFW_KEY_* se usan para leer el estado del teclado.
    struct KeyBindings
    {
        /// Traslación lateral izquierda.
        int strafeLeft = GLFW_KEY_A;
        /// Traslación lateral derecha.
        int strafeRight = GLFW_KEY_D;
        /// Avance hacia delante.
        int moveForward = GLFW_KEY_W;
        /// Retroceso.
        int moveBackward = GLFW_KEY_S;
        /// Desplazamiento hacia arriba.
        int ascend = GLFW_KEY_E;
        /// Desplazamiento hacia abajo.
        int descend = GLFW_KEY_Q;
        /// Rotación yaw a la izquierda.
        int turnLeft = GLFW_KEY_LEFT;
        /// Rotación yaw a la derecha.
        int turnRight = GLFW_KEY_RIGHT;
        /// Rotación pitch hacia arriba.
        int lookUp = GLFW_KEY_UP;
        /// Rotación pitch hacia abajo.
        int lookDown = GLFW_KEY_DOWN;
    };

    /// \brief Aplica entrada de teclado al \c GameObject.
    /// \param window Puntero a la ventana GLFW para consultar el estado de teclas.
    /// \param deltaTime Tiempo transcurrido desde el frame anterior (en segundos).
    /// \param object Objeto a mover/rotar (se modifica su \c Transform).
    /// \post \c object.transform se actualiza en función de \c keys, \c moveSpeed y \c lookSpeed.
    void update(GLFWwindow* window, float deltaTime, GameObject& object);

    /// Configuración de teclas activas.
    KeyBindings keys {};
    /// Velocidad lineal (unidades/segundo).
    float moveSpeed = 4.0f;
    /// Velocidad angular (radianes/segundo).
    float lookSpeed = 2.0f;
};
