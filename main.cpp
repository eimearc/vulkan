#include "evulkan.cpp"

#include <iostream>

int main()
{
    CubeApplication app;
    try
    {
        std::cout << "Running app\n";
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}