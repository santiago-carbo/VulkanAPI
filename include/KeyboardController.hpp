/*
 * Project: VulkanAPI
 * File: KeyboardController.hpp
 * Author: Santiago Carb� Garc�a
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "GameObject.hpp"
#include "Window.hpp"

 /// \brief Controlador de movimiento por teclado para un \c GameObject tipo c�mara/visor.
 /// \details Interpreta teclas de desplazamiento (WASD+Q/E) y de giro (flechas)
 /// para actualizar la \c Transform del \c GameObject objetivo en cada frame.
 /// El c�lculo usa \c deltaTime para garantizar movimiento dependiente del tiempo.
class KeyboardMovementController
{
public:
    /// \brief Mapeo de teclas para traslaci�n y rotaci�n.
    /// \details Las constantes \c GLFW_KEY_* se usan para leer el estado del teclado.
    struct KeyBindings
    {
        /// Traslaci�n lateral izquierda.
        int strafeLeft = GLFW_KEY_A;
        /// Traslaci�n lateral derecha.
        int strafeRight = GLFW_KEY_D;
        /// Avance hacia delante.
        int moveForward = GLFW_KEY_W;
        /// Retroceso.
        int moveBackward = GLFW_KEY_S;
        /// Desplazamiento hacia arriba.
        int ascend = GLFW_KEY_E;
        /// Desplazamiento hacia abajo.
        int descend = GLFW_KEY_Q;
        /// Rotaci�n yaw a la izquierda.
        int turnLeft = GLFW_KEY_LEFT;
        /// Rotaci�n yaw a la derecha.
        int turnRight = GLFW_KEY_RIGHT;
        /// Rotaci�n pitch hacia arriba.
        int lookUp = GLFW_KEY_UP;
        /// Rotaci�n pitch hacia abajo.
        int lookDown = GLFW_KEY_DOWN;
    };

    /// \brief Aplica entrada de teclado al \c GameObject.
    /// \param window Puntero a la ventana GLFW para consultar el estado de teclas.
    /// \param deltaTime Tiempo transcurrido desde el frame anterior (en segundos).
    /// \param object Objeto a mover/rotar (se modifica su \c Transform).
    /// \post \c object.transform se actualiza en funci�n de \c keys, \c moveSpeed y \c lookSpeed.
    void update(GLFWwindow* window, float deltaTime, GameObject& object);

    /// Configuraci�n de teclas activas.
    KeyBindings keys {};
    /// Velocidad lineal (unidades/segundo).
    float moveSpeed = 4.0f;
    /// Velocidad angular (radianes/segundo).
    float lookSpeed = 2.0f;
};
