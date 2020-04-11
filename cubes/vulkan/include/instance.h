#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "util.h"

class EVulkanInstance
{
public:
    static EVulkanInstance* instance();
    static void killInstance() { if(s_instance !=nullptr) delete s_instance;}
    
    EVulkanInstance(const EVulkanInstance&)=delete;
    EVulkanInstance& operator=(const EVulkanInstance& _other)=delete;

    void cleanup();

    struct EVkCreateWindow;
    struct EVkCreateInstance;
    struct EVkCreateSurface;
    struct EVkPickPhysicalDevice;

    void createWindow(EVkCreateWindow params, GLFWwindow *&window);
    void createInstance(EVkCreateInstance params, VkInstance *instance);
    void setupDebugMessenger(VkInstance instance);
    void createSurface(EVkCreateSurface params, VkSurfaceKHR *surface);
    void pickPhysicalDevice(EVkPickPhysicalDevice param);

    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
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
    ~EVulkanInstance();
};