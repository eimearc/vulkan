#include "egl.h"

int main()
{
    EGL app;
    try
    {
        app.run(10);
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}