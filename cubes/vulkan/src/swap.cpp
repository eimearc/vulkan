#include "evulkan.h"

void EVulkan::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(instance->m_window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(instance->m_window, &width, &height);
        glfwWaitEvents();
    }

    // Wait until nobody is using the device.
    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    EVkSwapchainCreateInfo swapchainInfo = {};
    swapchainInfo.physicalDevice = instance->m_physicalDevice;
    swapchainInfo.surface = instance->m_surface;
    swapchainInfo.window = instance->m_window;
    evkCreateSwapchain(device, &swapchainInfo, &swapChain, &swapChainImages, &swapChainImageFormat, &swapChainExtent);

    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

// EVulkan::SwapChainSupportDetails EVulkan::querySwapChainSupport(VkPhysicalDevice device)
// {
//     SwapChainSupportDetails details;

//     vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, instance->surface, &details.capabilities);

//     uint32_t formatCount;
//     vkGetPhysicalDeviceSurfaceFormatsKHR(device, instance->surface, &formatCount, nullptr);
//     if (formatCount != 0)
//     {
//         details.formats.resize(formatCount);
//         vkGetPhysicalDeviceSurfaceFormatsKHR(device, instance->surface, &formatCount, details.formats.data());
//     }

//     uint32_t presentModeCount;
//     vkGetPhysicalDeviceSurfacePresentModesKHR(device, instance->surface, &presentModeCount, nullptr);
//     if (presentModeCount != 0)
//     {
//         details.presentModes.resize(presentModeCount);
//         vkGetPhysicalDeviceSurfacePresentModesKHR(device, instance->surface, &presentModeCount, details.presentModes.data());
//     }

//     return details;
// }

// VkSurfaceFormatKHR EVulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
// {
//     for (const auto& availableFormat : availableFormats)
//     {
//         if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
//         {
//             return availableFormat;
//         }
//     }

//     throw std::runtime_error("no suitable format found in available formats.");
// }

// VkPresentModeKHR EVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
// {
//     for (const auto& availablePresentMode : availablePresentModes)
//     {
//         if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
//         {
//             return availablePresentMode;
//         }
//     }

//     return VK_PRESENT_MODE_FIFO_KHR;
// }

// VkExtent2D EVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
// {
//     if (capabilities.currentExtent.width != UINT32_MAX)
//     {
//         return capabilities.currentExtent;
//     }
//     else
//     {
//         int width, height;
//         glfwGetFramebufferSize(instance->m_window, &width, &height);

//         VkExtent2D actualExtent =
//         {
//             static_cast<uint32_t>(width),
//             static_cast<uint32_t>(height)
//         };

//         actualExtent.width = std::max(capabilities.minImageExtent.width,
//             std::min(capabilities.maxImageExtent.width, actualExtent.width));
//         actualExtent.height = std::max(capabilities.minImageExtent.height,
//             std::min(capabilities.maxImageExtent.height, actualExtent.height));

//         return actualExtent;
//     }   
// }

void EVulkan::cleanupSwapChain()
{
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    for (auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}