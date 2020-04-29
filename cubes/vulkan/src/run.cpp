#include "evulkan.h"

void EVulkan::initVulkan()
{
    instance = EVulkanInstance::instance();

    EVkDeviceCreateInfo deviceInfo = {};
    deviceInfo.deviceExtensions = instance->m_deviceExtensions;
    deviceInfo.validationLayers = instance->m_validationLayers;
    deviceInfo.surface = instance->m_surface;
    VkPhysicalDevice physicalDevice = instance->m_physicalDevice;
    evkCreateDevice(physicalDevice, &deviceInfo, &device, &graphicsQueue, &presentQueue);

    EVkSwapchainCreateInfo swapchainInfo = {};
    swapchainInfo.physicalDevice = instance->m_physicalDevice;
    swapchainInfo.surface = instance->m_surface;
    swapchainInfo.window = instance->m_window;
    evkCreateSwapchain(device, &swapchainInfo, &swapChain, &swapChainImages, &swapChainImageFormat, &swapChainExtent);
    
    EVkImageViewsCreateInfo imageViewsInfo = {};
    imageViewsInfo.images = swapChainImages;
    imageViewsInfo.swapChainImageFormat = swapChainImageFormat;
    evkCreateImageViews(device, &imageViewsInfo, &swapChainImageViews);

    EVkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.swapChainImageFormat = swapChainImageFormat;
    renderPassInfo.physicalDevice = instance->m_physicalDevice;
    evkCreateRenderPass(device, &renderPassInfo, &renderPass);

    EVkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
    evkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, &descriptorSetLayout);

    EVkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertShaderFile = "shaders/vert.spv";
    pipelineInfo.fragShaderFile = "shaders/frag.spv";
    pipelineInfo.swapchainExtent = swapChainExtent;
    pipelineInfo.pDescriptorSetLayout = &descriptorSetLayout;
    pipelineInfo.renderPass = renderPass;
    evkCreateGraphicsPipeline(device, &pipelineInfo, &pipelineLayout, &graphicsPipeline);

    EVkDepthResourcesCreateInfo depthResourcesInfo = {};
    depthResourcesInfo.physicalDevice = instance->m_physicalDevice;
    depthResourcesInfo.swapchainExtent = swapChainExtent;
    depthResourcesInfo.swapchainImageFormat = swapChainImageFormat;
    evkCreateDepthResources(device, &depthResourcesInfo, &depthImage, &depthImageView, &depthImageMemory);

    EVkFramebuffersCreateInfo framebuffersInfo = {};
    framebuffersInfo.swapchainExtent = swapChainExtent;
    framebuffersInfo.swapchainImageViews = swapChainImageViews;
    framebuffersInfo.renderPass = renderPass;
    framebuffersInfo.depthImageView = depthImageView;
    evkCreateFramebuffers(device, &framebuffersInfo, &swapChainFramebuffers);

    EVkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.physicalDevice = instance->m_physicalDevice;
    commandPoolInfo.surface = instance->m_surface;
    evkCreateCommandPool(device, &commandPoolInfo, &commandPool);

    EVkVertexBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.commandPool = commandPool;
    vertexBufferInfo.physicalDevice = instance->m_physicalDevice;
    vertexBufferInfo.queue = graphicsQueue;
    vertexBufferInfo.vertices = vertices;
    evkCreateVertexBuffer(device, &vertexBufferInfo, &vertexBuffer, &vertexBufferMemory);

    EVkIndexBufferCreateInfo indexBufferInfo = {};
    indexBufferInfo.commandPool = commandPool;
    indexBufferInfo.physicalDevice = instance->m_physicalDevice;
    indexBufferInfo.queue = graphicsQueue;
    indexBufferInfo.indices = indices;
    evkCreateIndexBuffer(device, &indexBufferInfo, &indexBuffer, &indexBufferMemory);

    EVkUniformBufferCreateInfo uniformBufferInfo = {};
    uniformBufferInfo.physicalDevice = instance->m_physicalDevice;
    uniformBufferInfo.swapchainImages = swapChainImages;
    evkCreateUniformBuffers(device, &uniformBufferInfo, &uniformBuffers, &uniformBuffersMemory);

    EVkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.swapchainImages = swapChainImages;
    evkCreateDescriptorPool(device, &descriptorPoolInfo, &descriptorPool);

    EVkDescriptorSetCreateInfo descriptorSetInfo = {};
    descriptorSetInfo.descriptorPool = descriptorPool;
    descriptorSetInfo.descriptorSetLayout = descriptorSetLayout;
    descriptorSetInfo.swapchainImages = swapChainImages;
    descriptorSetInfo.uniformBuffers = uniformBuffers;
    evkCreateDescriptorSets(device, &descriptorSetInfo, &descriptorSets);

    EVkCommandBuffersCreateInfo commandBuffersInfo = {};
    commandBuffersInfo.commandPool = commandPool;
    commandBuffersInfo.descriptorSets = descriptorSets;
    commandBuffersInfo.graphicsPipeline = graphicsPipeline;
    commandBuffersInfo.indexBuffer = indexBuffer;
    commandBuffersInfo.indices = indices;
    commandBuffersInfo.pipelineLayout = pipelineLayout;
    commandBuffersInfo.renderPass = renderPass;
    commandBuffersInfo.swapchainExtent = swapChainExtent;
    commandBuffersInfo.swapchainFramebuffers = swapChainFramebuffers;
    commandBuffersInfo.vertexBuffer = vertexBuffer;
    evkCreateCommandBuffers(device, &commandBuffersInfo, &commandBuffers);

    EVkSyncObjectsCreateInfo syncObjectsInfo = {};
    syncObjectsInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    syncObjectsInfo.swapchainSize = swapChainImages.size();
    evkCreateSyncObjects(device, &syncObjectsInfo, &imageAvailableSemaphores, &renderFinishedSemaphores, &inFlightFences, &imagesInFlight);
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
    uint32_t imageIndex;
    EVkDrawFrameInfo info = {};
    info.pInFlightFences = &inFlightFences;
    info.pImageAvailableSemaphores = &imageAvailableSemaphores;
    info.swapchain = swapChain;
    info.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    info.pCommandBuffers = &commandBuffers;
    info.graphicsQueue = graphicsQueue;
    info.presentQueue = presentQueue;
    info.pFramebufferResized = &framebufferResized;
    info.swapchainExtent = swapChainExtent;
    info.pUniformBufferMemory = &uniformBuffersMemory;
    info.pVertices = &vertices;
    info.physicalDevice = instance->m_physicalDevice;
    info.commandPool = commandPool;
    info.vertexBuffer = vertexBuffer;
    info.grid = grid;
    evkDrawFrame(device, &info, &currentFrame, &imagesInFlight, &renderFinishedSemaphores, &imageIndex);

    // vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // VkResult result = vkAcquireNextImageKHR(
    //     device, swapChain, UINT64_MAX,
    //     imageAvailableSemaphores[currentFrame],
    //     VK_NULL_HANDLE, &imageIndex);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    // {
    //     // recreateSwapChain();
    //     return;
    // }
    // else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    // {
    //     throw std::runtime_error("failed to acquire swap chain image.");
    // }

    // // Check if a previous frame is using this image. If so, wait on its fence.
    // if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    // {
    //     vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    // }

    // // Mark the image as being in use.
    // imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    // // Update the uniform buffers.
    // updateUniformBuffer(imageIndex);

    // // Also need to upate the vertex buffers.
    // updateVertexBuffer();
    
    // VkSubmitInfo submitInfo = {};
    // submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // submitInfo.waitSemaphoreCount = 1;
    // submitInfo.pWaitSemaphores = waitSemaphores;
    // submitInfo.pWaitDstStageMask = waitStages;

    // submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    // VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    // submitInfo.signalSemaphoreCount = 1;
    // submitInfo.pSignalSemaphores = signalSemaphores;

    // vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to submit draw command buffer!");
    // }

    // VkPresentInfoKHR presentInfo = {};
    // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // presentInfo.waitSemaphoreCount = 1;
    // presentInfo.pWaitSemaphores = signalSemaphores;
    // VkSwapchainKHR swapChains[] = {swapChain};
    // presentInfo.swapchainCount = 1;
    // presentInfo.pSwapchains = swapChains;
    // presentInfo.pImageIndices = &imageIndex;
    // presentInfo.pResults = nullptr;

    // VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    // {
    //     framebufferResized = false;
    //     // recreateSwapChain();
    // }
    // else if (result != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to present swap chain image.");
    // }

    // vkQueueWaitIdle(presentQueue);

    // currentFrame = (currentFrame+1) % MAX_FRAMES_IN_FLIGHT;
}

void EVulkan::cleanup()
{
    EVkSwapchainCleanupInfo cleanupInfo = {};
    cleanupInfo.depthImage = depthImage;
    cleanupInfo.depthImageView = depthImageView;
    cleanupInfo.depthImageMemory = depthImageMemory;
    cleanupInfo.swapchainFramebuffers = swapChainFramebuffers;
    cleanupInfo.commandPool = commandPool;
    cleanupInfo.pCommandBuffers = &commandBuffers;
    cleanupInfo.graphicsPipeline = graphicsPipeline;
    cleanupInfo.pipelineLayout = pipelineLayout;
    cleanupInfo.renderPass = renderPass;
    cleanupInfo.swapchainImageViews = swapChainImageViews;
    cleanupInfo.swapchain = swapChain;
    cleanupInfo.uniformBuffers = uniformBuffers;
    cleanupInfo.uniformBuffersMemory = uniformBuffersMemory;
    cleanupInfo.descriptorPool = descriptorPool;
    cleanupInfo.swapchainImages = swapChainImages;
    evkCleanupSwapchain(device, &cleanupInfo);

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