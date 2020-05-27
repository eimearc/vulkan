#include "evulkan.h"

void EVulkan::initVulkan()
{
    evkCreateWindow(EVkCreateWindow{}, window);

    EVkCreateInstance instanceInfo = {};
    instanceInfo.appTitle = "Vulkan App";
    instanceInfo.extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    instanceInfo.validationLayers = validationLayers;
    evkCreateInstance(&instanceInfo, &instance);

    evkSetupDebugMessenger(instance, &debugMessenger);

    EVkSurfaceCreate surfaceInfo{};
    surfaceInfo.window = window;
    evkCreateSurface(instance, &surfaceInfo, &surface);
    
    EVkPickPhysicalDevice pickInfo = {};
    pickInfo.surface = surface;
    evkPickPhysicalDevice(instance, &pickInfo, &physicalDevice);

    EVkDeviceCreateInfo deviceInfo = {};
    deviceInfo.deviceExtensions = deviceExtensions;
    deviceInfo.validationLayers = validationLayers;
    deviceInfo.surface = surface;
    evkCreateDevice(physicalDevice, &deviceInfo, &device, &graphicsQueue, &presentQueue);

    EVkSwapchainCreateInfo swapchainInfo = {};
    swapchainInfo.physicalDevice = physicalDevice;
    swapchainInfo.surface = surface;
    swapchainInfo.window = window;
    swapchainInfo.numImages = MAX_FRAMES_IN_FLIGHT;
    evkCreateSwapchain(device, &swapchainInfo, &swapChain, &swapChainImages, &swapChainImageFormat, &swapChainExtent);
    
    EVkImageViewsCreateInfo imageViewsInfo = {};
    imageViewsInfo.images = swapChainImages;
    imageViewsInfo.swapChainImageFormat = swapChainImageFormat;
    evkCreateImageViews(device, &imageViewsInfo, &swapChainImageViews);

    EVkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.swapChainImageFormat = swapChainImageFormat;
    renderPassInfo.physicalDevice = physicalDevice;
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
    depthResourcesInfo.physicalDevice = physicalDevice;
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
    commandPoolInfo.physicalDevice = physicalDevice;
    commandPoolInfo.surface = surface;
    commandPoolInfo.flags = 0;
    evkCreateCommandPool(device, &commandPoolInfo, &commandPool);

    EVkVertexBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.commandPool = commandPool;
    vertexBufferInfo.physicalDevice = physicalDevice;
    vertexBufferInfo.queue = graphicsQueue;
    vertexBufferInfo.vertices = vertices;
    evkCreateVertexBuffer(device, &vertexBufferInfo, &vertexBuffer, &vertexBufferMemory);

    EVkIndexBufferCreateInfo indexBufferInfo = {};
    indexBufferInfo.commandPool = commandPool;
    indexBufferInfo.physicalDevice = physicalDevice;
    indexBufferInfo.queue = graphicsQueue;
    indexBufferInfo.indices = indices;
    evkCreateIndexBuffer(device, &indexBufferInfo, &indexBuffer, &indexBufferMemory);

    EVkUniformBufferCreateInfo uniformBufferInfo = {};
    uniformBufferInfo.physicalDevice = physicalDevice;
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

    EVkSyncObjectsCreateInfo syncObjectsInfo = {};
    syncObjectsInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    syncObjectsInfo.swapchainSize = swapChainImages.size();
    evkCreateSyncObjects(device, &syncObjectsInfo, &imageAvailableSemaphores, &renderFinishedSemaphores, &inFlightFences, &imagesInFlight);
}

void test(int i)
{
    std::cout << "test " << i << std::endl;
}

void functionTest()
{

}

