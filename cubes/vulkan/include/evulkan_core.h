#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include "util.h"

#define ENABLE_VALIDATION true

struct EVkDeviceCreateInfo
{
    VkSurfaceKHR surface;
    std::vector<const char *> deviceExtensions;
    std::vector<const char *> validationLayers;
};

void evkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const EVkDeviceCreateInfo *pCreateInfo,
    VkDevice *pDevice,
    VkQueue *pGraphicsQueue,
    VkQueue *pPresentQueue);
    
QueueFamilyIndices getQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);