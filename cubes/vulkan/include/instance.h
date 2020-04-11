#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "util.h"

struct EVulkanInstance
{
    EVulkanInstance();
    // EVulkanInstance(const EVulkanInstance& _other)=default;
    // EVulkanInstance& operator=(const EVulkanInstance& _other)=default;
    EVulkanInstance& operator=(EVulkanInstance&& _other)
    {
        std::cout << "ME\n";
        window = _other.window;
        std::cout << window << "\n" << _other.window << '\n';
        surface = _other.surface;
        instance = _other.instance;
        physicalDevice = _other.physicalDevice;
        debugMessenger = _other.debugMessenger;
        return *this;
    };
    ~EVulkanInstance();
    void cleanup();

    void initWindow();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();

    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    bool enableValidationLayers = true;

    GLFWwindow *window;
    VkSurfaceKHR surface;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
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
};