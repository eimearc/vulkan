#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include "util.h"

#define ENABLE_VALIDATION true

struct EVkDeviceCreateInfo
{
    VkSurfaceKHR surface;
    std::vector<const char *> deviceExtensions;
    std::vector<const char *> validationLayers;
};

struct EVkSwapchainCreateInfo
{
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    GLFWwindow* window;
};

void evkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const EVkDeviceCreateInfo *pCreateInfo,
    VkDevice *pDevice,
    VkQueue *pGraphicsQueue,
    VkQueue *pPresentQueue);

QueueFamilyIndices getQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);

void evkCreateSwapchain(
    VkDevice device,
    const EVkSwapchainCreateInfo *pCreateInfo,
    VkSwapchainKHR *pSwapchain,
    std::vector<VkImage> *pSwapchainImages,
    VkFormat *pSwapchainImageFormat,
    VkExtent2D *pSwapchainExtent);

SwapChainSupportDetails querySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(
    GLFWwindow* window,
    const VkSurfaceCapabilitiesKHR& capabilities);
QueueFamilyIndices findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface);

struct EVkImageViewsCreateInfo
{
    std::vector<VkImage> images;
    VkFormat swapChainImageFormat;
};

void evkCreateImageViews
(
    VkDevice device,
    const EVkImageViewsCreateInfo *pCreateInfo,
    std::vector<VkImageView> *pSwapChainImageViews
);
VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

struct EVkRenderPassCreateInfo
{
    VkFormat swapChainImageFormat;
    VkPhysicalDevice physicalDevice;
};
void evkCreateRenderPass(
    VkDevice device,
    const EVkRenderPassCreateInfo *pCreateInfo,
    VkRenderPass *pRenderPass);
VkFormat findDepthFormat(const EVkRenderPassCreateInfo *pCreateInfo);
VkFormat findSupportedFormat(
    const EVkRenderPassCreateInfo *pCreateInfo,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features);

struct EVkDescriptorSetLayoutCreateInfo
{

};
void evkCreateDescriptorSetLayout(
    VkDevice device,
    const EVkDescriptorSetLayoutCreateInfo *pCreateInfo,
    VkDescriptorSetLayout *pDescriptorSetLayout
);

struct EVkGraphicsPipelineCreateInfo
{
    std::string vertShaderFile;
    std::string fragShaderFile;
    VkExtent2D swapchainExtent;
    VkDescriptorSetLayout *pDescriptorSetLayout;
    VkRenderPass renderPass;
};
void evkCreateGraphicsPipeline(
    VkDevice device,
    const EVkGraphicsPipelineCreateInfo *pCreateInfo,
    VkPipelineLayout *pPipelineLayout,
    VkPipeline *pPipeline);
void createShaderModule(VkDevice device, const std::vector<char>& code, VkShaderModule *pShaderModule);

struct EVkDepthResourcesCreateInfo
{
    VkFormat swapchainImageFormat;
    VkPhysicalDevice physicalDevice;
    VkExtent2D swapchainExtent;
};
void evkCreateDepthResources(
    VkDevice device,
    const EVkDepthResourcesCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkImageView *pImageView,
    VkDeviceMemory *pImageMemory
);

struct EVkImageCreateInfo
{
    VkPhysicalDevice physicalDevice;
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags properties;
};
void evkCreateImage(
    VkDevice device,
    const EVkImageCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory);

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

struct EVkFramebuffersCreateInfo
{
    std::vector<VkImageView> swapchainImageViews;
    VkExtent2D swapchainExtent;
    VkImageView depthImageView;
    VkRenderPass renderPass;
};
void evkCreateFramebuffers
(
    VkDevice device,
    const EVkFramebuffersCreateInfo *pCreateInfo,
    std::vector<VkFramebuffer> *pFramebuffers
);

struct EVkCommandPoolCreateInfo
{
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
};
void evkCreateCommandPool(
    VkDevice device,
    const EVkCommandPoolCreateInfo *pCreateInfo,
    VkCommandPool *pCommandPool);

struct EVkVertexBufferCreateInfo
{
    std::vector<Vertex> vertices;
    VkPhysicalDevice physicalDevice;
    VkQueue queue;
    VkCommandPool commandPool;
};
void evkCreateVertexBuffer(
    VkDevice device,
    const EVkVertexBufferCreateInfo *pCreateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
);
struct EVkIndexBufferCreateInfo
{
    std::vector<uint16_t> indices;
    VkPhysicalDevice physicalDevice;
    VkQueue queue;
    VkCommandPool commandPool;
};
void evkCreateIndexBuffer(
    VkDevice device,
    const EVkIndexBufferCreateInfo *pCreateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
);
void createBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory);
void copyBuffer(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size);
void beginSingleTimeCommands(
    VkDevice device,
    VkCommandPool commandPool,
    VkCommandBuffer *pCommandBuffer);
void endSingleTimeCommands(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkCommandBuffer commandBuffer);

struct EVkUniformBufferCreateInfo
{
    VkPhysicalDevice physicalDevice;
    std::vector<VkImage> swapchainImages;  
};
void evkCreateUniformBuffers(
    VkDevice device,
    const EVkUniformBufferCreateInfo *pCreateInfo,
    std::vector<VkBuffer> *pBuffer,
    std::vector<VkDeviceMemory> *pBufferMemory
);

struct EVkDescriptorPoolCreateInfo
{
    std::vector<VkImage> swapchainImages;
};
void evkCreateDescriptorPool(
    VkDevice device,
    const EVkDescriptorPoolCreateInfo *pCreateInfo,
    VkDescriptorPool *pDescriptorPool
);

struct EVkDescriptorSetCreateInfo
{
    std::vector<VkImage> swapchainImages;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkBuffer> uniformBuffers;
};
void evkCreateDescriptorSets(
    VkDevice device,
    const EVkDescriptorSetCreateInfo *pCreateInfo,
    std::vector<VkDescriptorSet> *pDescriptorSets
);

struct EVkCommandBuffersCreateInfo
{
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    VkExtent2D swapchainExtent;
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    std::vector<uint16_t> indices;
};
void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    std::vector<VkCommandBuffer> *pCommandBuffers
);