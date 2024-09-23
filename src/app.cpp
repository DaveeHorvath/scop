#include "app.hpp"
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <stdexcept>

void App::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto current = std::chrono::high_resolution_clock::now();
    float deltatime = std::chrono::duration<float, std::chrono::seconds::period>(current - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), deltatime * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view = glm::lookAt(glm::vec3(5.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(90.0f), Swapchain::swapchainExtent.width / (float)Swapchain::swapchainExtent.height, 0.1f, 30.0f);
    // ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void App::drawFrame()
{
    vkWaitForFences(VulkanInstance::device, 1, &syncobjects.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t image;
    VkResult res = vkAcquireNextImageKHR(VulkanInstance::device, Swapchain::swapchain, UINT64_MAX, syncobjects.imageDoneSemaphores[currentFrame], VK_NULL_HANDLE, &image);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        swapchain.remakeSwapchain();
        makeDepthResources();
        renderpipeline.makeFrameBuffer(depth);
        return;
    }
    else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to get next image");

    updateUniformBuffer(currentFrame);

    vkResetFences(VulkanInstance::device, 1, &syncobjects.inFlightFences[currentFrame]);

    vkResetCommandBuffer(renderpipeline.commandBuffers[currentFrame], 0);
    renderpipeline.recordCommandBuffer(renderpipeline.commandBuffers[currentFrame], image, currentFrame, vertexBuffer, indexBuffer, model);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderpipeline.commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &syncobjects.renderFinishedSemaphores[currentFrame];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &syncobjects.imageDoneSemaphores[currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;

    if (vkQueueSubmit(RenderPipeline::graphicsQueue, 1, &submitInfo, syncobjects.inFlightFences[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit to queue");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain.swapchain;
    presentInfo.pImageIndices = &image;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &syncobjects.renderFinishedSemaphores[currentFrame];

    res = vkQueuePresentKHR(RenderPipeline::graphicsQueue, &presentInfo);

    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || frameResize)
    {
        frameResize = false;
        swapchain.remakeSwapchain();
        makeDepthResources();
        renderpipeline.makeFrameBuffer(depth);
        return;
    }
    else if (res != VK_SUCCESS)
        throw std::runtime_error("Failed to present next image");

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// fix this
void App::makeIndexBuffer()
{
    VkDeviceSize deviceSize = sizeof(model.indices[0]) * model.indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    Buffer::makeBuffer(deviceSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(instance.device, stagingBufferMemory, 0, deviceSize, 0, &data);
    memcpy(data, model.indices.data(), (size_t)deviceSize);
    vkUnmapMemory(instance.device, stagingBufferMemory);

    Buffer::makeBuffer(deviceSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer.buffer, indexBuffer.bufferMemory);

    Buffer::copyBuffer(stagingBuffer, indexBuffer.buffer, deviceSize);

    vkDestroyBuffer(instance.device, stagingBuffer, nullptr);
    vkFreeMemory(instance.device, stagingBufferMemory, nullptr);
}

// fix this
void App::makeVertexBuffer()
{
    VkDeviceSize deviceSize = sizeof(model.vertices[0]) * model.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    Buffer::makeBuffer(deviceSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(instance.device, stagingBufferMemory, 0, deviceSize, 0, &data);
    memcpy(data, model.vertices.data(), (size_t)deviceSize);
    vkUnmapMemory(instance.device, stagingBufferMemory);

    Buffer::makeBuffer(deviceSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.buffer, vertexBuffer.bufferMemory);

    Buffer::copyBuffer(stagingBuffer, vertexBuffer.buffer, deviceSize);

    vkDestroyBuffer(instance.device, stagingBuffer, nullptr);
    vkFreeMemory(instance.device, stagingBufferMemory, nullptr);
}

void App::makeUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        Buffer::makeBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer, uniformBuffers[i].bufferMemory);
        vkMapMemory(instance.device, uniformBuffers[i].bufferMemory, 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

// fix this
void App::makeTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load("swmg.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
        throw std::runtime_error("Failed to load texture image!");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    Buffer::makeBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void *data;
    vkMapMemory(VulkanInstance::device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, imageSize);
    vkUnmapMemory(VulkanInstance::device, stagingBufferMemory);

    stbi_image_free(pixels);

    texture.makeImage(texWidth, texHeight, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    texture.transitionImageLayout(renderpipeline, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // specify correct layout for transfers
    copyBufferToImage(stagingBuffer, texture.image, texWidth, texHeight);
    texture.transitionImageLayout(renderpipeline, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // specify correct layout for shaders

    vkDestroyBuffer(VulkanInstance::device, stagingBuffer, nullptr);
    vkFreeMemory(VulkanInstance::device, stagingBufferMemory, nullptr);
}

// fix this
void App::makeDepthResources()
{
    depth.makeImage(swapchain.swapchainExtent.width, swapchain.swapchainExtent.height, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depth.makeImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
}

void App::loop()
{
    std::cout << "=====  LOOP'O CLOCK  =====\n";
    while (!glfwWindowShouldClose(Window::win))
    {
        glfwPollEvents();
        drawFrame();
    }
    std::cout << "=====  WAIT FOR CLEAN  =====\n";
    vkDeviceWaitIdle(VulkanInstance::device);
}

// needs refactor
void App::clean()
{
    std::cout << "=====  CLEANUP  =====\n";

    // for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    // {
    //     vkDestroyBuffer(VulkanInstance::device, uniformBuffers[i], nullptr);
    //     vkFreeMemory(VulkanInstance::device, uniformBuffersMemory[i], nullptr);
    // }

    // vkDestroyImageView(VulkanInstance::device, depthImageView, nullptr);
    // vkDestroyImage(VulkanInstance::device, depthImage, nullptr);
    // vkFreeMemory(VulkanInstance::device, depthImageMemory, nullptr);

    // vkDestroySampler(VulkanInstance::device, textureSampler, nullptr);

    // vkDestroyImageView(VulkanInstance::device, textureImageView, nullptr);

    // vkDestroyImage(VulkanInstance::device, textureImage, nullptr);
    // vkFreeMemory(VulkanInstance::device, textureImageMemory, nullptr);

    // vkDestroyDescriptorPool(VulkanInstance::device, descriptorPool, nullptr);
    // vkDestroyDescriptorSetLayout(VulkanInstance::device, descriptorSetLayout, nullptr);

    // vkDestroyBuffer(VulkanInstance::device, vertexBuffer, nullptr);
    // vkFreeMemory(VulkanInstance::device, vertexBufferMemory, nullptr);

    // vkDestroyBuffer(VulkanInstance::device, indexBuffer, nullptr);
    // vkFreeMemory(VulkanInstance::device, indexBufferMemory, nullptr);

    // for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    // {
    //     vkDestroyFence(VulkanInstance::device, inFlightFences[i], nullptr);
    //     vkDestroySemaphore(VulkanInstance::device, imageDoneSemaphores[i], nullptr);
    //     vkDestroySemaphore(VulkanInstance::device, renderFinishedSemaphores[i], nullptr);
    // }

    // vkDestroyCommandPool(VulkanInstance::device, commandPool, nullptr);
    // for (auto &buffer : swapchainFramebuffers)
    //     vkDestroyFramebuffer(VulkanInstance::device, buffer, nullptr);
    // vkDestroyPipeline(VulkanInstance::device, graphicsPipeline, nullptr);
    // vkDestroyPipelineLayout(VulkanInstance::device, pipelineLayout, nullptr);
    // vkDestroyRenderPass(VulkanInstance::device, renderPass, nullptr);
    // for (auto &imageView : swapchainImagesViews)
    //     vkDestroyImageView(VulkanInstance::device, imageView, nullptr);
    // vkDestroySwapchainKHR(VulkanInstance::device, swapchain, nullptr);
    // vkDestroyDevice(VulkanInstance::device, nullptr);
    // vkDestroySurfaceKHR(instance, surface, nullptr);
    // vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window.win);
    glfwTerminate();
}

void App::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = RenderPipeline::beginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    RenderPipeline::endSingleTimeCommands(commandBuffer);
}

void App::run()
{
    init();
    loop();
    clean();
}

void App::init()
{
    window.init();
    std::cout << "instance creation\n";
    instance.init();
    // init_window();

    /* Surface*/
    instance.createSurface();

    /* device selection and creation */
    instance.pickPhysicalDevice();
    instance.makeLogicalDevice();

    std::cout << "swapchain creation\n";
    /* SwapChain */
    swapchain.makeSwapchain();

    /* pipeline */
    std::cout << "renderpipeline creation\n";
    renderpipeline.makeRenderPass(depth);
    renderpipeline.makeDescriptorSetLayout();
    renderpipeline.makePipeline();
    renderpipeline.makeCommandPool();
    std::cout << "depth resource\n";
    makeDepthResources();
    std::cout << "frame buffer\n";
    renderpipeline.makeFrameBuffer(depth);
    std::cout << "texture image\n";
    makeTextureImage();
    texture.makeImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    texture.makeImageSampler();
    std::cout << "cmdbuffer\n";
    renderpipeline.makeCommandBuffer();

    /* Vertex Buffer */
    std::cout << "model creation\n";
    model.loadModel();

    std::cout << "buffer creation\n";
    makeVertexBuffer();
    makeIndexBuffer();
    makeUniformBuffers();
    renderpipeline.makeDescriptorPool();
    renderpipeline.makeDescriptorSets(uniformBuffers, texture);

    /* Sync */
    std::cout << "sync creation\n";
    syncobjects.makeSyncObjects();
}