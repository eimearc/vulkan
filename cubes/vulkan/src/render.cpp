#include "evulkan_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

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
    // Do one per cube.
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
        std::cout << "\tNumber of indices to draw: " << pCreateInfo->indices.size() << " my index: " << index << std::endl;
        std::cout << "Num threads: " << NUM_THREADS << " num indices each: " << numIndices/NUM_THREADS << " my base index: " << myBaseIndex << std::endl;
        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(pCreateInfo->indices.size()/2), 1, myBaseIndex, 0, 0);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer.");
        }
        std::cout << "\tCommand buffer:" << commandBuffers[i] << std::endl;
    }
}

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    std::vector<VkCommandBuffer> *pCommandBuffers,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<thread> *pThreadPool
)
{
    thread::reset();

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
}

void evkCreateSyncObjects(
    VkDevice device,
    const EVkSyncObjectsCreateInfo *pCreateInfo,
    std::vector<VkSemaphore> *pImageAvailableSemaphores,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores,
    std::vector<VkFence> *pFencesInFlight,
    std::vector<VkFence> *pImagesInFlight
)
{
    pImageAvailableSemaphores->resize(pCreateInfo->maxFramesInFlight);
    pRenderFinishedSemaphores->resize(pCreateInfo->maxFramesInFlight);
    pFencesInFlight->resize(pCreateInfo->maxFramesInFlight);
    pImagesInFlight->resize(pCreateInfo->swapchainSize, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < pCreateInfo->maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &(*pImageAvailableSemaphores)[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &(*pRenderFinishedSemaphores)[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &(*pFencesInFlight)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores for a frame.");
        }
    }
}