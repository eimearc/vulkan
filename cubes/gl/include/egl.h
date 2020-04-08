#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

class EGL
{
public:
    void run()
    {
        initWindow();
        initGL();
        mainLoop();
        cleanup();   
    }
    
private:
    GLFWwindow* window;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    GLuint VAO;
    GLuint VBO;
    GLuint shaderProgram;
    GLuint EBO;

    void initWindow();
    void initGL();
    void mainLoop();
    void cleanup();

    void createShaders();
    void setupBuffers();

    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0";
};
