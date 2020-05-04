#include "evulkan_core.h"

#include <set>
#include <iostream>

void evkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const EVkDeviceCreateInfo *pCreateInfo,
    VkDevice *pDevice,
    VkQueue *pGraphicsQueue,
    VkQueue *pPresentQueue)
{
    QueueFamilyIndices indices = getQueueFamilies(physicalDevice, pCreateInfo->surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(pCreateInfo->deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = pCreateInfo->deviceExtensions.data();

    if (ENABLE_VALIDATION) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(pCreateInfo->validationLayers.size());
        createInfo.ppEnabledLayerNames = pCreateInfo->validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, pDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device.");
    }

    vkGetDeviceQueue(*pDevice, indices.graphicsFamily.value(), 0, pGraphicsQueue);
    vkGetDeviceQueue(*pDevice, indices.presentFamily.value(), 0, pPresentQueue);
}

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); //Crashes here.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
        }
        if (indices.isComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

void evkCreateSwapchain(
    VkDevice device,
    const EVkSwapchainCreateInfo *pCreateInfo,
    VkSwapchainKHR *pSwapchain,
    std::vector<VkImage> *pSwapchainImages,
    VkFormat *pSwapchainImageFormat,
    VkExtent2D *pSwapchainExtent)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pCreateInfo->physicalDevice, pCreateInfo->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(pCreateInfo->window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = pCreateInfo->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(pCreateInfo->physicalDevice, pCreateInfo->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(device, &createInfo, nullptr, pSwapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(device, *pSwapchain, &imageCount, nullptr);
    pSwapchainImages->resize(imageCount);
    vkGetSwapchainImagesKHR(device, *pSwapchain, &imageCount, pSwapchainImages->data());

    *pSwapchainImageFormat = surfaceFormat.format;
    *pSwapchainExtent = extent;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    throw std::runtime_error("no suitable format found in available formats.");
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }   
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr); //Crashes here.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
        }
        if (indices.isComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

void evkCreateImageViews(
    VkDevice device,
    const EVkImageViewsCreateInfo *pCreateInfo,
    std::vector<VkImageView> *pSwapChainImageViews)
{
    pSwapChainImageViews->resize(pCreateInfo->images.size());

    for (uint32_t i = 0; i < pCreateInfo->images.size(); i++) {
        (*pSwapChainImageViews)[i] = createImageView(device, pCreateInfo->images[i], pCreateInfo->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkImageView createImageView(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void evkCreateRenderPass(
    VkDevice device,
    const EVkRenderPassCreateInfo *pCreateInfo,
    VkRenderPass *pRenderPass)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = pCreateInfo->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat(pCreateInfo);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, pRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass.");
    }
}

VkFormat findDepthFormat(
    const EVkRenderPassCreateInfo *pCreateInfo)
{
    return findSupportedFormat(
        pCreateInfo,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat findSupportedFormat(
    const EVkRenderPassCreateInfo *pCreateInfo,
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(pCreateInfo->physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format.");
}

// Create the descriptor layout. This specifies the type of resources that are going
// to be accessed by the pipeline. The descriptor set describes the actual buffer
// or image that will be bound to the descriptor.
void evkCreateDescriptorSetLayout(
    VkDevice device,
    const EVkDescriptorSetLayoutCreateInfo *pCreateInfo,
    VkDescriptorSetLayout *pDescriptorSetLayout)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, pDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout.");
    }
}

void evkCreateGraphicsPipeline(
    VkDevice device,
    const EVkGraphicsPipelineCreateInfo *pCreateInfo,
    VkPipelineLayout *pPipelineLayout,
    VkPipeline *pPipeline)
{
    auto vertShaderCode = readFile("shaders/vert.spv");
    auto fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule;
    createShaderModule(device, vertShaderCode, &vertShaderModule);
    VkShaderModule fragShaderModule;
    createShaderModule(device, fragShaderCode, &fragShaderModule);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    // Set up input to vertex shader.
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Set up input assembly.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Set up the viewport.
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) pCreateInfo->swapchainExtent.width;
    viewport.height = (float) pCreateInfo->swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Set up scissor.
    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = pCreateInfo->swapchainExtent;

    // Combine viewport and scissor.
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Set up the rasterizer.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Set up multisampling.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Set up colour blending.
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = pCreateInfo->pDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, pPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout.");
    }

    // Set up depth testing.
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    // Fixed function stages.
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    
    // Pipeline layout.
    pipelineInfo.layout = *pPipelineLayout;

    // Render pass and sub-pass.
    pipelineInfo.renderPass = pCreateInfo->renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    // Add the depth stencil.
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, pPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline.");
    }        
        
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void createShaderModule(VkDevice device, const std::vector<char>& code, VkShaderModule *pShaderModule)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, pShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module.");
    }
}

void evkCreateDepthResources(
    VkDevice device,
    const EVkDepthResourcesCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkImageView *pImageView,
    VkDeviceMemory *pImageMemory)
{
    EVkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.swapChainImageFormat = pCreateInfo->swapchainImageFormat;
    renderPassInfo.physicalDevice = pCreateInfo->physicalDevice;
    VkFormat depthFormat = findDepthFormat(&renderPassInfo);

    EVkImageCreateInfo createInfo = {};
    createInfo.physicalDevice = pCreateInfo->physicalDevice;
    createInfo.width = pCreateInfo->swapchainExtent.width;
    createInfo.height = pCreateInfo->swapchainExtent.height;
    createInfo.format = depthFormat;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    createInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    evkCreateImage(device, &createInfo, pImage, pImageMemory);
    *pImageView = createImageView(device, *pImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void evkCreateImage(
    VkDevice device,
    const EVkImageCreateInfo *pCreateInfo,
    VkImage *pImage,
    VkDeviceMemory *pImageMemory)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = pCreateInfo->width;
    imageInfo.extent.height = pCreateInfo->height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = pCreateInfo->format;
    imageInfo.tiling = pCreateInfo->tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = pCreateInfo->usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, nullptr, pImage) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *pImage, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        pCreateInfo->physicalDevice,
        memRequirements.memoryTypeBits,
        pCreateInfo->properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, pImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory.");
    }

    vkBindImageMemory(device, *pImage, *pImageMemory, 0);
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1<<i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type.");
}

