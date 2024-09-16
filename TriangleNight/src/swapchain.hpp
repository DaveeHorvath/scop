#ifndef SWAPCHAIN_HPP
#define SWAPCHAIN_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <optional>

struct QueueFamilyIndicies
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> modes;
};

VkImageView makeImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
QueueFamilyIndicies findQueueFamilyIndicies(VkPhysicalDevice dev);

class Swapchain {
    private:
        VkPhysicalDevice& physicalDevice;
        VkDevice& device;
        VkSurfaceKHR& surface;
        GLFWwindow *win;

        SwapChainSupportDetails findSwapChainSupportDetails();
    public:
        Swapchain(VkPhysicalDevice& _physical, VkDevice& _device, VkSurfaceKHR& _surface, GLFWwindow *_win) : physicalDevice(_physical), device(_device), surface(_surface), win(_win) {};
        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImagesViews;
        void makeSwapchain();
        void remakeSwapchain();
        void cleanupSwapChain();
};

#endif