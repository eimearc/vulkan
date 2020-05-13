#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <vector>

#include "grid.h"
#include "vertex.h"
#include "flags.h"

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
    size_t NUM_CUBES = sqrt(FLAGS_num_cubes);
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
    void updateVertexBuffer();

    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
};
