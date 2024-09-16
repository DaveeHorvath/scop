#include "swapchain.hpp"
#include "QueueFamilyIndicies.hpp"

void Swapchain::makeSwapchain()
{
    SwapChainSupportDetails details = findSwapChainSupportDetails();
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
        imageCount = details.capabilities.maxImageCount;
    VkSurfaceFormatKHR surfaceFormat = pickSurfaceFormat(details.formats);
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainImageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageFormat = swapchainImageFormat;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainExtent = pickSwapChainExtent(details.capabilities);
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.presentMode = pickSwapPresentMode(details.modes);
    QueueFamilyIndicies indicies = QueueFamilyIndicies::findQueueFamilyIndicies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indicies.graphicsFamily.value(), indicies.presentFamily.value()};
    if (indicies.graphicsFamily != indicies.presentFamily) 
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = details.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = nullptr;
    if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapchain");

    /* Make swapchain images */
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
    swapchainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
    /* Make swapchain image views */
    swapchainImagesViews.resize(swapchainImages.size());
    for (int i = 0; i < swapchainImages.size(); i++)
        swapchainImagesViews[i] = makeImageView(swapchainImages[i], swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Swapchain::remakeSwapchain()
{
    int width = 0, height = 0;
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(win, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);
    cleanupSwapChain();
    makeSwapchain();
    // makeDepthResources();
    // makeFrameBuffer();
}

// void Swapchain::makeFrameBuffer()
//         {
//             swapchainFramebuffers.resize(swapchainImagesViews.size());

//             for (int i = 0; i < swapchainImagesViews.size(); i++)
//             {
//                 std::array<VkImageView, 2> attachments = {
//                     swapchainImagesViews[i],
//                     depthImageView
//                 };

//                 VkFramebufferCreateInfo framebufferCreateInfo{};
//                 framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//                 framebufferCreateInfo.renderPass = renderPass;
//                 framebufferCreateInfo.attachmentCount = attachments.size();
//                 framebufferCreateInfo.pAttachments = attachments.data();
//                 framebufferCreateInfo.height = swapchainExtent.height;
//                 framebufferCreateInfo.width = swapchainExtent.width;
//                 framebufferCreateInfo.layers = 1;

//                 if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
//                     throw std::runtime_error("Failed to create framebuffers");
//             }
//         }

void Swapchain::cleanupSwapChain()
{
    /* other classes responsibility*/
    // vkDestroyImageView(device, depthImageView, nullptr);
    // vkDestroyImage(device, depthImage, nullptr);
    // vkFreeMemory(device, depthImageMemory, nullptr);
    for (auto& imview : swapchainImagesViews)
        vkDestroyImageView(device, imview, nullptr);
    // for (auto& framebuf : swapchainFramebuffers)
    //     vkDestroyFramebuffer(device, framebuf, nullptr);
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

SwapChainSupportDetails Swapchain::findSwapChainSupportDetails()
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }
    uint32_t modesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modesCount, nullptr);
    if (modesCount)
    {
        details.modes.resize(modesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modesCount, details.modes.data());
    }
    return details;
}

/* UTILS */
VkSurfaceFormatKHR pickSurfaceFormat(const std::vector<VkSurfaceFormatKHR> formats)
{
    for (const auto& format : formats) 
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }
    return formats[0];
}
VkPresentModeKHR pickSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
{
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D pickSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    return capabilities.currentExtent;
}