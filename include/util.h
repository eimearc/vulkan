#pragma once

#include <vector>
#include <fstream>
#include <glm/glm.hpp>
#include "vertex.h"
#include "grid.h"
#include <optional>

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

// struct QueueFamilyIndices
// {
//     std::optional<uint32_t> graphicsFamily;
//     std::optional<uint32_t> presentFamily;

//     bool isComplete()
//     {
//         return graphicsFamily.has_value() && presentFamily.has_value();
//     }
// };

// struct SwapChainSupportDetails
// {
//     VkSurfaceCapabilitiesKHR capabilities;
//     std::vector<VkSurfaceFormatKHR> formats;
//     std::vector<VkPresentModeKHR> presentModes;
// };

std::vector<char> readFile(const std::string& filename);
void update(std::vector<Vertex> &vertices, const Grid &grid, size_t startOffset, size_t endOffset);
UniformBufferObject getUBO(const uint32_t &_width, const uint32_t &_height);