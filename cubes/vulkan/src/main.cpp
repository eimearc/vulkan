#include "evulkan.h"

#include <gflags/gflags.h>
#include "flags.h"
#include <iostream>

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("Bob Loblaw");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::cout << FLAGS_big_menu << std::endl;

    EVulkan app;

    try
    {
        app.run(10);
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    exit(0); // TODO: figure out why this doesn't work.
    return EXIT_SUCCESS;
}
