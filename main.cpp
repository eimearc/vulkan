#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    const int WIDTH = 800;
    const int HEIGHT = 600;

    void initWindow() {
        glfwInit(); // Initialize the GLFW library.
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create OpenGL context.
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window=glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {

    }

    void mainLoop() {
        while(!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }

    void cleanup() {
        glfwDestroyWindow(window);
        
        glfwTerminate();
    }

    GLFWwindow* window;
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}