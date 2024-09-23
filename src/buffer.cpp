#include "buffer.hpp"
#include <cstring>
#include "renderPipeline.hpp"
#include "Vulkan.hpp"
#include "app.hpp"

void Buffer::makeBuffer(VkDeviceSize tmpsize, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer &tmpbuffer, VkDeviceMemory &tmpbufferMemory)
{
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = tmpsize;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(VulkanInstance::device, &bufferCreateInfo, nullptr, &tmpbuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create buffer");
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VulkanInstance::device, tmpbuffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, props, VulkanInstance::physicalDevice);

    if (vkAllocateMemory(VulkanInstance::device, &allocInfo, nullptr, &tmpbufferMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate memory");
    vkBindBufferMemory(VulkanInstance::device, tmpbuffer, tmpbufferMemory, 0);
}

void Buffer::init(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props)
{

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    makeBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    makeBuffer(size, usage, props | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, bufferMemory);

    void *data;
    vkMapMemory(VulkanInstance::device, stagingBufferMemory, 0, size, 0, &data);
    memcpy(data, data, (size_t)size);
    vkUnmapMemory(VulkanInstance::device, stagingBufferMemory);

    copyBuffer(stagingBuffer, buffer, size);

    vkDestroyBuffer(VulkanInstance::device, stagingBuffer, nullptr);
    vkFreeMemory(VulkanInstance::device, stagingBufferMemory, nullptr);
}

void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer cmdBuffer = RenderPipeline::beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    RenderPipeline::endSingleTimeCommands(cmdBuffer);
}