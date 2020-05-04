#include "evulkan_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

struct thread
{
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    size_t size;
    VkDevice device;

    thread(VkDevice device, VkCommandPool commandPool, size_t size);
    void createSecondaryCommandBuffers(const EVkCommandBuffersCreateInfo *pCreateInfo);
};

thread::thread(VkDevice _device, VkCommandPool _commandPool, size_t _size)
{
    device = _device;
    commandPool = _commandPool;
    size = _size;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = size;
    commandBuffers.resize(size);

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers.");
    }
}

void thread::createSecondaryCommandBuffers(const EVkCommandBuffersCreateInfo *pCreateInfo)
{
    for (size_t i = 0; i < size; i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer.");
        }
        
        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pCreateInfo->renderPass;
        renderPassInfo.framebuffer = pCreateInfo->swapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = pCreateInfo->swapchainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->graphicsPipeline);
        VkBuffer vertexBuffers[] = {pCreateInfo->vertexBuffer};
        VkDeviceSize offsets[] = {0};

        // Bind the vertex and index buffers during rendering operations.
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[i], pCreateInfo->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        // Bind the descriptor set for each swap chain image.
        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->pipelineLayout, 0, 1, &(pCreateInfo->descriptorSets[i]), 0, nullptr);
        
        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(pCreateInfo->indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer.");
        }
    }
}

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    std::vector<VkCommandBuffer> *pCommandBuffers
)
{
    const size_t &size = pCreateInfo->swapchainFramebuffers.size();
    std::vector<thread> threadPool;

    for (int j = 0; j < NUM_THREADS; ++j)
    {
        thread t(device, pCreateInfo->commandPools[j], size);
        t.createSecondaryCommandBuffers(pCreateInfo);
        threadPool.push_back(t);
    }

    for (const auto &thread : threadPool)
    {
        for (const auto &cb : thread.commandBuffers)
        pCommandBuffers->push_back(cb);
    }
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