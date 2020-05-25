#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include "util.h"
#include "flags.h"
#include "bench.h"

#define ENABLE_VALIDATION true

// DECLARE_int32(num_threads);

// size_t NUM_THREADS;

struct EVkCreateWindow
{
    bool resizeable;
    std::string title;
    int width;
    int height;
};
void evkCreateWindow(
    const EVkCreateWindow params,
    GLFWwindow *&window
);

struct EVkCreateInstance
{
    const char* appTitle;
    std::vector<const char*> extensions;
    std::vector<const char*> validationLayers;
};
void evkCreateInstance(
    const EVkCreateInstance *pCreateInfo,
    VkInstance *instance
);

void evkSetupDebugMessenger(
    VkInstance instance,
    VkDebugUtilsMessengerEXT *pDebugMessenger
);

struct EVkSurfaceCreate
{
    GLFWwindow *window;
};
void evkCreateSurface(
    VkInstance instance,
    const EVkSurfaceCreate *pCreateInfo,
    VkSurfaceKHR *surface
);

struct EVkPickPhysicalDevice
{
    VkSurfaceKHR surface;
    std::vector<const char *> deviceExtensions;
};
void evkPickPhysicalDevice(
    VkInstance instance,
    const EVkPickPhysicalDevice *pPickInfo,
    VkPhysicalDevice *physicalDevice
);
bool isDeviceSuitable(
    VkPhysicalDevice device,
    const EVkPickPhysicalDevice *pPickInfo
);
bool checkDeviceExtensionSupport(
    VkPhysicalDevice device,
    const EVkPickPhysicalDevice *pPickInfo
);

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
);

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

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
    uint32_t numImages;
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
    VkCommandPoolCreateFlags flags;
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
    std::vector<uint32_t> indices;
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
    VkFramebuffer framebuffer;
    VkCommandPool commandPool;
    EVkCommandPoolCreateInfo poolCreateInfo;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    VkExtent2D swapchainExtent;
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    std::vector<uint32_t> indices;
};

struct thread
{
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    size_t size;
    VkDevice device;
    size_t index;
    static size_t i;

    static void reset(){i=0;}
    void cleanup();
    thread(VkDevice device, const EVkCommandPoolCreateInfo *pCreateInfo, size_t size);
    void createSecondaryCommandBuffers(const EVkCommandBuffersCreateInfo *pCreateInfo);
};

void evkCreateCommandBuffers(
    VkDevice device,
    const EVkCommandBuffersCreateInfo *pCreateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<thread> *pThreadPool
);

struct EVkSyncObjectsCreateInfo
{
    size_t maxFramesInFlight;
    size_t swapchainSize;
};
void evkCreateSyncObjects(
    VkDevice device,
    const EVkSyncObjectsCreateInfo *pCreateInfo,
    std::vector<VkSemaphore> *pImageAvailableSemaphores,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores,
    std::vector<VkFence> *pFencesInFlight,
    std::vector<VkFence> *pImagesInFlight
);

struct EVkDrawFrameInfo
{
    VkPhysicalDevice physicalDevice;
    std::vector<VkFence> *pInFlightFences;
    std::vector<VkSemaphore> *pImageAvailableSemaphores;
    size_t maxFramesInFlight;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    VkExtent2D swapchainExtent;
    std::vector<VkFramebuffer> framebuffers;
    bool *pFramebufferResized;
    std::vector<VkDeviceMemory> *pUniformBufferMemory;
    std::vector<Vertex> *pVertices;
    Grid grid;
    VkCommandPool commandPool;
    VkBuffer vertexBuffer;
    EVkCommandBuffersCreateInfo *pCommandBuffersCreateInfo;
};
void evkDrawFrame(
    VkDevice device,
    const EVkDrawFrameInfo *pDrawInfo,
    size_t *pCurrentFrame,
    std::vector<VkFence> *pImagesInFlight,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores,
    VkCommandBuffer *pPrimaryCommandBuffer,
    uint32_t *pImageIndex,
    Bench &bench
);

struct EVkSwapchainRecreateInfo
{
    GLFWwindow *pWindow;
    VkSwapchainKHR *pSwapchain;
    std::vector<VkImage> *pSwapchainImages;
    std::vector<VkImageView> *pSwapchainImageViews;
    VkFormat *pSwapchainImageFormats;
    VkExtent2D *pSwapchainExtent;
    VkRenderPass *pRenderPass;
    VkPipelineLayout *pPipelineLayout;
    VkPipeline *pPipeline;
    VkImage *pDepthImage;
    VkImageView *pDepthImageView;
    VkDeviceMemory *pDepthImageMemory;
    std::vector<VkFramebuffer> *pSwapchainFramebuffers;
    std::vector<VkBuffer> *pUniformBuffers;
    std::vector<VkDeviceMemory> *pUniformBuffersMemory;
    VkDescriptorPool *pDescriptorPool;
    std::vector<VkDescriptorSet> *pDescriptorSets;
    std::vector<VkCommandBuffer> *pCommandBuffers;
    VkCommandBuffer *pPrimaryCommandBuffer;

    EVkSwapchainCreateInfo swapchainCreateInfo;
    EVkImageViewsCreateInfo imageViewsCreateInfo;
    EVkRenderPassCreateInfo renderPassCreateInfo;
    EVkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
    EVkDepthResourcesCreateInfo depthResourcesCreateInfo;
    EVkFramebuffersCreateInfo framebuffersCreateInfo;
    EVkUniformBufferCreateInfo uniformBuffersCreateInfo;
    EVkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
    EVkDescriptorSetCreateInfo EVkDescriptorSetCreateInfo;
    EVkCommandBuffersCreateInfo commandBuffersCreateInfo;
};
void evkRecreateSwapchain(
    VkDevice device,
    const EVkSwapchainRecreateInfo *pCreateInfo
);

struct EVkSwapchainCleanupInfo
{
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> *pCommandBuffers;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    std::vector<VkImageView> swapchainImageViews;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
};
void evkCleanupSwapchain(VkDevice device, const EVkSwapchainCleanupInfo *pCleanupInfo);

struct EVkUniformBufferUpdateInfo
{
    uint32_t currentImage;
    VkExtent2D swapchainExtent;
    std::vector<VkDeviceMemory> *pUniformBufferMemory;
};
void evkUpdateUniformBuffer(VkDevice device, const EVkUniformBufferUpdateInfo *pUpdateInfo);

struct EVkVertexBufferUpdateInfo
{
   std::vector<Vertex> *pVertices;
   Grid grid; 
   VkPhysicalDevice physicalDevice;
   VkCommandPool commandPool;
   VkQueue graphicsQueue;
   VkBuffer vertexBuffer;
   VkSurfaceKHR surface;
};
void evkUpdateVertexBuffer(VkDevice device, const EVkVertexBufferUpdateInfo *pUpdateInfo);

struct EVkSceneUpdateInfo
{
   const EVkVertexBufferUpdateInfo *pVertexUpdateInfo;
   const EVkCommandBuffersCreateInfo *pCommandBuffersCreateInfo;
};
void evkUpdateScene(
    VkDevice device,
    const EVkSceneUpdateInfo *pUpdateInfo,
    VkCommandBuffer *pPrimaryCommandBuffer,
    std::vector<thread> *pThreadPool,
    Bench &bench
);