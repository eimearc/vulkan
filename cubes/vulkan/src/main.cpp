#include "evulkan.h"

#include <gflags/gflags.h>
#include "flags.h"
#include <iostream>

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for benchmarking Vulkan over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    EVulkan app;

    try
    {
        app.run(sqrt(FLAGS_num_cubes));
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    exit(0); // TODO: figure out why this doesn't work.
    return EXIT_SUCCESS;
}
