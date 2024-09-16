#ifndef QUEUEFAMILY_HPP
#define QUEUEFAMILY_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>

struct QueueFamilyIndicies
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    static QueueFamilyIndicies findQueueFamilyIndicies(VkPhysicalDevice dev, VkSurfaceKHR surface)
    {
        QueueFamilyIndicies indicies;

        uint32_t deviceQueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &deviceQueueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(deviceQueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &deviceQueueFamilyCount, families.data());
        int index = 0;
        for (auto &fam : families)
        {
            if (fam.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indicies.graphicsFamily = index;
            VkBool32 isPresentQueue = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(dev, index, surface, &isPresentQueue);
            if (isPresentQueue)
                indicies.presentFamily = index;
            index++;
        }
        return indicies;
    }
};

#endif
