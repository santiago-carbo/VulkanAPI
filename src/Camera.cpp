/*
 * Project: VulkanAPI
 * File: Camera.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "Camera.hpp"

/// \brief Configura proyección ortográfica.
/// \param left Límite izquierdo del frustum ortográfico.
/// \param right Límite derecho del frustum ortográfico.
/// \param top Límite superior del frustum ortográfico.
/// \param bottom Límite inferior del frustum ortográfico.
/// \param nearPlane Plano cercano (distancia positiva).
/// \param farPlane Plano lejano (distancia positiva).
/// \post Actualiza \c projection.
void Camera::setOrthographicProjection(
    float left, float right, float top, float bottom, float nearPlane, float farPlane)
{
    projection = glm::mat4{ 1.0f };
    projection[0][0] = 2.0f / (right - left);
    projection[1][1] = 2.0f / (bottom - top);
    projection[2][2] = 1.0f / (farPlane - nearPlane);
    projection[3][0] = -(right + left) / (right - left);
    projection[3][1] = -(bottom + top) / (bottom - top);
    projection[3][2] = -nearPlane / (farPlane - nearPlane);
}

/// \brief Configura proyección en perspectiva.
/// \param verticalFov Campo de visión vertical (radianes).
/// \param aspectRatio Relación de aspecto (ancho/alto).
/// \param nearPlane Plano cercano (distancia positiva).
/// \param farPlane Plano lejano (distancia positiva).
/// \post Actualiza \c projection.
void Camera::setPerspectiveProjection(
    float verticalFov, float aspectRatio, float nearPlane, float farPlane)
{
    assert(glm::abs(aspectRatio - std::numeric_limits<float>::epsilon()) > 0.0f);

    const float tanHalfFov = tan(verticalFov / 2.0f);

    projection = glm::mat4{0.0f};
    projection[0][0] = 1.0f / (aspectRatio * tanHalfFov);
    projection[1][1] = 1.0f / tanHalfFov;
    projection[2][2] = farPlane / (farPlane - nearPlane);
    projection[2][3] = 1.0f;
    projection[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
}

/// \brief Define la vista a partir de posición y vector dirección.
/// \details Construye la matriz de vista mirando desde \c position hacia
/// \c direction (vector normalizado esperado), con \c up como
/// referencia vertical.
/// \param position Posición de la cámara en mundo.
/// \param direction Vector dirección (hacia dónde mirar).
/// \param up Vector "arriba" (por defecto, eje Y mundial).
/// \post Actualiza \c view e \c inverseView.
void Camera::lookAtDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    const glm::vec3 w = glm::normalize(direction);
    const glm::vec3 u = glm::normalize(glm::cross(w, up));
    const glm::vec3 v = glm::cross(w, u);

    view = glm::mat4{1.0f};
    view[0][0] = u.x;
    view[1][0] = u.y;
    view[2][0] = u.z;
    view[0][1] = v.x;
    view[1][1] = v.y;
    view[2][1] = v.z;
    view[0][2] = w.x;
    view[1][2] = w.y;
    view[2][2] = w.z;
    view[3][0] = -glm::dot(u, position);
    view[3][1] = -glm::dot(v, position);
    view[3][2] = -glm::dot(w, position);

    inverseView = glm::mat4{1.0f};
    inverseView[0][0] = u.x;
    inverseView[0][1] = u.y;
    inverseView[0][2] = u.z;
    inverseView[1][0] = v.x;
    inverseView[1][1] = v.y;
    inverseView[1][2] = v.z;
    inverseView[2][0] = w.x;
    inverseView[2][1] = w.y;
    inverseView[2][2] = w.z;
    inverseView[3][0] = position.x;
    inverseView[3][1] = position.y;
    inverseView[3][2] = position.z;
}

/// \brief Define la vista a partir de posición y objetivo.
/// \details Construye la matriz de vista mirando desde \c position hacia
/// el punto \c target, con \c up como referencia vertical.
/// \param position Posición de la cámara en mundo.
/// \param target Punto objetivo a mirar.
/// \param up Vector "arriba" (por defecto, eje Y mundial).
/// \post Actualiza \c view e \c inverseView.
void Camera::lookAtTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    lookAtDirection(position, target - position, up);
}

/// \brief Define la vista con ángulos de Euler en orden Y-X-Z.
/// \details Interpreta \c rotation como (yaw, pitch, roll) en radianes,
/// y fija \c view/\c inverseView en función de \c position.
/// \param position Posición de la cámara en mundo.
/// \param rotation Rotación (yaw, pitch, roll) en radianes.
/// \post Actualiza \c view e \c inverseView.
void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);

    const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
    const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
    const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };

    view = glm::mat4{1.0f};
    view[0][0] = u.x;
    view[1][0] = u.y;
    view[2][0] = u.z;
    view[0][1] = v.x;
    view[1][1] = v.y;
    view[2][1] = v.z;
    view[0][2] = w.x;
    view[1][2] = w.y;
    view[2][2] = w.z;
    view[3][0] = -glm::dot(u, position);
    view[3][1] = -glm::dot(v, position);
    view[3][2] = -glm::dot(w, position);

    inverseView = glm::mat4{ 1.0f };
    inverseView[0][0] = u.x;
    inverseView[0][1] = u.y;
    inverseView[0][2] = u.z;
    inverseView[1][0] = v.x;
    inverseView[1][1] = v.y;
    inverseView[1][2] = v.z;
    inverseView[2][0] = w.x;
    inverseView[2][1] = w.y;
    inverseView[2][2] = w.z;
    inverseView[3][0] = position.x;
    inverseView[3][1] = position.y;
    inverseView[3][2] = position.z;
}


