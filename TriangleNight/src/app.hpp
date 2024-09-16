#ifndef APP_HPP
#define APP_HPP

#include <memory>

class Swapchain;

class App {
private:
    std::unique_ptr<Swapchain> swapchain;
};

#endif