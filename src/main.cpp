#include "VulkanApplication.hpp"

#include <iostream>

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