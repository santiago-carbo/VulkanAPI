/*
 * Project: VulkanAPI
 * File: main.cpp
 * Author: Santiago Carbó García
 * SPDX-License-Identifier: MIT
 *
 */

#include "VulkanApplication.hpp"

#include <iostream>

/// \brief Punto de entrada de la aplicación.
/// \details Crea una instancia de \c VulkanApplication y ejecuta su bucle principal con \c run.
/// Gestiona excepciones de tipo \c std::exception para imprimir un mensaje de error
/// y devolver un código de salida distinto de cero en caso de fallo.
int main()
{
    try
    {
        VulkanApplication app;
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "💥[Vulkan API] Unhandled exception: " << e.what() << '\n';

        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}