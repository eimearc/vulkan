#include "util.h"

std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file.");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

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