#ifndef RENDERPIPELINE_HPP
#define RENDERPIPELINE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef MAX_FRAMES_IN_FLIGHT
# define MAX_FRAMES_IN_FLIGHT 2
#endif

#include <vector>

class RenderObject;

class RenderPipeline {
private:
    VkDevice& device;
    VkPhysicalDevice& physicalDevice;
    VkExtent2D& swapchainExtent;
    Model& model;
    Buffer& vertexBuffer;
    Buffer& indexBuffer;
    VkSurfaceKHR& surface;
    std::vector<Buffer>& uniformBuffers;
    Image& textureImage;
    Image& depthImage;
    VkFormat& swapchainImageFormat;
    Swapchain& swapchain;

    VkShaderModule makeShaderModule(const std::vector<char>& shader);
public:
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer buffer);
    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t image, uint32_t currenFrame);   

    void makeCommandPool();
    void makeCommandBuffer();

    void makeFrameBuffer();

    void makeDescriptorSetLayout();
    void makeDescriptorSets();
    void makeDescriptorPool();

    void makeRenderPass();
    void makePipeline();
};

#endif