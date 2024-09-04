#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>
#include <optional>

#include <string.h>

#define WIDTH 500
#define HEIGHT 500

class App {
    struct QueueFamilyIndicies
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };
    
    public:
        GLFWwindow *win;
        VkInstance instance;
        const std::vector<const char *> validationLayer {"VK_LAYER_KHRONOS_validation"};
        #ifdef DEBUG
            const bool enableValidationLayers = true;
        #else
            const bool enableValidationLayers = false;
        #endif

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSurfaceKHR surface;

        void run()
        {
            init_window();
            init_vk();
            loop();
            clean();
        }
    private:
        bool check_validation_layer_support()
        {
            uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> layers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
            for (auto& layer : validationLayer)
            {
                bool found = false;
                for (auto& available : layers)
                {
                    if (strcmp(available.layerName, layer) == 0)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            return true;
            
        }

        void createSurface()
        {
            if (glfwCreateWindowSurface(instance, win, nullptr, &surface) != VK_SUCCESS)
                throw std::runtime_error("Failed to create surface");
        }

        void makeLogicalDevice()
        {
            QueueFamilyIndicies queues = findQueueFamilyIndicies(physicalDevice);
            std::set<uint32_t> uniquequeues{queues.graphicsFamily.value(), queues.presentFamily.value()}; 

            
            float queuePrio = 1.0f;
            std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos{};
            for (auto& queue : uniquequeues)
            {
                VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
                deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                deviceQueueCreateInfo.queueCount = 1;
                deviceQueueCreateInfo.queueFamilyIndex = queue;
                deviceQueueCreateInfo.pQueuePriorities = &queuePrio;
                deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
            }


            VkPhysicalDeviceFeatures deviceFeatures{}; 

            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
            deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
            deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
            deviceCreateInfo.enabledExtensionCount = 0;

            if (enableValidationLayers) {
                deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayer.size());
                deviceCreateInfo.ppEnabledLayerNames = validationLayer.data();
            } else {
                deviceCreateInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device ) != VK_SUCCESS)
                throw std::runtime_error("Failed to create logical device");
            
            vkGetDeviceQueue(device, queues.graphicsFamily.value(), 0, &graphicsQueue);
            vkGetDeviceQueue(device, queues.presentFamily.value(), 0, &presentQueue);
        }

        QueueFamilyIndicies findQueueFamilyIndicies(VkPhysicalDevice dev)
        {
            QueueFamilyIndicies indicies;

            uint32_t deviceQueueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(dev, &deviceQueueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(deviceQueueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(dev, &deviceQueueFamilyCount, families.data());
            int index = 0;
            for (auto& fam : families)
            {
                if (fam.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indicies.graphicsFamily = index;
                VkBool32 isPresentQueue = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &isPresentQueue);
                if (isPresentQueue)
                    indicies.presentFamily = index;
                index++;
            }
            return indicies;
        }

        void pickPhysicalDevice()
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount == 0)
                throw std::runtime_error("No suitable device");
            std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
            
            physicalDevice = physicalDevices[0];
        }

        void init_window()
        {
            std::cout << "=====  Window init  =====\n";
            glfwInit();
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            win = glfwCreateWindow(WIDTH, HEIGHT, "FUNny", nullptr, nullptr);
        }
        void init_vk()
        {
            std::cout << "=====  Vulkan init  =====\n";
            /* Validation layer */
            if (enableValidationLayers && !check_validation_layer_support())
                throw std::runtime_error("Failed to init validation layers");
            /* vulkan instance*/
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.apiVersion = VK_API_VERSION_1_0;
            appInfo.pApplicationName = "Tardation";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);

            VkInstanceCreateInfo instanceInfo{};
            instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceInfo.pApplicationInfo = &appInfo;

            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            instanceInfo.enabledExtensionCount = glfwExtensionCount;
            instanceInfo.ppEnabledExtensionNames = glfwExtensions;

            if (enableValidationLayers)
            {
                instanceInfo.enabledLayerCount = validationLayer.size();
                instanceInfo.ppEnabledLayerNames = validationLayer.data();
            }
            else
                instanceInfo.enabledLayerCount = 0;

            if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
                throw std::runtime_error("Failed to create instance");

            /* extensions */
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            if (extensionCount != 0)
            {
                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
                std::cout << "Availiable Extensions: \n";
                for (auto& ext : extensions)
                    std::cout << "\t" << ext.extensionName << "\n";
            }

            /* Surface and pipeline */
            createSurface();

            /* device selection and creation */
            pickPhysicalDevice();
            makeLogicalDevice();


        }
        void loop()
        {
            std::cout << "=====  LOOP'O CLOCK  =====\n";
            while(!glfwWindowShouldClose(win))
                glfwPollEvents();
        }
        void clean()
        {
            std::cout << "=====  CLEANUP  =====\n";

            vkDestroyDevice(device, nullptr);
            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);
            glfwDestroyWindow(win);
            glfwTerminate();
        }
};

int main()
{
    App app;

    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(69);
    }
}