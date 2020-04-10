#include "evulkan.h"

void EVulkan::createLogicalDevice()
{
    QueueFamilyIndices indices = instance.findQueueFamilies(instance.physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(instance.deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = instance.deviceExtensions.data();

    if (instance.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(instance.validationLayers.size());
        std::cout << "Num validation layers:" << instance.validationLayers.size() << std::endl;
        createInfo.ppEnabledLayerNames = instance.validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(instance.physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device.");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

// void EVulkan::pickPhysicalDevice()
// {
//     uint32_t deviceCount = 0;
//     vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

//     if (deviceCount == 0)
//     {
//         throw std::runtime_error("failed to find GPUs with Vulkan support.");
//     }

//     std::vector<VkPhysicalDevice> devices(deviceCount);
//     vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

//     for (const auto& device : devices)
//     {
//         if (isDeviceSuitable(device))
//         {
//             instance.physicalDevice = device;
//             break;
//         }
//     }

//     if (instance.physicalDevice == VK_NULL_HANDLE)
//     {
//         throw std::runtime_error("failed to find suitable GPU.");
//     }
// }

// QueueFamilyIndices EVulkan::findQueueFamilies(VkPhysicalDevice device)
// {
//     QueueFamilyIndices indices;

//     uint32_t queueFamilyCount = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
//     std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//     vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

//     int i = 0;
//     for (const auto& queueFamily : queueFamilies)
//     {
//         if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
//         {
//             indices.graphicsFamily = i;

//             VkBool32 presentSupport = false;
//             vkGetPhysicalDeviceSurfaceSupportKHR(device, i, instance.surface, &presentSupport);
//             if (presentSupport)
//             {
//                 indices.presentFamily = i;
//             }
//         }
//         if (indices.isComplete())
//         {
//             break;
//         }
//         i++;
//     }

//     return indices;
// }

// bool EVulkan::isDeviceSuitable(VkPhysicalDevice device)
// {
//     QueueFamilyIndices indices = findQueueFamilies(device);

//     bool extensionsSupported = checkDeviceExtensionSupport(device);

//     bool swapChainAdequate = false;
//     if (extensionsSupported)
//     {
//         SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
//         swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
//     }

//     VkPhysicalDeviceFeatures supportedFeatures;
//     vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

//     return indices.isComplete() && extensionsSupported
//         && swapChainAdequate && supportedFeatures.samplerAnisotropy;
// }

// bool EVulkan::checkDeviceExtensionSupport(VkPhysicalDevice device)
// {
//     uint32_t extensionCount;
//     vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
//     std::vector<VkExtensionProperties> availableExtensions(extensionCount);
//     vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

//     std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

//     for (const auto& extension : availableExtensions)
//     {
//         requiredExtensions.erase(extension.extensionName);
//     }

//     return requiredExtensions.empty();
// }

// void EVulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
// {
//     createInfo = {};
//     createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//     createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
//         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
//         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//     createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
//         | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
//         | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//     createInfo.pfnUserCallback = debugCallback;
//     createInfo.pUserData = nullptr;
// }

// VKAPI_ATTR VkBool32 VKAPI_CALL EVulkan::debugCallback(
//     VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//     VkDebugUtilsMessageTypeFlagsEXT messageType,
//     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
//     void* pUserData)
// {
//     std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

//     return VK_FALSE;
// }