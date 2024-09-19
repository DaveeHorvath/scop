#include "window.hpp"
#include <iostream>
#include "app.hpp"
void Window::init()
{
    std::cout << "=====  Window init  =====\n";
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    win = glfwCreateWindow(WIDTH, HEIGHT, "FUNny", nullptr, nullptr);
    glfwSetWindowUserPointer(win, this);
    glfwSetFramebufferSizeCallback(win, framebufferResizeCallback);
}

static void framebufferResizeCallback(GLFWwindow *win, int height, int width)
{
    auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(win));
    app->frameResize = true;
}