void EVulkan::mainLoop()
{
    // TODO: allocate vbo command pools here.
    std::vector<VkCommandPool> vertexUpdateCommandPools(FLAGS_num_threads);
    std::vector<VkCommandPool> sceneUpdateCommandPools(FLAGS_num_threads);
    for (auto &cp : vertexUpdateCommandPools)
    {
        EVkCommandPoolCreateInfo info = {};
        info.physicalDevice = physicalDevice;
        info.surface = surface;
        evkCreateCommandPool(device, &info, &cp);
    }
    // for (auto &cp : sceneUpdateCommandPools)
    // {
    //     EVkCommandPoolCreateInfo info = {};
    //     info.physicalDevice = physicalDevice;
    //     info.surface = surface;
    //     evkCreateCommandPool(device, &info, &cp);
    // }

    // int i = 0;
    std::chrono::steady_clock::time_point startTime, endTime;
    uint32_t imageIndex;

    EVkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.physicalDevice = physicalDevice;
    commandPoolInfo.surface = surface;
    commandPoolInfo.flags = 0;

    EVkCommandBuffersCreateInfo commandBuffersInfo = {};
    commandBuffersInfo.commandPool = commandPool;
    commandBuffersInfo.descriptorSets = descriptorSets;
    commandBuffersInfo.graphicsPipeline = graphicsPipeline;
    commandBuffersInfo.indexBuffer = indexBuffer;
    commandBuffersInfo.indices = indices;
    commandBuffersInfo.pipelineLayout = pipelineLayout;
    commandBuffersInfo.renderPass = renderPass;
    commandBuffersInfo.swapchainExtent = swapChainExtent;
    commandBuffersInfo.vertexBuffer = vertexBuffer;
    commandBuffersInfo.poolCreateInfo = commandPoolInfo;

    EVkDrawFrameInfo drawInfo = {};
    drawInfo.pInFlightFences = &inFlightFences;
    drawInfo.pImageAvailableSemaphores = &imageAvailableSemaphores;
    drawInfo.swapchain = swapChain;
    drawInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
    drawInfo.graphicsQueue = graphicsQueue;
    drawInfo.presentQueue = presentQueue;
    drawInfo.pFramebufferResized = &framebufferResized;
    drawInfo.swapchainExtent = swapChainExtent;
    drawInfo.pUniformBufferMemory = &uniformBuffersMemory;
    drawInfo.pVertices = &vertices;
    drawInfo.pGrid = &grid;
    drawInfo.physicalDevice = physicalDevice;
    drawInfo.commandPool = commandPool;
    drawInfo.vertexBuffer = vertexBuffer;
    // drawInfo.pCommandBuffersCreateInfo = &commandBuffersInfo;
    drawInfo.framebuffers = swapChainFramebuffers;
    drawInfo.commandPools = vertexUpdateCommandPools;

    int frameNum=0;
    bool timed=false;
    if (FLAGS_num_frames > 0) timed=true; 

    ThreadPool pool;
    pool.setThreadCount(FLAGS_num_threads);

    std::vector<VkCommandBuffer> commandBuffers(FLAGS_num_threads);

    std::vector<VkCommandBuffer> primaryCommandBuffers(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        std::cout << i << " here\n";
        commandBuffersInfo.framebuffer=swapChainFramebuffers[i];
        // Needs to be done once for each framebuffer.
        evkCreateCommandBuffers(device,
            &commandBuffersInfo,
            &primaryCommandBuffers[i],
            &commandBuffers,
            &vertexUpdateCommandPools,
            pool);
    }

    std::cout << "Finished up setup.\n";

    // int currentFrame=0;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        std::cout << currentFrame << &primaryCommandBuffers[currentFrame] << std::endl;
        bench.numVertices(vertices.size());
        bench.numThreads(FLAGS_num_threads);
        bench.numCubes(FLAGS_num_cubes);
        auto startTime = bench.start();
        evkDrawFrame(
            device,
            &drawInfo,
            &currentFrame, &imagesInFlight,
            &renderFinishedSemaphores,
            &primaryCommandBuffers[currentFrame],
            &imageIndex,
            bench,
            pool);
        bench.frameTime(startTime);
        bench.record();

        frameNum++;
        if (timed && frameNum >= FLAGS_num_frames) break;
        // currentFrame=(currentFrame+1)%MAX_FRAMES_IN_FLIGHT;
    }

    if (vkDeviceWaitIdle(device)!=VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for vkDeviceWaitIdle");
    }

        for (int i = 0; i < vertexUpdateCommandPools.size(); ++i)
    {
        vkFreeCommandBuffers(device, vertexUpdateCommandPools[i], 1, &commandBuffers[i]);
        // vkDestroyCommandPool(device, commandPools[i], nullptr);  // TODO: move this out?
    }

        for (auto &cp : vertexUpdateCommandPools)
    {
        vkDestroyCommandPool(device, cp, nullptr);
    }
}

void EVulkan::cleanup()
{
    EVkSwapchainCleanupInfo cleanupInfo = {};
    cleanupInfo.depthImage = depthImage;
    cleanupInfo.depthImageView = depthImageView;
    cleanupInfo.depthImageMemory = depthImageMemory;
    cleanupInfo.swapchainFramebuffers = swapChainFramebuffers;
    cleanupInfo.commandPool = commandPool;
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

    if (ENABLE_VALIDATION)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}