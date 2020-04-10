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
    void run(size_t n=4)
    {
        numCubes = n;
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
    size_t numCubes = 4;
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
    void updateVertexBuffer();

    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
};
