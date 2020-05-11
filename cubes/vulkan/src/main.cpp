#include "evulkan.h"

#include <gflags/gflags.h>
#include <iostream>

DEFINE_bool(big_menu, true, "Include 'advanced' options in the menu listing");

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("Bob Loblaw");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

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
