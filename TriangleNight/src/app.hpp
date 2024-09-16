#ifndef APP_HPP
#define APP_HPP
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props, VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        if (typeFilter & (i << 1) && (memProps.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    throw std::runtime_error("Failed to find suitable memory type");
}
#include "UniformBufferObject.hpp"
#include "window.hpp"
#include "Vulkan.hpp"
#include "swapchain.hpp"
#include "renderPipeline.hpp"
#include "syncobjects.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "Model.hpp"

#define MAX_FRAMES_IN_FLIGHT 2


class App {
    uint32_t currentFrame;
    public:
        Window window;
        bool frameResize = false;

        void run()
        {
            init();
            loop();
            clean();
        }
    private:
        VulkanInstance instance{renderpipeline.graphicsQueue, renderpipeline.presentQueue, window.win};
        // VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        // VkDevice device = VK_NULL_HANDLE;
        // VkSurfaceKHR surface;

        Swapchain swapchain{instance.physicalDevice, instance.device, instance.surface, window.win};
        // VkSwapchainKHR swapchain;

        RenderPipeline renderpipeline;
        // VkQueue graphicsQueue;
        // VkQueue presentQueue;

        // VkRenderPass renderPass;
        // VkDescriptorSetLayout descriptorSetLayout;
        // VkPipelineLayout pipelineLayout;

        // VkPipeline graphicsPipeline;
        // std::vector<VkFramebuffer> swapchainFramebuffers;

        // VkCommandPool commandPool;
        // std::vector<VkCommandBuffer> commandBuffers;

        // VkDescriptorPool descriptorPool;
        // std::vector<VkDescriptorSet> descriptorSets;

        /* Sync objects */
        Syncobjects syncobjects{instance.device};
        // std::vector<VkSemaphore> imageDoneSemaphores;
        // std::vector<VkSemaphore> renderFinishedSemaphores;
        // std::vector<VkFence> inFlightFences;

        /* should be a class*/
        Buffer vertexBuffer{instance.device, instance.physicalDevice, renderpipeline};
        // VkBuffer vertexBuffer;
        // VkDeviceMemory vertexBufferMemory;
        /* end of class */

        Buffer indexBuffer{instance.device, instance.physicalDevice, renderpipeline};
        // VkBuffer indexBuffer;
        // VkDeviceMemory indexBufferMemory;

        std::vector<Buffer> uniformBuffers;
        // std::vector<VkBuffer> uniformBuffers;
        // std::vector<VkDeviceMemory> uniformBuffersMemory;
        // std::vector<void *> uniformBuffersMapped;


        Image texture{VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            instance.device, instance.physicalDevice};
        // VkImage textureImage;
        // VkDeviceMemory textureImageMemory;
        // VkImageView textureImageView;
        // VkSampler textureSampler;

        Image depth{VK_FORMAT_UNDEFINED, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            instance.device, instance.physicalDevice};
        // VkImage depthImage;
        // VkDeviceMemory depthImageMemory;
        // VkImageView depthImageView;

        Model model;

        void updateUniformBuffer(uint32_t currentImage);
        void drawFrame();
        
        // needs to move somewhere, unsure atm
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
        {
            VkCommandBuffer commandBuffer = renderpipeline.beginSingleTimeCommands();
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
                    1
                };
                vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            renderpipeline.endSingleTimeCommands(commandBuffer);
        }
        void makeIndexBuffer();
        void makeVertexBuffer();
        void makeUniformBuffers();

        void makeTextureImage();

        void makeDepthResources();        
        void init()
        {
            window.init();
            // init_window();

            /* Surface*/
            instance.createSurface();

            /* device selection and creation */
            instance.pickPhysicalDevice();
            instance.makeLogicalDevice();

            /* SwapChain */
            swapchain.makeSwapchain();

            /* pipeline */
            renderpipeline.makeRenderPass();
            renderpipeline.makeDescriptorSetLayout();
            renderpipeline.makePipeline();
            renderpipeline.makeCommandPool();
            makeDepthResources();
            renderpipeline.makeFrameBuffer();
            makeTextureImage();
            // makeTextureImageView();
            // makeTextureSampler();
            renderpipeline.makeCommandBuffer();

            /* Vertex Buffer */
            model.loadModel();

            makeVertexBuffer();
            makeIndexBuffer();
            makeUniformBuffers();
            renderpipeline.makeDescriptorPool();
            renderpipeline.makeDescriptorSets();

            /* Sync */
            syncobjects.makeSyncObjects();
        }
        void loop();
        
        void clean();
        
};

#endif