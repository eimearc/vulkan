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

    int i = 0;
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
    drawInfo.pCommandBuffersCreateInfo = &commandBuffersInfo;
    drawInfo.framebuffers = swapChainFramebuffers;
    drawInfo.commandPools = vertexUpdateCommandPools;

    int frameNum=0;
    bool timed=false;
    if (FLAGS_num_frames > 0) timed=true; 

    ThreadPool pool;
    pool.setThreadCount(FLAGS_num_threads);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // boost::asio::io_service ioService;
        // boost::asio::io_service::work work(ioService);
        
        // auto t = [](int i){std::cout << "Hello from task " << i << ".\n";};

        // boost::thread_group threadpool;
        // threadpool.create_thread(boost::bind(&boost::asio::io_service::run, &ioService));
        // ioService.post(boost::bind(test,0));
        // ioService.run();
        // threadpool.join_all();
        // ioService.stop();
        // std::cout << "After join_all\n";
        // work=boost::none;

        // ioService.reset();
        // std::cout << "About to run\n";
        // ioService.run();
        // std::cout << "About to work\n";
        // boost::asio::io_service::work work(ioService);
        // ioService.post(boost::bind<void>(t,1));
        // std::cout << "\n\nWaiting for second task\n";
        // ioService.stop();
        // threadpool.join_all();
        // std::cout << "After second join_all\n";

        // boost::asio::io_service service;
        // std::unique_ptr<boost::asio::io_service::work> work = std::make_unique<boost::asio::io_service::work>(boost::asio::io_service::work(service));

        // // service.run();
        // boost::thread_group pool;
        // for (int i = 0; i< 4; ++i)
        //     pool.create_thread([&service](){service.run();});

        // for (int i = 0; i<4; ++i)
        //     service.post([i](){std::cout << "Post " << i << " " << std::this_thread::get_id() << std::endl;});

        // work.reset();
        // pool.join_all();

        // std::cout <<"\n\n\n\n";

        bench.numVertices(vertices.size());
        bench.numThreads(FLAGS_num_threads);
        bench.numCubes(FLAGS_num_cubes);
        auto startTime = bench.start();
        evkDrawFrame(
            device,
            &drawInfo,
            &currentFrame, &imagesInFlight,
            &renderFinishedSemaphores,
            &primaryCommandBuffer,
            &imageIndex,
            bench,
            pool);
        bench.frameTime(startTime);
        bench.record();

        frameNum++;
        if (timed && frameNum >= FLAGS_num_frames) break;
    }

    if (vkDeviceWaitIdle(device)!=VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for vkDeviceWaitIdle");
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