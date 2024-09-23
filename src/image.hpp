#ifndef IMAGE_HPP
#define IMAGE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>

// uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props, VkPhysicalDevice physical);

class RenderPipeline;

class Image{
public:
    Image(VkFormat _format, VkImageAspectFlags _flags)
    : flags(_flags), format(_format) {};
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkSampler sampler;
    void makeImageView(VkImageAspectFlags aspectFlags);
    void makeImage(uint32_t width, uint32_t height, VkImageTiling tiling, VkImageUsageFlags usage);
    void makeImageSampler();
    void transitionImageLayout(RenderPipeline& renderpipeline, VkImageLayout oldLayout, VkImageLayout newLayout);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
private:
    VkFormat format;
    VkImageAspectFlags flags;
};

#endif