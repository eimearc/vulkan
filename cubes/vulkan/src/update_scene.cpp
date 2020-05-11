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
    evkUpdateVertexBuffer(device, pUpdateInfo->pVertexUpdateInfo);
    evkCreateCommandBuffers(device, pUpdateInfo->pCommandBuffersCreateInfo, pPrimaryCommandBuffer, pThreadPool);
}

void updateVertexBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkQueue queue,
    VkCommandPool commandPool,
    VkBuffer vertexBuffer,
    const std::vector<Vertex> &verts,
    size_t bufferSize,
    size_t bufferOffset,
    size_t vertsOffset
    )
{
    // Use a host visible buffer as a staging buffer.
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, &verts[vertsOffset], bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Copy the vertex data from the staging buffer to the device-local buffer.
    const VkBuffer &dstBuffer = vertexBuffer;
    VkCommandBuffer commandBuffer;
    beginSingleTimeCommands(device, commandPool, &commandBuffer);
    VkBufferCopy copyRegion = {};
    copyRegion.size = bufferSize;
    copyRegion.dstOffset = bufferOffset;
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommands(device, queue, commandPool, commandBuffer);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void evkUpdateVertexBuffer(VkDevice device, const EVkVertexBufferUpdateInfo *pUpdateInfo)
{
    update(*pUpdateInfo->pVertices, pUpdateInfo->grid, 0, pUpdateInfo->pVertices->size()-1);

    // This can all be done across multple threads.
    const VkDeviceSize wholeBufferSize = sizeof((pUpdateInfo->pVertices)[0]) * pUpdateInfo->pVertices->size();
    const std::vector<Vertex> &verts = pUpdateInfo->pVertices[0];
    constexpr int num_threads = 4;
    const int num_verts = verts.size();
    const int num_verts_each = num_verts/num_threads;
    const size_t threadBufferSize = wholeBufferSize/num_threads;

    for (int i = 0; i < num_threads; ++i)
    {
        size_t bufferOffset = threadBufferSize*i;
        int vertsOffset = num_verts_each*i;

        updateVertexBuffer(
            device, pUpdateInfo->physicalDevice, pUpdateInfo->graphicsQueue,
            pUpdateInfo->commandPool, pUpdateInfo->vertexBuffer,
            verts, threadBufferSize, bufferOffset, vertsOffset);
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

    // TODO: make the above happen in parallel.

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