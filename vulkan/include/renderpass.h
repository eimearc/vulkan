#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct Renderpass
{
    Renderpass()=default;
    Renderpass(VkDevice _device) : device(_device)
    {
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
    }
    ~Renderpass()noexcept=default;

    void createVertexBuffer();
    void updateVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer &buffer,
        VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void updateUniformBuffer(uint32_t currentImage);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();

    void cleanup();

    VkDevice device;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkAttachmentDescription colorAttachment;
    VkAttachmentDescription depthAttachment;
};
