#ifndef SYNCOBJ_HPP
#define SYNCOBJ_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>

#define MAX_FRAMES_IN_FLIGHT 2

class Syncobjects {
private:
public:
    inline static std::vector<VkSemaphore> imageDoneSemaphores;
    inline static std::vector<VkSemaphore> renderFinishedSemaphores;
    inline static std::vector<VkFence> inFlightFences;
    void makeSyncObjects();
};

#endif