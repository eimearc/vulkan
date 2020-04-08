#pragma once

#include <glm/glm.hpp>
#include <vector>

class Cube
{
public:
    Cube()=default;
    Cube(glm::vec3 center, glm::vec3 color, float size)
    {
        float half = size/2.0f;
        float x = center.x + half;
        float y = center.y + half;
        float z = center.z + half;

        m_vertices = {
            // Top
            {-x, -y, z},  // bottom left
            {x, -y, z},   // bottom right
            {x, y, z},   // top right
            {-x, y, z},  // top left
            // Bottom
            {-x, -y, -z}, // bottom left
            {x, -y, -z},  // bottom right
            {x, y, -z},   // top right
            {-x, y, -z},
        };
        m_indices = {
            0, 1, 2, 2, 3, 0, // top
            4, 5, 6, 6, 7, 4, // bottom
            0, 4, 5, 5, 1, 0, // side 0
            1, 5, 6, 6, 2, 1, // side 1
            2, 6, 7, 7, 3, 2, // side 2
            3, 7, 4, 4, 0, 3  // side 3
        };
        m_color = color;
    }
    ~Cube()noexcept=default;
    std::vector<glm::vec3> vertices()
    {
        return m_vertices;
    }
    std::vector<uint16_t> indices()
    {
        return m_indices;
    }
    glm::vec3 color()
    {
        return m_color;
    }
private:
    glm::vec3 m_center;
    std::vector<glm::vec3> m_vertices;
    std::vector<uint16_t> m_indices;
    glm::vec3 m_color;
};