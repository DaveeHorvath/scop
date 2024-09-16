#include "window.hpp"
#include <iostream>

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