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
#include <fstream>
#include <iostream>

#include "vertex.h"
#include "grid.h"
#include "util.h"
#include "bench.h"

class EVulkan {
public:
    void run() {
        bench.open("time_vulkan.csv", FLAGS_overwrite);
        createGrid();
        initVulkan();
        mainLoop();
        cleanup();
        bench.close();
    }

private:
    const size_t NUM_CUBES = sqrt(FLAGS_num_cubes);

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const int MAX_FRAMES_IN_FLIGHT = 2; // Must be greater than minImageCount.

    Bench bench;

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
    VkCommandBuffer primaryCommandBuffer;
    std::vector<VkCommandBuffer> secondaryCommandBuffers;

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
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
};