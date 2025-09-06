/*
 * Project: VulkanAPI
 * File: GameObject.hpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "Model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

 /// \brief Transformación de un objeto en el espacio 3D.
 /// \details Contiene traslación, escala y rotación (en radianes, convención YXZ
 /// si se usa junto a utilidades tipo \c setViewYXZ). Expone utilidades
 /// para obtener la matriz de modelo y la matriz de normales.
struct Transform
{
    /// Traslación del objeto en coordenadas de mundo.
    glm::vec3 translation {};

    /// Escala no uniforme (xyz).
    glm::vec3 scale {1.0f, 1.0f, 1.0f};

    /// Rotación en radianes.
    glm::vec3 rotation {};

    /// \brief Devuelve la matriz de modelo a partir de T, R y S.
    /// \return Matriz 4x4 para transformar de espacio local a mundo.
    glm::mat4 matrix() const;

    /// \brief Devuelve la matriz de normales asociada a la transformación.
    /// \details Normalmente es \c transpose(inverse(mat3(M))) para mantener
    /// las normales correctas ante escalas no uniformes.
    /// \return Matriz 3x3 para transformar normales a espacio de mundo.
    glm::mat3 normalMatrix() const;
};

/// \brief Componente de luz puntual asociable a un objeto.
/// \details Define la intensidad luminosa del emisor. El color se toma del
/// propio \c GameObject::color.
struct PointLight
{
    float intensity = 1.0f; ///< Intensidad de la luz (factor multiplicativo).
};

/// \brief Entidad mínima renderizable o lógica de la escena.
/// \details Agrega color, transform, una malla opcional (\c Model) y una luz
/// puntual. Se identifica de forma única por un \c id.
class GameObject
{
    public:
        /// \brief Crea un nuevo objeto con \c id único.
        /// \details El \c id se genera internamente (normalmente de un contador global).
        /// \return Instancia de \c GameObject.
        static GameObject create();

        /// \brief Factoría para crear un \c GameObject con componente de luz puntual.
        /// \details Inicializa \c light y establece color, intensidad y un radio
        /// lógico si la representación lo requiere (p.ej., para debug).
        /// \param intensity Intensidad luminosa inicial.
        /// \param radius Radio lógico (útil si se visualiza la luz como esfera).
        /// \param color Color base del emisor (se usa \c GameObject::color).
        /// \return \c GameObject configurado como luz puntual.
        static GameObject makePointLight(
            float intensity = 10.f,
            float radius = 0.1f,
            glm::vec3 color = glm::vec3(1.f));

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        /// \brief Devuelve el identificador único del objeto.
        /// \return \c id del objeto.
        unsigned int getId() const
        {
            return (id);
        }

        /// Color base del objeto o emisor (RGB en [0,1]).
        glm::vec3 color {};

        /// Transformación de coordenadas locales a coordenadas de mundo.
        Transform transform{};

        /// Malla asociada.
        std::shared_ptr<Model> model{};

        /// Componente de luz puntual.
        std::unique_ptr<PointLight> light = nullptr;

    private:
        /// \brief Constructor privado.
        /// \param objId Identificador único asignado externamente.
        explicit GameObject(unsigned int objId) : id{ objId } {}

        /// Identificador único del objeto en la escena.
        unsigned int id;
};

