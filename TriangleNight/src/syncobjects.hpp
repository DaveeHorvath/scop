#ifndef SYNCOBJ_HPP
#define SYNCOBJ_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>

#define MAX_FRAMES_IN_FLIGHT 2

class Syncobjects {
private:
    VkDevice& device;
public:
    Syncobjects(VkDevice& _device) : device(_device) {};
    std::vector<VkSemaphore> imageDoneSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    void makeSyncObjects();
};

#endif