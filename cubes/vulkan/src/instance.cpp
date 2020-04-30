#include "evulkan_core.h"

#include <set>
#include <vector>
#include <string>
#include <iostream>


void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// EVulkanInstance::EVulkanInstance()
// {
//     // Device setup and stuff.
//     VkInstance instance;
//     GLFWwindow *window;
//     VkSurfaceKHR surface;
//     VkPhysicalDevice physicalDevice;

//     evkCreateWindow(EVkCreateWindow{}, window);

//     EVkCreateInstance instanceInfo{};
//     instanceInfo.appTitle = "Vulkan App";
//     instanceInfo.requiredExtensions = getRequiredExtensions();
//     instanceInfo.validationLayers = m_validationLayers;

//     evkCreateInstance(instanceInfo, &instance);

//     evkSetupDebugMessenger(instance);

//     EVkCreateSurface surfaceInfo{};
//     surfaceInfo.instance = instance;
//     surfaceInfo.window = window;
//     evkCreateSurface(surfaceInfo, &m_surface);
    
//     evkPickPhysicalDevice(instance, EVkPickPhysicalDevice{}, &physicalDevice);

//     m_window = window;
//     m_instance = instance;
//     m_physicalDevice = physicalDevice;
// }

void evkCleanup(VkInstance instance, GLFWwindow *window, VkSurfaceKHR surface, VkDebugUtilsMessengerEXT debugMessenger)
{
    // Device cleanup and stuff.
    if (ENABLE_VALIDATION)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

// void EVulkanInstance::framebufferResizeCallback(GLFWwindow* window, int width, int height)
// {
//     auto app = reinterpret_cast<EVulkanInstance*>(glfwGetWindowUserPointer(window));
//     app->m_framebufferResized = true;
// }

// void EVulkanInstance::evkCreateWindow(EVkCreateWindow params, GLFWwindow *&window)
// {
//     glfwInit(); // Initialize the GLFW library.
    
//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create OpenGL context.
//     glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

//     window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
//     glfwSetWindowUserPointer(window, this);
//     // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
// }

void evkCreateWindow(const EVkCreateWindow params, GLFWwindow *&window)
{
    glfwInit(); // Initialize the GLFW library.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create OpenGL context.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window=glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    // glfwSetWindowUserPointer(window, this);
    // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void evkCreateInstance(const EVkCreateInstance *pCreateInfo, VkInstance *instance)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = pCreateInfo->appTitle;
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(ENABLE_VALIDATION)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(pCreateInfo->validationLayers.size());
        createInfo.ppEnabledLayerNames = pCreateInfo->validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance->");
    }
}

std::vector<const char*> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLE_VALIDATION) 
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Macro for VK_EXT_debug_utils, used for debug messages.
    }

    return extensions;
}

// bool checkValidationLayerSupport()
// {
//     uint32_t layerCount;
//     vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

//     std::vector<VkLayerProperties> availableLayers(layerCount);
//     vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

//     for (const char* layerName : m_validationLayers)
//     {
//         bool layerFound = false;

//         for (const auto& layerProperties : availableLayers)
//         {
//             if (strcmp(layerName, layerProperties.layerName) == 0)
//             {
//                 layerFound = true;
//                 break;
//             }
//         }

//         if (!layerFound)
//         {
//             return false;
//         }
//     }

//     return true;
// }

void evkSetupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    if (!ENABLE_VALIDATION) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, pDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger.");
    }
}

void evkCreateSurface(VkInstance instance, const EVkSurfaceCreate *pCreateInfo, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, pCreateInfo->window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window instance->surface.");
    }
}

void evkPickPhysicalDevice(VkInstance instance, const EVkPickPhysicalDevice *pPickInfo, VkPhysicalDevice *physicalDevice)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, pPickInfo))
        {
            *physicalDevice = device;
            break;
        }
    }

    if (*physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find suitable GPU.");
    }
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, const EVkPickPhysicalDevice *pPickInfo)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, pPickInfo->surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, pPickInfo->surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, pPickInfo->surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, pPickInfo->surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, pPickInfo->surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const EVkPickPhysicalDevice *pPickInfo)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); //Crashes here.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, pPickInfo->surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
        }
        if (indices.isComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device, const EVkPickPhysicalDevice *pPickInfo)
{
    QueueFamilyIndices indices = findQueueFamilies(device, pPickInfo);

    bool extensionsSupported = checkDeviceExtensionSupport(device, pPickInfo);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, pPickInfo);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported
        && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const EVkPickPhysicalDevice *pPickInfo)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(pPickInfo->deviceExtensions.begin(), pPickInfo->deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}