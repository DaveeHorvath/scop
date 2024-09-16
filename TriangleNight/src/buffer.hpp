#ifndef BUFFER_HPP
#define BUFFER_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props, VkPhysicalDevice physicalDevice);
class RenderPipeline;


class Buffer {
public:
    Buffer(VkDevice& _device, VkPhysicalDevice& _physical, RenderPipeline& _renderer) 
            : device(_device), physicalDevice(_physical), renderer(_renderer) {};
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    void *data;
    VkDeviceSize size;

    void init(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);
    void makeBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buffer, VkDeviceMemory& memory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
private:
    VkDevice& device;
    VkPhysicalDevice& physicalDevice;
    RenderPipeline& renderer;
};

#endif