#ifndef BUFFER_HPP
#define BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props, VkPhysicalDevice physicalDevice);
class RenderPipeline;


class Buffer {
public:
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    void *data;
    VkDeviceSize size;

    void init(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
    static void makeBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buffer, VkDeviceMemory& memory);
    static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};

#endif