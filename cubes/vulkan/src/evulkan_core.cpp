#include "evulkan_core.h"

#include <set>

void evkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const EVkDeviceCreateInfo *pCreateInfo,
    VkDevice *pDevice,
    VkQueue *pGraphicsQueue,
    VkQueue *pPresentQueue)
{
    QueueFamilyIndices indices = getQueueFamilies(physicalDevice, pCreateInfo->surface);

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

    createInfo.enabledExtensionCount = static_cast<uint32_t>(pCreateInfo->deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = pCreateInfo->deviceExtensions.data();

    if (ENABLE_VALIDATION) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(pCreateInfo->validationLayers.size());
        createInfo.ppEnabledLayerNames = pCreateInfo->validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, pDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device.");
    }

    vkGetDeviceQueue(*pDevice, indices.graphicsFamily.value(), 0, pGraphicsQueue);
    vkGetDeviceQueue(*pDevice, indices.presentFamily.value(), 0, pPresentQueue);
}

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
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