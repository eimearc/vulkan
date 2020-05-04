#pragma once

#include "evulkan_core.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <fstream>
#include <array>
#include <chrono>

#include "vertex.h"
#include "grid.h"
#include "util.h"
// #include "renderpass.h"
// #include "instance.h"

class EVulkan {
public:
    void run(size_t n=4) {
        numCubes = n;
        createGrid();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const int MAX_FRAMES_IN_FLIGHT = 2;

    GLFWwindow *window;
    VkSurfaceKHR surface;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<const char*> validationLayers =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };
    std::vector<const char*> deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    bool framebufferResized = false;

    VkDevice device;
    VkQueue presentQueue;
    VkQueue graphicsQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    void createGrid();
    void setupVertices();
    Grid grid;
    size_t numCubes = 4;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    // // swap.cpp
    // void recreateSwapChain();
    // VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    // VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    // VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    // void cleanupSwapChain();

    // // render.cpp
    // bool hasStencilComponent(VkFormat format);
    // void createImage(uint32_t width, uint32_t height, VkFormat format,
    //     VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    //     VkImage& image, VkDeviceMemory& imageMemory);
    // void transitionImageLayout(VkImage image, VkFormat format,
    //     VkImageLayout oldLayout, VkImageLayout newLayout);
    // void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};