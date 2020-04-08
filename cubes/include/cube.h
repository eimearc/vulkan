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
        float pos_x = center.x + half;
        float neg_x = center.x - half;
        float pos_y = center.y + half;
        float neg_y = center.y - half;
        float pos_z = center.z + half;
        float neg_z = center.z - half;

        m_vertices = {
            // Top
            {neg_x, neg_y, pos_z},  // bottom left
            {pos_x, neg_y, pos_z},   // bottom right
            {pos_x, pos_y, pos_z},   // top right
            {neg_x, pos_y, pos_z},  // top left
            // Bottom
            {neg_x, neg_y, neg_z}, // bottom left
            {pos_x, neg_y, neg_z},  // bottom right
            {pos_x, pos_y, neg_z},   // top right
            {neg_x, pos_y, neg_z},
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
        m_center = center;
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
    glm::vec3 center()
    {
        return m_center;
    }
private:
    glm::vec3 m_center;
    std::vector<glm::vec3> m_vertices;
    std::vector<uint16_t> m_indices;
    glm::vec3 m_color;
};