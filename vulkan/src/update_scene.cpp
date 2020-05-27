#include "evulkan_core.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

void createSecondaryCommandBuffers(
    VkDevice device,
    const EVkCommandPoolCreateInfo *pCommandPoolCreateInfo,
    const VkCommandPool *pCommandPool,
    VkCommandBuffer *pCommandBuffer,
    size_t indexOffset,
    size_t numIndices,
    VkDescriptorSet *pDescriptorSet,
    const EVkCommandBuffersCreateInfo *pCreateInfo
)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *pCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, pCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers.");
    }

    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = pCreateInfo->renderPass;
    inheritanceInfo.framebuffer = pCreateInfo->framebuffer;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    if (vkBeginCommandBuffer(*pCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer.");
    }
    
    vkCmdBindPipeline(*pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->graphicsPipeline);

    VkBuffer vertexBuffers[] = {pCreateInfo->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(*pCommandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(*pCommandBuffer, pCreateInfo->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(*pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->pipelineLayout, 0, 1, &(pCreateInfo->descriptorSets[0]), 0, nullptr);

    vkCmdDrawIndexed(*pCommandBuffer, numIndices, 1, indexOffset, 0, 0);

    if (vkEndCommandBuffer(*pCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer.");
    }
}


void evkUpdateScene(
    VkDevice device,
    const EVkSceneUpdateInfo *pUpdateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    Bench &bench,
    ThreadPool &threadpool
)
{
    auto startTime = bench.start();
    evkUpdateVertexBuffer(device, pUpdateInfo->pVertexUpdateInfo, threadpool);
    bench.updateVBOTime(startTime);
    evkCreateCommandBuffers(device,
        pUpdateInfo->pCommandBuffersCreateInfo,
        pPrimaryCommandBuffer,
        pUpdateInfo->pCommandBuffers,pUpdateInfo->pCommandPools, threadpool);
}

void evkUpdateVertexBuffer(VkDevice device, const EVkVertexBufferUpdateInfo *pUpdateInfo, ThreadPool &threadpool)
{
    size_t NUM_THREADS=FLAGS_num_threads;
    const VkDeviceSize wholeBufferSize = sizeof((pUpdateInfo->pVertices)[0]) * pUpdateInfo->pVertices->size();
    const VkQueue queue = pUpdateInfo->graphicsQueue;
    std::vector<Vertex> &verts = pUpdateInfo->pVertices[0];
    const int num_verts = verts.size();
    int num_verts_each = num_verts/NUM_THREADS;
    size_t threadBufferSize = wholeBufferSize/NUM_THREADS;

    std::vector<std::thread> workers;
    auto &commandPools = pUpdateInfo->commandPools;
    std::vector<VkCommandBuffer> commandBuffers(NUM_THREADS);
    std::vector<VkBuffer> buffers(NUM_THREADS);
    std::vector<VkDeviceMemory> bufferMemory(NUM_THREADS);

    auto f = [&](int i)
    {
        int vertsOffset = num_verts_each*i;
        size_t bufferOffset=(num_verts_each*sizeof(verts[0]))*i;
        if (i==(FLAGS_num_threads-1))
        {
            num_verts_each = verts.size()-(i*num_verts_each);
        }
        size_t numVerts=num_verts_each;
        size_t bufferSize = numVerts*sizeof(verts[0]);
        auto &stagingBuffer = buffers[i];
        auto &stagingBufferMemory = bufferMemory[i];
        update(verts, *pUpdateInfo->pGrid, vertsOffset, numVerts);

        // Use a host visible buffer as a staging buffer.
        createBuffer(
            device,
            pUpdateInfo->physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &buffers[i], &bufferMemory[i]);

        // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
        // accessible memory.
        void *data;
        vkMapMemory(device, bufferMemory[i], 0, bufferSize, 0, &data);
        memcpy(data, &verts[vertsOffset], bufferSize);
        vkUnmapMemory(device, bufferMemory[i]);

        // Copy the vertex data from the staging buffer to the device-local buffer.
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPools[i];
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[i]);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        VkBufferCopy copyRegion = {};
        copyRegion.size = bufferSize;
        copyRegion.dstOffset = bufferOffset;
        vkCmdCopyBuffer(commandBuffers[i], buffers[i], pUpdateInfo->vertexBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffers[i]);
    };

    auto startTime = std::chrono::high_resolution_clock::now();
    // boost::asio::io_service service;
    // std::unique_ptr<boost::asio::io_service::work> work = std::make_unique<boost::asio::io_service::work>(boost::asio::io_service::work(service));
    // boost::thread_group pool;
    // for (int i = 0; i<FLAGS_num_threads; ++i)
    //     pool.create_thread([&service](){service.run();});
    // for (int i = 0; i<FLAGS_num_threads; ++i)
    // {
    //     service.post(std::bind(f,i));
    // }
    // work.reset();
    // pool.join_all();

    // ThreadPool pool;
    // pool.setThreadCount(FLAGS_num_threads);
    int i = 0;
    for (auto &t: threadpool.threads)
    {
        t->addJob(std::bind(f,i++));
    }
    threadpool.wait();

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
        // vkDestroyCommandPool(device, commandPools[i], nullptr); // TODO: Move these to outside and reuse.
    }
}

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<VkCommandBuffer> *pCommandBuffers,
    const std::vector<VkCommandPool> *pCommandPools,
    ThreadPool &threadpool
)
{
    size_t NUM_THREADS=FLAGS_num_threads;

    static int imageIndex = 0;

    // Create primary command buffer.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pCreateInfo->commandPool;
    allocInfo.commandBufferCount = 1;
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

    EVkCommandPoolCreateInfo poolCreateInfo = pCreateInfo->poolCreateInfo;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    std::vector<std::thread> workers;
    const std::vector<VkCommandPool> &commandPools=*pCommandPools;
    std::vector<VkCommandBuffer> &commandBuffers=*pCommandBuffers;
    const std::vector<uint32_t> &indices = pCreateInfo->indices;

    auto f =[&](int i)
    {
        size_t numIndices=indices.size()/NUM_THREADS;
        size_t indexOffset=numIndices*i;
        if (i==(FLAGS_num_threads-1))
        {
            numIndices = indices.size()-(i*numIndices);
        }
        createSecondaryCommandBuffers(device,
            &poolCreateInfo,&commandPools[i],&commandBuffers[i],indexOffset,numIndices,nullptr,pCreateInfo);
    };

    auto startTime = std::chrono::high_resolution_clock::now();
    // for (int i = 0; i < NUM_THREADS; ++i)
    // {
    //     workers.push_back(std::thread(f,i));
    // }
    // for (std::thread &t: workers)
    // {
    //     t.join();
    // }

    int i = 0;
    for (auto &t: threadpool.threads)
    {
        t->addJob(std::bind(f,i++));
    }
    threadpool.wait();

    auto endTime = std::chrono::high_resolution_clock::now();       
    float time = std::chrono::duration<float, std::chrono::milliseconds::period>(endTime - startTime).count();

	vkCmdExecuteCommands(primaryCommandBuffer, commandBuffers.size(), commandBuffers.data());
    vkCmdEndRenderPass(primaryCommandBuffer);
    if (vkEndCommandBuffer(primaryCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not end primaryCommandBuffer.");   
    }

    imageIndex = (imageIndex+1)%3;
}