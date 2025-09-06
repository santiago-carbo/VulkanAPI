/*
 * Project: VulkanAPI
 * File: Camera.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <glm/glm.hpp>

 /// \brief Cámara de escena: gestiona matrices de proyección y vista.
 /// \details Mantiene las matrices de proyección (\c projection), vista (\c view)
 /// e inversa de vista (\c inverseView). Proporciona utilidades para
 /// configurar proyección ortográfica/perspectiva y distintas formas
 /// de orientar la cámara (dirección, objetivo o ángulos YXZ).
class Camera
{
    public:
        /// \brief Configura proyección ortográfica.
        /// \param left Límite izquierdo del frustum ortográfico.
        /// \param right Límite derecho del frustum ortográfico.
        /// \param top Límite superior del frustum ortográfico.
        /// \param bottom Límite inferior del frustum ortográfico.
        /// \param nearPlane Plano cercano (distancia positiva).
        /// \param farPlane Plano lejano (distancia positiva).
        /// \post Actualiza \c projection.
        void setOrthographicProjection(
            float left, float right, float top, float bottom, float nearPlane, float farPlane);

        /// \brief Configura proyección en perspectiva.
        /// \param verticalFov Campo de visión vertical (radianes).
        /// \param aspectRatio Relación de aspecto (ancho/alto).
        /// \param nearPlane Plano cercano (distancia positiva).
        /// \param farPlane Plano lejano (distancia positiva).
        /// \post Actualiza \c projection.
        void setPerspectiveProjection(
            float verticalFov, float aspectRatio, float nearPlane, float farPlane);

        /// \brief Define la vista a partir de posición y vector dirección.
        /// \details Construye la matriz de vista mirando desde \c position hacia
        /// \c direction (vector normalizado esperado), con \c up como
        /// referencia vertical.
        /// \param position Posición de la cámara en mundo.
        /// \param direction Vector dirección (hacia dónde mirar).
        /// \param up Vector "arriba" (por defecto, eje Y mundial).
        /// \post Actualiza \c view e \c inverseView.
        void lookAtDirection(
            glm::vec3 position,
            glm::vec3 direction,
            glm::vec3 up = glm::vec3{0.0f, 1.0f, 0.0f});

        /// \brief Define la vista a partir de posición y objetivo.
        /// \details Construye la matriz de vista mirando desde \c position hacia
        /// el punto \c target, con \c up como referencia vertical.
        /// \param position Posición de la cámara en mundo.
        /// \param target Punto objetivo a mirar.
        /// \param up Vector "arriba" (por defecto, eje Y mundial).
        /// \post Actualiza \c view e \c inverseView.
        void lookAtTarget(
            glm::vec3 position,
            glm::vec3 target,
            glm::vec3 up = glm::vec3{0.0f, 1.0f, 0.0f});

        /// \brief Define la vista con ángulos de Euler en orden Y-X-Z.
        /// \details Interpreta \c rotation como (yaw, pitch, roll) en radianes,
        /// y fija \c view/\c inverseView en función de \c position.
        /// \param position Posición de la cámara en mundo.
        /// \param rotation Rotación (yaw, pitch, roll) en radianes.
        /// \post Actualiza \c view e \c inverseView.
        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

        /// \brief Obtiene la matriz de proyección actual.
        /// \return Referencia constante a \c projection.
        const glm::mat4& getProjectionMatrix() const
        {
            return (projection);
        }

        /// \brief Obtiene la matriz de vista actual.
        /// \return Referencia constante a \c view.
        const glm::mat4& getViewMatrix() const
        {
            return (view);
        }

        /// \brief Obtiene la inversa de la matriz de vista actual.
        /// \return Referencia constante a \c inverseView.
        const glm::mat4& getInverseViewMatrix() const
        {
            return (inverseView);
        }

        /// \brief Devuelve la posición de la cámara en coordenadas de mundo.
        /// \details Extrae la traslación desde \c inverseView.
        /// \return Posición (x,y,z) de la cámara.
        const glm::vec3 getPosition() const
        {
            return glm::vec3(inverseView[3]);
        }

    private:
        /// Matriz de proyección (ortográfica o perspectiva).
        glm::mat4 projection {1.0f}; 

        /// Matriz de vista.
        glm::mat4 view {1.0f};        

        /// Inversa de vista.
        glm::mat4 inverseView {1.0f}; 
};
