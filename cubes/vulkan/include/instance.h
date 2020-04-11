#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "util.h"

class EVulkanInstance
{
public:
    static EVulkanInstance* instance();
    EVulkanInstance(const EVulkanInstance&)=delete;
    EVulkanInstance& operator=(const EVulkanInstance& _other)=delete;
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

    bool m_enableValidationLayers = true;

    GLFWwindow *m_window;
    VkSurfaceKHR m_surface;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    std::vector<const char*> m_validationLayers =
    {
        "VK_LAYER_LUNARG_standard_validation"
    };
    std::vector<const char*> m_deviceExtensions = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    bool m_framebufferResized = false;
    static EVulkanInstance *s_instance;

private:
    EVulkanInstance();
};