#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <vector>

#include "grid.h"
#include "vertex.h"

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
    Grid grid;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    void initWindow();
    void initGL();
    void mainLoop();
    void cleanup();

    void createShaders();
    void createGrid();
    void setupBuffers();
    void setupVertices();

    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;

    // const char *vertexShaderSource = "#version 400\n"
    //     "#extension GL_ARB_separate_shader_objects : enable\n"
    //     "layout(location = 0) in vec3 aPos;\n"
    //     "layout(location = 1) in vec3 fragColor;\n"
    //     "layout(location = 0) out vec3 color;\n"
    //     "void main()\n"
    //     "{\n"
    //     "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    //     "   color = fragColor;\n"
    //     "}\0";
    // const char *fragmentShaderSource = "#version 400\n"
    //     "#extension GL_ARB_separate_shader_objects : enable\n"
    //     "layout(location = 0) in vec3 color;\n"
    //     "layout(location = 0) out vec4 FragColor;\n"
    //     "void main()\n"
    //     "{\n"
    //     "   FragColor = vec4(color, 1.0f);\n"
    //     "}\n\0";
};
