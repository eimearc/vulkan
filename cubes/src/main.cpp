#include "egl.h"
#include "evulkan.h"

#include <iostream>

int main()
{
    EGL gl;
    EVulkan vulkan;
    try
    {
        gl.run(10);
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    try
    {
        vulkan.run(10);
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}