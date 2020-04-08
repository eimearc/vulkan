#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vertex.h>
#include <grid.h>

void update(std::vector<Vertex> &vertices, const Grid &grid)
{
    glm::mat3 rotate = glm::rotate(glm::mat4(1.0f), 0.01f * glm::radians(90.0f), glm::vec3(1.0f,0.0f,0.0f));
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        int j = floor(i/8);
        glm::vec3 center = grid.cubes[floor(i/8)].center;
        glm::vec3 tmp = vertices[i].pos;
        tmp -= center;
        tmp = rotate*tmp;
        tmp += center;
        vertices[i].pos = tmp;
    }
}