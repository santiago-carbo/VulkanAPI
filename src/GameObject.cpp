/*
 * Project: VulkanAPI
 * File: GameObject.cpp
 * Author: Santiago Carb� Garc�a
 * SPDX-License-Identifier: MIT
 *
 */

#include "GameObject.hpp"

 /// \brief Devuelve la matriz de modelo a partir de T, R y S.
 /// \return Matriz 4x4 para transformar de espacio local a mundo.
glm::mat4 Transform::matrix() const
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);

    return glm::mat4{
        {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f} };
}

/// \brief Devuelve la matriz de normales asociada a la transformaci�n.
/// \details Normalmente es \c transpose(inverse(mat3(M))) para mantener
/// las normales correctas ante escalas no uniformes.
/// \return Matriz 3x3 para transformar normales a espacio de mundo.
glm::mat3 Transform::normalMatrix() const
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 invScale = 1.0f / scale;

    return glm::mat3{
        {
            invScale.x * (c1 * c3 + s1 * s2 * s3),
            invScale.x * (c2 * s3),
            invScale.x * (c1 * s2 * s3 - c3 * s1),
        },
        {
            invScale.y * (c3 * s1 * s2 - c1 * s3),
            invScale.y * (c2 * c3),
            invScale.y * (c1 * c3 * s2 + s1 * s3),
        },
        {
            invScale.z * (c2 * s1),
            invScale.z * (-s2),
            invScale.z * (c1 * c2),
        },
    };
}

/// \brief Crea un nuevo objeto con \c id �nico.
/// \details El \c id se genera internamente (normalmente de un contador global).
/// \return Instancia de \c GameObject.
GameObject GameObject::create()
{
    static unsigned int currentId = 0;

    return GameObject{currentId++};
}

/// \brief Factor�a para crear un \c GameObject con componente de luz puntual.
/// \details Inicializa \c light y establece color, intensidad y un radio
/// l�gico si la representaci�n lo requiere (p.ej., para debug).
/// \param intensity Intensidad luminosa inicial.
/// \param radius Radio l�gico (�til si se visualiza la luz como esfera).
/// \param color Color base del emisor (se usa \c GameObject::color).
/// \return \c GameObject configurado como luz puntual.
GameObject GameObject::makePointLight(float intensity, float radius, glm::vec3 color)
{
    GameObject obj = GameObject::create();
    obj.color = color;
    obj.transform.scale.x = radius;
    obj.light = std::make_unique<PointLight>();
    obj.light->intensity = intensity;

    return (obj);
}

