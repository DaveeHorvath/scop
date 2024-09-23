#ifndef RENDERPIPELINE_HPP
#define RENDERPIPELINE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifndef MAX_FRAMES_IN_FLIGHT
# define MAX_FRAMES_IN_FLIGHT 2
#endif

#include <vector>

class RenderObject;
class Image;
class Model;
class Buffer;

class RenderPipeline {
private:
    VkShaderModule makeShaderModule(const std::vector<char>& shader);
public:
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    inline static VkQueue graphicsQueue;
    inline static VkQueue presentQueue;

    inline static VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;

    inline static VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    static VkCommandBuffer beginSingleTimeCommands();
    static void endSingleTimeCommands(VkCommandBuffer buffer);
    void recordCommandBuffer(VkCommandBuffer buffer, uint32_t image, uint32_t currentFrame , Buffer vertexBuffer, Buffer indexBuffer, Model model);

    void makeCommandPool();
    void makeCommandBuffer();

    void makeFrameBuffer(Image depthImage);

    void makeDescriptorSetLayout();
    void makeDescriptorSets(std::vector<Buffer> uniformBuffers, Image textureImage);
    void makeDescriptorPool();

    void makeRenderPass(Image depthImage);
    void makePipeline();
};

#endif