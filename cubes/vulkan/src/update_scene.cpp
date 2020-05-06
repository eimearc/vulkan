#include "evulkan_core.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

size_t thread::i = 0;

thread::thread(VkDevice _device, const EVkCommandPoolCreateInfo *pCreateInfo, size_t _size)
{
    index = i++;
    device = _device;
    evkCreateCommandPool(device, pCreateInfo, &commandPool);
    size = _size/NUM_THREADS;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = size;
    commandBuffers.resize(size);

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers.");
    }
}

void thread::cleanup()
{
    vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
    vkDestroyCommandPool(device, commandPool, nullptr);
}

void thread::createSecondaryCommandBuffers(const EVkCommandBuffersCreateInfo *pCreateInfo)
{
    for (size_t i = 0; i < size; i++)
    {
        VkCommandBufferInheritanceInfo inheritanceInfo = {};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.renderPass = pCreateInfo->renderPass;
        inheritanceInfo.framebuffer = pCreateInfo->swapchainFramebuffers[i];

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = &inheritanceInfo;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer.");
        }
        
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->graphicsPipeline);

        VkBuffer vertexBuffers[] = {pCreateInfo->vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[i], pCreateInfo->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->pipelineLayout, 0, 1, &(pCreateInfo->descriptorSets[i]), 0, nullptr);

        // Update here - update vertices.
        const size_t &numIndices = pCreateInfo->indices.size();
        const size_t myBaseIndex = index*(numIndices/NUM_THREADS);
        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(pCreateInfo->indices.size()/2), 1, myBaseIndex, 0, 0);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer.");
        }
    }
}


void evkUpdateScene(
    VkDevice device,
    const EVkSceneUpdateInfo *pUpdateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<thread> *pThreadPool
)
{
    update(*pUpdateInfo->pVertexUpdateInfo->pVertices, pUpdateInfo->pVertexUpdateInfo->grid);
    evkUpdateVertexBuffer(device, pUpdateInfo->pVertexUpdateInfo);
    evkCreateCommandBuffers(device, pUpdateInfo->pCommandBuffersCreateInfo, pPrimaryCommandBuffer, pThreadPool);
}

void evkUpdateVertexBuffer(VkDevice device, const EVkVertexBufferUpdateInfo *pUpdateInfo)
{
    VkDeviceSize bufferSize = sizeof((pUpdateInfo->pVertices)[0]) * pUpdateInfo->pVertices->size();

    auto startTime = std::chrono::high_resolution_clock::now();

    // Use a host visible buffer as a temporary buffer.
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        device,
        pUpdateInfo->physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.

    const std::vector<Vertex> &verts = pUpdateInfo->pVertices[0];

    VkFence vertexCopyFence;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &fenceInfo, nullptr, &vertexCopyFence);

    int num_threads = 4;

    std::vector<VkFence> fences(num_threads);

    int num_verts = verts.size();
    int num_verts_each = num_verts/num_threads;
    int offset = bufferSize/num_threads;
    int size = bufferSize/num_threads;

    // std::thread t1{[&]
    // {
    //     for (size_t i = 0; i < num_threads; ++i)
    //     {
    //         std::lock_guard<std::mutex> l(m);
    //         void *data;
    //         vkMapMemory(device, stagingBufferMemory, i*offset, size, 0, &data);
    //         memcpy(data, &verts[i*num_verts_each], (size_t) size);
    //         vkUnmapMemory(device, stagingBufferMemory);
    //     }
    // }};

    std::mutex m;
    std::condition_variable cond;

    auto f = [&](int i){
        std::lock_guard<std::mutex> lk(m);
        std::cout << "Hi " << i << std::endl;
        void *data;
        vkMapMemory(device, stagingBufferMemory, i*offset, size, 0, &data);
        memcpy(data, &verts[i*num_verts_each], (size_t) size);
        vkUnmapMemory(device, stagingBufferMemory);
        cond.notify_one();
    };

    std::vector<std::thread> workers;
    for (int i = 0; i < num_threads; ++i)
    {
        workers.push_back(std::thread{f,i});
    }
    for (auto &t : workers)
    {
        t.join();
    }

    // for (size_t i = 0; i < num_threads; ++i)
    // {
    //     std::lock_guard<std::mutex> l(m);
    //     void *data;
    //     vkMapMemory(device, stagingBufferMemory, i*offset, size, 0, &data);
    //     memcpy(data, &verts[i*num_verts_each], (size_t) size);
    //     vkUnmapMemory(device, stagingBufferMemory);
    // }

    vkDestroyFence(device, vertexCopyFence, nullptr);
    for (size_t i=0; i < num_threads; ++i)
    {
        vkDestroyFence(device, fences[i], nullptr);
    }

    // Copy the vertex data from the staging buffer to the device-local buffer.
    copyBuffer(
        device,
        pUpdateInfo->commandPool,
        pUpdateInfo->graphicsQueue,
        stagingBuffer, pUpdateInfo->vertexBuffer, bufferSize);

    auto endTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();
    std::cout << "Copy took " << time << std::endl;

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<thread> *pThreadPool
)
{
    thread::reset();

    static int imageIndex = 0;

    // Create primary command buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pCreateInfo->commandPool;
    allocInfo.commandBufferCount = NUM_THREADS;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkAllocateCommandBuffers(device, &allocInfo, pPrimaryCommandBuffer);

    const VkCommandBuffer &primaryCommandBuffer = *pPrimaryCommandBuffer;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pCreateInfo->renderPass;
    renderPassInfo.framebuffer = pCreateInfo->swapchainFramebuffers[0]; // TODO: This isn't right.
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = pCreateInfo->swapchainExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(
        primaryCommandBuffer,
        &beginInfo);

    vkCmdBeginRenderPass(
        primaryCommandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    const size_t &size = pCreateInfo->swapchainFramebuffers.size(); // 3 images in swapchain.
    std::vector<thread> threadPool;
    EVkCommandPoolCreateInfo poolCreateInfo = pCreateInfo->poolCreateInfo;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (int j = 0; j < NUM_THREADS; ++j)
    {
        thread t(device, &poolCreateInfo, size);
        t.createSecondaryCommandBuffers(pCreateInfo);
        // Goes and updates the vertex buffers in own thread.
        threadPool.push_back(t);
    }

    std::cout << "Created secondary command buffers\n";
    std::vector<VkCommandBuffer> tmp;
    for (const auto &thread : threadPool)
    {
        std::cout << "\tPushing back " << thread.index << std::endl;
        for (const auto &cb : thread.commandBuffers)
        {
            std::cout << "\t\tCommand buffer\n"; 
            tmp.push_back(cb);
        }
    }
    std::cout << "Pushed to pCommandBuffers\n";

	vkCmdExecuteCommands(primaryCommandBuffer, tmp.size(), tmp.data());
    vkCmdEndRenderPass(primaryCommandBuffer);
    if (vkEndCommandBuffer(primaryCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not end primaryCommandBuffer.");   
    }

    std::cout << "Created command buffers\n";

    *pThreadPool = threadPool;
    imageIndex = (imageIndex+1)%3;
}