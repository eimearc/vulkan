#include "evulkan_core.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// void EVulkan::transitionImageLayout(VkImage image, VkFormat format,
//     VkImageLayout oldLayout, VkImageLayout newLayout)
// {
//     VkCommandBuffer commandBuffer;
//     beginSingleTimeCommands(
//         device,
//         commandPool,
//         &commandBuffer
//     );

//     VkImageMemoryBarrier barrier = {};
//     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//     barrier.oldLayout = oldLayout;
//     barrier.newLayout = newLayout;
//     barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//     barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//     barrier.image = image;
//     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     barrier.subresourceRange.baseMipLevel = 0;
//     barrier.subresourceRange.levelCount = 1;
//     barrier.subresourceRange.baseArrayLayer = 0;
//     barrier.subresourceRange.layerCount = 1;
//     barrier.srcAccessMask = 0;
//     barrier.dstAccessMask = 0;

//     VkPipelineStageFlags sourceStage;
//     VkPipelineStageFlags destinationStage;

//     if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
//     {
//         barrier.srcAccessMask = 0;
//         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//         destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//     }
//     else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//     {
//         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//         sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//         destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//     }
//     else
//     {
//         throw std::runtime_error("unsupported layout transition.");
//     }
    
//     // Insert an execution dependency on sets of commands.
//     vkCmdPipelineBarrier(commandBuffer,
//         sourceStage, destinationStage,
//         0,
//         0, nullptr,
//         0, nullptr,
//         1, &barrier);

//     endSingleTimeCommands(
//         device,
//         graphicsQueue,
//         commandPool,
//         commandBuffer);
// }

// void EVulkan::copyBufferToImage(VkBuffer buffer, VkImage image,
//     uint32_t width, uint32_t height)
// {
//     VkCommandBuffer commandBuffer;
//     beginSingleTimeCommands(
//         device,
//         commandPool,
//         &commandBuffer
//     );

//     VkBufferImageCopy region = {};
//     region.bufferOffset = 0;
//     region.bufferRowLength = 0;
//     region.bufferImageHeight = 0;
//     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     region.imageSubresource.mipLevel = 0;
//     region.imageSubresource.baseArrayLayer = 0;
//     region.imageSubresource.layerCount = 1;
//     region.imageOffset = {0,0,0};
//     region.imageExtent =
//     {
//         width, height, 1
//     };

//     vkCmdCopyBufferToImage(commandBuffer,
//         buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

//     endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
// }

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    std::vector<VkCommandBuffer> *pCommandBuffers
)
{
    const size_t &size = pCreateInfo->swapchainFramebuffers.size();
    pCommandBuffers->resize(size);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pCreateInfo->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = size;

    if (vkAllocateCommandBuffers(device, &allocInfo, pCommandBuffers->data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers.");
    }

    for (size_t i = 0; i < size; i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer((*pCommandBuffers)[i], &beginInfo) != VK_SUCCESS)
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
        vkCmdBeginRenderPass((*pCommandBuffers)[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline((*pCommandBuffers)[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->graphicsPipeline);
        VkBuffer vertexBuffers[] = {pCreateInfo->vertexBuffer};
        VkDeviceSize offsets[] = {0};

        // Bind the vertex and index buffers during rendering operations.
        vkCmdBindVertexBuffers((*pCommandBuffers)[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer((*pCommandBuffers)[i], pCreateInfo->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        // Bind the descriptor set for each swap chain image.
        vkCmdBindDescriptorSets((*pCommandBuffers)[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pCreateInfo->pipelineLayout, 0, 1, &(pCreateInfo->descriptorSets[i]), 0, nullptr);
        
        vkCmdDrawIndexed((*pCommandBuffers)[i], static_cast<uint32_t>(pCreateInfo->indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass((*pCommandBuffers)[i]);
        if (vkEndCommandBuffer((*pCommandBuffers)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer.");
        }
    }
}

// bool EVulkan::hasStencilComponent(VkFormat format)
// {
//     return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
// }

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