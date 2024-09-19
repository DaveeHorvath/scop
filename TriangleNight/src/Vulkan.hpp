#ifndef VULKAN_HPP
#define VULKAN_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>

#ifdef DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

class VulkanInstance {
private:
    const std::vector<const char *> validationLayer {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance instance;
public:
    static VkPhysicalDevice physicalDevice;
    static VkDevice device;
    static VkSurfaceKHR surface;

    void init();
    bool check_validation_layer_support();
    void pickPhysicalDevice();
    void makeLogicalDevice();
    void createSurface();
};

#endif