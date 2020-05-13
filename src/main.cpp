#include "egl.h"
#include "evulkan.h"

#include <iostream>

int main()
{
    try
    {
        EGL gl;
        gl.run(10);
        EVulkan vulkan;
        vulkan.run(10);
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}