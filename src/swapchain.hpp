#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <optional>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> modes;
};

VkImageView makeImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

class Swapchain {
    private:
        SwapChainSupportDetails findSwapChainSupportDetails();
    public:
        inline static VkSwapchainKHR swapchain;
        inline static VkExtent2D swapchainExtent;
        // needs refactor into Image class
        inline static std::vector<VkImage> swapchainImages;
        inline static std::vector<VkImageView> swapchainImagesViews;
        inline static VkFormat swapchainImageFormat;
        uint32_t swapchainImageCount = 0;
        void makeSwapchain();
        void remakeSwapchain();
        void cleanupSwapChain();
};

#endif