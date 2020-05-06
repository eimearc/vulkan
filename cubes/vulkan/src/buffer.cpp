#include "evulkan.h"

#include <glm/gtx/string_cast.hpp>

void EVulkan::setupVertices()
{
    int i=0;
    const size_t numVerts = 8;
    Vertex vertex = {{}, {1,0,0}};
    for (auto cube : grid.cubes)
    {
        std::vector<glm::vec3> verts = cube.vertices;
        std::vector<uint16_t> ind = cube.indices;
        for(size_t j = 0; j<verts.size(); ++j)
        {
            vertex.pos=verts[j];
            vertex.color=cube.color;
            vertices.push_back(vertex);
        }
        for(size_t j = 0; j<ind.size(); ++j)
        {
            indices.push_back(ind[j]+i*numVerts);
        }
        ++i;
    }
    std::cout << "Num verts: " << vertices.size() << std::endl;
    std::cout << "Num indices: " << indices.size() << std::endl;
}

void EVulkan::createGrid()
{
    uint16_t num = numCubes;
    float gridSize = 2.0f;
    float cubeSize = (gridSize/num)*0.5;
    grid = Grid(gridSize, cubeSize, num);
    setupVertices();
}

void evkCreateIndexBuffer(
    VkDevice device,
    const EVkIndexBufferCreateInfo *pCreateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory
)
{
    VkDeviceSize bufferSize = sizeof(pCreateInfo->indices[0]) * pCreateInfo->indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        device,
        pCreateInfo->physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, pCreateInfo->indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        device,
        pCreateInfo->physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        pBuffer, pBufferMemory);

    copyBuffer(device, pCreateInfo->commandPool, pCreateInfo->queue, stagingBuffer, *pBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void evkCreateUniformBuffers(
    VkDevice device,
    const EVkUniformBufferCreateInfo *pCreateInfo,
    std::vector<VkBuffer> *pBuffer,
    std::vector<VkDeviceMemory> *pBufferMemory
)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    const size_t &size = pCreateInfo->swapchainImages.size();
    pBuffer->resize(size);
    pBufferMemory->resize(size);

    for (size_t i = 0; i < size; i++)
    {
        createBuffer(
            device,
            pCreateInfo->physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &(*pBuffer)[i], &(*pBufferMemory)[i]);
    }
}

void evkCreateVertexBuffer(
    VkDevice device,
    const EVkVertexBufferCreateInfo *pCreateInfo,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(pCreateInfo->vertices[0]) * pCreateInfo->vertices.size();

    // Use a host visible buffer as a temporary buffer.
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        device,
        pCreateInfo->physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        &stagingBufferMemory);

    // Copy vertex data to the staging buffer by mapping the buffer memory into CPU
    // accessible memory.
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, pCreateInfo->vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Use a device-local buffer as the actual vertex buffer.
    createBuffer(
        device,
        pCreateInfo->physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        pBuffer,
        pBufferMemory);

    // Copy the vertex data from the staging buffer to the device-local buffer.
    copyBuffer(device, pCreateInfo->commandPool, pCreateInfo->queue, stagingBuffer, *pBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void evkUpdateUniformBuffer(VkDevice device, const EVkUniformBufferUpdateInfo *pUpdateInfo)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.model=glm::mat4(1.0f);
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), pUpdateInfo->swapchainExtent.width / (float) pUpdateInfo->swapchainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    std::vector<VkDeviceMemory> &uniformBufferMemory = *pUpdateInfo->pUniformBufferMemory;
    vkMapMemory(device, uniformBufferMemory[pUpdateInfo->currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBufferMemory[pUpdateInfo->currentImage]);
}

void createBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer *pBuffer,
    VkDeviceMemory *pBufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, pBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer.");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *pBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, pBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory");
    }

    vkBindBufferMemory(device, *pBuffer, *pBufferMemory, 0);
}

void copyBuffer(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size)
{
    VkCommandBuffer commandBuffer;
    beginSingleTimeCommands(device, commandPool, &commandBuffer);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(device, queue, commandPool, commandBuffer);
}