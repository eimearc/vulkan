#include "egl.h"

int main(int argc, char **argv)
{
    gflags::SetUsageMessage("A program for benchmarking OpenGL over multiple threads.");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::cout << "num cubes in main: " << FLAGS_num_cubes << std::endl;

    EGL app;
    try
    {
        app.run();
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}