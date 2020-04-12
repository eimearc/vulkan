#include "evulkan.h"

#define ENABLE_VALIDATON true

struct EVulkan::EVkDeviceCreateInfo
{
    VkPhysicalDevice physicalDevice;
    std::vector<const char *> deviceExtensions;
    std::vector<const char *> validationLayers;
};

void EVulkan::initVulkan()
{
    instance = EVulkanInstance::instance();

    EVkDeviceCreateInfo deviceInfo = {};
    deviceInfo.physicalDevice = instance->m_physicalDevice;
    deviceInfo.deviceExtensions = instance->m_deviceExtensions;
    deviceInfo.validationLayers = instance->m_validationLayers;
    VkInstance vInstance = instance->m_instance;
    evkCreateDevice(vInstance, &deviceInfo, &device);

    EVkSwapchainCreateInfo swapchainInfo = {};
    swapchainInfo.physicalDevice = instance->m_physicalDevice;
    swapchainInfo.surface = instance->m_surface;
    evkCreateSwapchain(device, &swapchainInfo, &swapChain);
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createCommandPool();

    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();

    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void EVulkan::evkCreateDevice(VkInstance _instance, const EVkDeviceCreateInfo *pCreateInfo, VkDevice *pDevice)
{
    QueueFamilyIndices indices = instance->findQueueFamilies(pCreateInfo->physicalDevice);

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

    if (ENABLE_VALIDATON) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(pCreateInfo->validationLayers.size());
        createInfo.ppEnabledLayerNames = pCreateInfo->validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(instance->m_physicalDevice, &createInfo, nullptr, pDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device.");
    }

    vkGetDeviceQueue(*pDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(*pDevice, indices.presentFamily.value(), 0, &presentQueue);
}

void EVulkan::mainLoop()
{
    int i = 0;
    std::chrono::steady_clock::time_point startTime, endTime;
    while(!glfwWindowShouldClose(instance->m_window))
    {
        glfwPollEvents();
        if ((i % 10) == 0) startTime = std::chrono::high_resolution_clock::now();
        drawFrame();
        if ((i % 10) == 0)
        {
            endTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();
            // std::cout << "Frame draw time: " << time << std::endl;
        }
        ++i;
    }

    vkDeviceWaitIdle(device);
}

void EVulkan::drawFrame()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device, swapChain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image.");
    }

    // Check if a previous frame is using this image. If so, wait on its fence.
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    // Mark the image as being in use.
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    // Update the uniform buffers.
    updateUniformBuffer(imageIndex);

    // Also need to upate the vertex buffers.
    updateVertexBuffer();
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image.");
    }

    vkQueueWaitIdle(presentQueue);

    currentFrame = (currentFrame+1) % MAX_FRAMES_IN_FLIGHT;
}

void EVulkan::cleanup()
{
    cleanupSwapChain();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    instance->cleanup(instance->m_instance, instance->m_window, instance->m_surface, instance->m_debugMessenger);
}