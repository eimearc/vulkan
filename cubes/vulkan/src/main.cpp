#include "evulkan.h"

#include <iostream>

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for benchmarking Vulkan over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    EVulkan app;

    try
    {
        app.run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    exit(0); // TODO: figure out why this doesn't work.
    return EXIT_SUCCESS;
}
