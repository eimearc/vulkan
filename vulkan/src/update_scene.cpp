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
    size = 1;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;
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
    size_t NUM_THREADS=1; //TODO: Update
    int i = 0;
    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = pCreateInfo->renderPass;
    inheritanceInfo.framebuffer = pCreateInfo->framebuffer;

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
    vkCmdBindIndexBuffer(commandBuffers[i], pCreateInfo->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->pipelineLayout, 0, 1, &(pCreateInfo->descriptorSets[i]), 0, nullptr);

    // Update here - update vertices.
    const size_t &numIndices = pCreateInfo->indices.size();
    const size_t myBaseIndex = index*(numIndices/NUM_THREADS);
    vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(pCreateInfo->indices.size()/NUM_THREADS), 1, myBaseIndex, 0, 0);

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer.");
    }
}


void evkUpdateScene(
    VkDevice device,
    const EVkSceneUpdateInfo *pUpdateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<thread> *pThreadPool,
    Bench &bench
)
{
    bench.start();
    evkUpdateVertexBuffer(device, pUpdateInfo->pVertexUpdateInfo);
    bench.updateVBOTime();
    evkCreateCommandBuffers(device, pUpdateInfo->pCommandBuffersCreateInfo, pPrimaryCommandBuffer, pThreadPool);
}

void updateVertexBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkQueue queue,
    VkSurfaceKHR surface,
    VkCommandPool *pCommandPool,
    VkCommandBuffer *pCommandBuffer,
    VkBuffer *stagingBuffer,
    VkDeviceMemory *stagingBufferMemory,
    VkBuffer vertexBuffer,
    std::vector<Vertex> &verts,
    const Grid &grid,
    size_t bufferSize,
    size_t bufferOffset,
    size_t vertsOffset,
    size_t numVerts
    )
{
    bufferSize = numVerts*sizeof(verts[0]);
    std::cout << "vertsOffset:" << vertsOffset << " bufferOffset:" << bufferOffset << " bufferSize:" << bufferSize << " numVerts:" << numVerts << std::endl;
    update(verts, grid, vertsOffset, numVerts);

    EVkCommandPoolCreateInfo info = {};
    info.physicalDevice = physicalDevice;
    info.surface = surface;
    evkCreateCommandPool(device, &info, pCommandPool);

    // Use a host visible buffer as a staging buffer.
    createBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.
    void *data;
    vkMapMemory(device, *stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, &verts[vertsOffset], bufferSize);
    vkUnmapMemory(device, *stagingBufferMemory);

    // Copy the vertex data from the staging buffer to the device-local buffer.
    const VkBuffer &dstBuffer = vertexBuffer;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *pCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &allocInfo, pCommandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(*pCommandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = bufferSize;
    copyRegion.dstOffset = bufferOffset;
    vkCmdCopyBuffer(*pCommandBuffer, *stagingBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(*pCommandBuffer);
}

void evkUpdateVertexBuffer(VkDevice device, const EVkVertexBufferUpdateInfo *pUpdateInfo)
{
    size_t NUM_THREADS=FLAGS_num_threads;
    const VkDeviceSize wholeBufferSize = sizeof((pUpdateInfo->pVertices)[0]) * pUpdateInfo->pVertices->size();
    std::cout << "Whole buffer size: " << wholeBufferSize << std::endl;
    const VkQueue queue = pUpdateInfo->graphicsQueue;
    std::vector<Vertex> &verts = pUpdateInfo->pVertices[0];
    const int num_verts = verts.size();
    int num_verts_each = num_verts/NUM_THREADS;
    size_t threadBufferSize = wholeBufferSize/NUM_THREADS;

    std::vector<std::thread> workers;
    std::vector<VkCommandPool> commandPools(NUM_THREADS);
    std::vector<VkCommandBuffer> commandBuffers(NUM_THREADS);
    std::vector<VkBuffer> buffers(NUM_THREADS);
    std::vector<VkDeviceMemory> bufferMemory(NUM_THREADS);

    auto f = [&](int i)
    {
        size_t bufferOffset = threadBufferSize*i;
        int vertsOffset = num_verts_each*i;
        bufferOffset=(num_verts_each*sizeof(verts[0]))*i;
        if (i==(FLAGS_num_threads-1))
        {
            num_verts_each = verts.size()-(i*num_verts_each);
            threadBufferSize = sizeof(verts[0])*num_verts_each;
        }
        updateVertexBuffer(
            device, pUpdateInfo->physicalDevice, pUpdateInfo->graphicsQueue, pUpdateInfo->surface,
            &commandPools[i], &commandBuffers[i], &buffers[i], &bufferMemory[i], pUpdateInfo->vertexBuffer,
            verts, pUpdateInfo->grid, threadBufferSize, bufferOffset, vertsOffset, num_verts_each);
    };

    auto startTime = std::chrono::high_resolution_clock::now();

     std::cout << NUM_THREADS << std::endl;
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        workers.push_back(std::thread(f,i));
    }
    for (std::thread &t : workers)
    {
        t.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    for (size_t i = 0; i<NUM_THREADS; ++i)
    {
        vkFreeCommandBuffers(device, commandPools[i], 1, &commandBuffers[i]);
        vkDestroyBuffer(device, buffers[i], nullptr);
        vkFreeMemory(device, bufferMemory[i], nullptr);
        vkDestroyCommandPool(device, commandPools[i], nullptr);
    }
}

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<thread> *pThreadPool
)
{
    thread::reset();
    size_t NUM_THREADS=1;  // TODO: Change.

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
    renderPassInfo.framebuffer = pCreateInfo->framebuffer;
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

    std::vector<thread> threadPool;
    EVkCommandPoolCreateInfo poolCreateInfo = pCreateInfo->poolCreateInfo;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (int j = 0; j < NUM_THREADS; ++j)
    {
        thread t(device, &poolCreateInfo, 1);
        t.createSecondaryCommandBuffers(pCreateInfo);
        threadPool.push_back(t);
    }

    // TODO: make the above happen in parallel.
    
    std::vector<VkCommandBuffer> tmp;
    for (const auto &thread : threadPool)
    {
        for (const auto &cb : thread.commandBuffers)
        {
            tmp.push_back(cb);
        }
    }

	vkCmdExecuteCommands(primaryCommandBuffer, tmp.size(), tmp.data());
    vkCmdEndRenderPass(primaryCommandBuffer);
    if (vkEndCommandBuffer(primaryCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not end primaryCommandBuffer.");   
    }

    *pThreadPool = threadPool;
    imageIndex = (imageIndex+1)%3;
}