void evkCreateFramebuffers(
    VkDevice device,
    const EVkFramebuffersCreateInfo *pCreateInfo,
    std::vector<VkFramebuffer> *pFramebuffers
)
{
    pFramebuffers->resize(pCreateInfo->swapchainImageViews.size());

    for (size_t i = 0; i < pCreateInfo->swapchainImageViews.size(); i++)
    {
        std::array<VkImageView,2> attachments =
        {
            pCreateInfo->swapchainImageViews[i],
            pCreateInfo->depthImageView            
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = pCreateInfo->renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = pCreateInfo->swapchainExtent.width;
        framebufferInfo.height = pCreateInfo->swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &(*pFramebuffers)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer.");
        }
    }
}

void beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer *pCommandBuffer)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &allocInfo, pCommandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(*pCommandBuffer, &beginInfo);
}

void endSingleTimeCommands(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void evkDrawFrame(
    VkDevice device,
    const EVkDrawFrameInfo *pDrawInfo,
    size_t *pCurrentFrame,
    std::vector<VkFence> *pImagesInFlight,
    std::vector<VkSemaphore> *pRenderFinishedSemaphores, uint32_t *pImageIndex)
{
    const std::vector<VkFence> &inFlightFences = *(pDrawInfo->pInFlightFences);
    const std::vector<VkSemaphore> &imageAvailableSemaphores = *(pDrawInfo->pImageAvailableSemaphores);
    std::vector<VkFence> &imagesInFlight = *(pImagesInFlight);
    // const std::vector<VkCommandBuffer> &commandBuffers = *(pDrawInfo->pCommandBuffers);
    const VkCommandBuffer &commandBuffer = pDrawInfo->primaryCommandBuffer;

    vkWaitForFences(device, 1, &inFlightFences[*pCurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t &imageIndex = *pImageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device, pDrawInfo->swapchain, UINT64_MAX,
        imageAvailableSemaphores[*pCurrentFrame],
        VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        std::cout << "RECREATE SWAPCHAIN\n";
        // recreateSwapChain();
        // return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image.");
    }

    // Check if a previous frame is using this image. If so, wait on its fence.
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device, 1, &(imagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX);
    }

    // Mark the image as being in use.
    imagesInFlight[imageIndex] = inFlightFences[*pCurrentFrame];

    // Update the uniform buffers.
    EVkUniformBufferUpdateInfo updateInfo = {};
    updateInfo.currentImage = *pImageIndex;
    updateInfo.swapchainExtent = pDrawInfo->swapchainExtent;
    updateInfo.pUniformBufferMemory = pDrawInfo->pUniformBufferMemory;
    evkUpdateUniformBuffer(device, &updateInfo);

    // Also need to upate the vertex buffers.
    EVkVertexBufferUpdateInfo vUpdateInfo = {};
    vUpdateInfo.pVertices = pDrawInfo->pVertices;
    vUpdateInfo.physicalDevice = pDrawInfo->physicalDevice;
    vUpdateInfo.commandPool = pDrawInfo->commandPool;
    vUpdateInfo.graphicsQueue = pDrawInfo->graphicsQueue;
    vUpdateInfo.vertexBuffer = pDrawInfo->vertexBuffer;
    vUpdateInfo.grid = pDrawInfo->grid;
    evkUpdateVertexBuffer(device, &vUpdateInfo);
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[*pCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {(*pRenderFinishedSemaphores)[*pCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[*pCurrentFrame]);

    if (vkQueueSubmit(pDrawInfo->graphicsQueue, 1, &submitInfo, inFlightFences[*pCurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {pDrawInfo->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(pDrawInfo->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || *pDrawInfo->pFramebufferResized)
    {
        // *pDrawInfo->pFramebufferResized = false;
        std::cout << "RECREATE SWAPCHAIN\n";
        // recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image.");
    }

    vkQueueWaitIdle(pDrawInfo->presentQueue);

    *pCurrentFrame = ((*pCurrentFrame)+1) % pDrawInfo->maxFramesInFlight;
}

void evkRecreateSwapChain(VkDevice device, const EVkSwapchainRecreateInfo *pCreateInfo)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(pCreateInfo->pWindow, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(pCreateInfo->pWindow, &width, &height);
        glfwWaitEvents();
    }

    // Wait until nobody is using the device.
    vkDeviceWaitIdle(device);

    EVkSwapchainCleanupInfo cleanupInfo = {};
    cleanupInfo.depthImage = *pCreateInfo->pDepthImage;
    cleanupInfo.depthImageView = *pCreateInfo->pDepthImageView;
    cleanupInfo.swapchainFramebuffers = *pCreateInfo->pSwapchainFramebuffers;
    cleanupInfo.commandPool = pCreateInfo->commandBuffersCreateInfo.commandPool;
    cleanupInfo.pCommandBuffers = pCreateInfo->pCommandBuffers;
    cleanupInfo.graphicsPipeline = *pCreateInfo->pPipeline;
    cleanupInfo.pipelineLayout = *pCreateInfo->pPipelineLayout;
    cleanupInfo.renderPass = *pCreateInfo->pRenderPass;
    cleanupInfo.swapchainImageViews = *pCreateInfo->pSwapchainImageViews;
    cleanupInfo.swapchain = *pCreateInfo->pSwapchain;
    cleanupInfo.swapchainImages = *pCreateInfo->pSwapchainImages;
    cleanupInfo.uniformBuffers = *pCreateInfo->pUniformBuffers;
    cleanupInfo.uniformBuffersMemory = *pCreateInfo->pUniformBuffersMemory;
    cleanupInfo.descriptorPool = *pCreateInfo->pDescriptorPool;
    evkCleanupSwapchain(device, &cleanupInfo);

    EVkSwapchainCreateInfo swapchainInfo = {};
    swapchainInfo = pCreateInfo->swapchainCreateInfo;
    evkCreateSwapchain(device, &swapchainInfo, pCreateInfo->pSwapchain, pCreateInfo->pSwapchainImages, pCreateInfo->pSwapchainImageFormats, pCreateInfo->pSwapchainExtent);

    EVkImageViewsCreateInfo imageViewsInfo = pCreateInfo->imageViewsCreateInfo;
    evkCreateImageViews(device, &imageViewsInfo, pCreateInfo->pSwapchainImageViews);

    EVkRenderPassCreateInfo renderPassInfo = pCreateInfo->renderPassCreateInfo;
    evkCreateRenderPass(device, &renderPassInfo, pCreateInfo->pRenderPass);

    EVkGraphicsPipelineCreateInfo pipelineInfo = pCreateInfo->graphicsPipelineCreateInfo;
    evkCreateGraphicsPipeline(device, &pipelineInfo, pCreateInfo->pPipelineLayout, pCreateInfo->pPipeline);

    EVkDepthResourcesCreateInfo depthResourcesInfo = pCreateInfo->depthResourcesCreateInfo;
    evkCreateDepthResources(device, &depthResourcesInfo, pCreateInfo->pDepthImage, pCreateInfo->pDepthImageView, pCreateInfo->pDepthImageMemory);

    EVkFramebuffersCreateInfo framebuffersInfo = pCreateInfo->framebuffersCreateInfo;
    evkCreateFramebuffers(device, &framebuffersInfo, pCreateInfo->pSwapchainFramebuffers);

    EVkUniformBufferCreateInfo uniformBufferInfo = pCreateInfo->uniformBuffersCreateInfo;
    evkCreateUniformBuffers(device, &uniformBufferInfo, pCreateInfo->pUniformBuffers, pCreateInfo->pUniformBuffersMemory);

    EVkDescriptorPoolCreateInfo descriptorPoolInfo = pCreateInfo->descriptorPoolCreateInfo;
    evkCreateDescriptorPool(device, &descriptorPoolInfo, pCreateInfo->pDescriptorPool);

    EVkDescriptorSetCreateInfo descriptorSetInfo = pCreateInfo->EVkDescriptorSetCreateInfo;
    evkCreateDescriptorSets(device, &descriptorSetInfo, pCreateInfo->pDescriptorSets);

    EVkCommandBuffersCreateInfo commandBuffersInfo = pCreateInfo->commandBuffersCreateInfo;
    evkCreateCommandBuffers(device, &commandBuffersInfo, pCreateInfo->pCommandBuffers, pCreateInfo->pPrimaryCommandBuffer);
}

void evkCleanupSwapchain(VkDevice device, const EVkSwapchainCleanupInfo *pCleanupInfo)
{
    vkDestroyImageView(device, pCleanupInfo->depthImageView, nullptr);
    vkDestroyImage(device, pCleanupInfo->depthImage, nullptr);
    vkFreeMemory(device, pCleanupInfo->depthImageMemory, nullptr);

    for (auto framebuffer : pCleanupInfo->swapchainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(
        device, pCleanupInfo->commandPool,
        static_cast<uint32_t>(pCleanupInfo->pCommandBuffers->size()),
        pCleanupInfo->pCommandBuffers->data());

    vkDestroyPipeline(device, pCleanupInfo->graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pCleanupInfo->pipelineLayout, nullptr);
    vkDestroyRenderPass(device, pCleanupInfo->renderPass, nullptr);

    for (auto imageView : pCleanupInfo->swapchainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, pCleanupInfo->swapchain, nullptr);

    for (size_t i = 0; i < pCleanupInfo->swapchainImages.size(); i++)
    {
        vkDestroyBuffer(device, pCleanupInfo->uniformBuffers[i], nullptr);
        vkFreeMemory(device, pCleanupInfo->uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, pCleanupInfo->descriptorPool, nullptr);
}