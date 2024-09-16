#ifndef WINDOW_HPP
#define WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define WIDTH 1000
#define HEIGHT 1000

static void framebufferResizeCallback(GLFWwindow * win, int height, int width);

class Window {
public:
    void init();
    GLFWwindow *win;
};

#endif