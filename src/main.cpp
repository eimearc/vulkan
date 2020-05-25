#include "egl.h"
#include "evulkan.h"

#include <iostream>

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for benchmarking Vulkan and OpenGL over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    try
    {
        EGL gl;
        gl.run();
        EVulkan vulkan;
        vulkan.run();
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}