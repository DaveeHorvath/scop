#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>
#include <optional>
#include <fstream>

#include <string.h>

#define WIDTH 500
#define HEIGHT 500

static std::vector<char> readShader(const std::string& filename) 
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

class App {
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

    
    public:
        GLFWwindow *win;
        VkInstance instance;
        const std::vector<const char *> validationLayer {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        #ifdef DEBUG
            const bool enableValidationLayers = true;
        #else
            const bool enableValidationLayers = false;
        #endif

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapchain;
        VkSurfaceKHR surface;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;

        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImagesViews;

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;

        VkPipeline graphicsPipeline;
        std::vector<VkFramebuffer> swapchainFramebuffers;

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

        VkShaderModule makeShaderModule(const std::vector<char>& shader)
        {
            VkShaderModuleCreateInfo shaderModuleCreateInfo{};
            shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderModuleCreateInfo.codeSize = shader.size();
            shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shader.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
                throw std::runtime_error("Failed to create shadermodule");
            return shaderModule;
        }

        void createSurface()
        {
            if (glfwCreateWindowSurface(instance, win, nullptr, &surface) != VK_SUCCESS)
                throw std::runtime_error("Failed to create surface");
        }

        void makeFrameBuffer()
        {
            swapchainFramebuffers.resize(swapchainImagesViews.size());

            for (int i = 0; i < swapchainImagesViews.size(); i++)
            {
                VkImageView imageViews[] = {swapchainImagesViews[i]};

                VkFramebufferCreateInfo framebufferCreateInfo{};
                framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferCreateInfo.renderPass = renderPass;
                framebufferCreateInfo.attachmentCount = 1;
                framebufferCreateInfo.pAttachments = imageViews;
                framebufferCreateInfo.height = swapchainExtent.height;
                framebufferCreateInfo.width = swapchainExtent.width;
                framebufferCreateInfo.layers = 1;

                if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create framebuffers");
            }
        }

        void makeRenderPass()
        {
            VkAttachmentDescription attachmentDescription{};
            attachmentDescription.format = swapchainImageFormat;
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference attachmentReference{};
            attachmentReference.attachment = 0;
            attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpassDescription{};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.pColorAttachments = &attachmentReference;


            VkRenderPassCreateInfo renderPassCreateInfo{};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassCreateInfo.attachmentCount = 1;
            renderPassCreateInfo.pAttachments = &attachmentDescription;
            renderPassCreateInfo.subpassCount = 1;
            renderPassCreateInfo.pSubpasses = &subpassDescription;

            if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
                throw std::runtime_error("Failed to create renderpass");
        }

        void makePipeline()
        {
            std::vector<char> vertexShader = readShader("vert.spv");
            std::vector<char> fragmentShader = readShader("frag.spv");

            VkShaderModule vertexShaderModule = makeShaderModule(vertexShader);
            VkShaderModule fragmentShaderModule = makeShaderModule(fragmentShader);

            VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
            vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertexShaderStageCreateInfo.module = vertexShaderModule;
            vertexShaderStageCreateInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
            fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragmentShaderStageCreateInfo.module = fragmentShaderModule;
            fragmentShaderStageCreateInfo.pName = "main";

            VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

            /* HALF ASLEEP PLEASE REVIEW TMRW FOR REASONS, MOSTLY OUT OF TEXTBOOK */
            /* Viewport setup with scisor and the viewport itself */
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
            pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
            pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;

            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
            pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapchainExtent.width;
            viewport.height = (float) swapchainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapchainExtent;

            VkPipelineViewportStateCreateInfo pipelineViewpoerStateCreateInfo{};
            pipelineViewpoerStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            pipelineViewpoerStateCreateInfo.scissorCount = 1;
            pipelineViewpoerStateCreateInfo.pScissors = &scissor;
            pipelineViewpoerStateCreateInfo.viewportCount = 1;
            pipelineViewpoerStateCreateInfo.pViewports = &viewport;

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
            pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

            /* rasterizer */
            VkPipelineRasterizationStateCreateInfo pipelineRasterizationCreateInfo{};
            pipelineRasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            pipelineRasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
            pipelineRasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
            pipelineRasterizationCreateInfo.lineWidth = 1.0f;
            pipelineRasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            pipelineRasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
            pipelineRasterizationCreateInfo.depthBiasEnable = VK_FALSE;

            /* JUST TAKEN, all defaults, used to antialias*/
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
            pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
            pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
            pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
            pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
            pipelineColorBlendStateCreateInfo.attachmentCount = 1;
            pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

            if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("Failed to create pipeline layout");

            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
            graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            graphicsPipelineCreateInfo.stageCount = 2;
            graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfo;
            graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
            graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
            graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
            graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationCreateInfo;
            graphicsPipelineCreateInfo.layout = pipelineLayout;
            graphicsPipelineCreateInfo.pViewportState = &pipelineViewpoerStateCreateInfo;
            graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
            graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
            graphicsPipelineCreateInfo.renderPass = renderPass;
            graphicsPipelineCreateInfo.subpass = 0;
            graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            graphicsPipelineCreateInfo.basePipelineIndex = -1;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
                throw std::runtime_error("Failed to create pipeline");

            vkDestroyShaderModule(device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
        }

        void makeSwapChain()
        {
            SwapChainSupportDetails details = findSwapChainSupportDetails(physicalDevice);

            VkSurfaceFormatKHR surfaceFormat = pickSurfaceFormat(details.formats);
            VkSwapchainCreateInfoKHR swapchainCreateInfo{};
            swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swapchainCreateInfo.surface = surface;
            swapchainCreateInfo.minImageCount = details.capabilities.maxImageCount > 0 ? std::min(details.capabilities.minImageCount + 1, details.capabilities.maxImageCount) : details.capabilities.minImageCount + 1;

            swapchainImageFormat = surfaceFormat.format;
            swapchainCreateInfo.imageFormat = swapchainImageFormat;
            swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;

            swapchainExtent = pickSwapChainExtent(details.capabilities);
            swapchainCreateInfo.imageExtent = swapchainExtent;
            swapchainCreateInfo.imageArrayLayers = 1;
            swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            swapchainCreateInfo.presentMode = pickSwapPresentMode(details.modes);

            QueueFamilyIndicies indicies = findQueueFamilyIndicies(physicalDevice);
            uint32_t queueFamilyIndices[] = {indicies.graphicsFamily.value(), indicies.presentFamily.value()};

            if (indicies.graphicsFamily != indicies.presentFamily) 
            {
                swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                swapchainCreateInfo.queueFamilyIndexCount = 2;
                swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else
                swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

            swapchainCreateInfo.preTransform = details.capabilities.currentTransform;
            swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            swapchainCreateInfo.clipped = VK_TRUE;
            swapchainCreateInfo.oldSwapchain = nullptr;


            if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
                throw std::runtime_error("Failed to create swapchain");

            uint32_t swapchainImageCount = 0;

            /* Make swapchain images */
            vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
            if (swapchainImageCount)
            {
                swapchainImages.resize(swapchainImageCount);
                vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
            }
            /* Make swapchain image views */
            swapchainImagesViews.resize(swapchainImages.size());
            for (int i = 0; i < swapchainImages.size(); i++)
            {
                VkImageViewCreateInfo imageViewCreateInfo{};
                imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                imageViewCreateInfo.image = swapchainImages[i];

                imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                imageViewCreateInfo.format = swapchainImageFormat;

                imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
                imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
                imageViewCreateInfo.subresourceRange.levelCount = 1;
                imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
                imageViewCreateInfo.subresourceRange.layerCount = 1;
                if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImagesViews[i]) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create Swapchain Image Views");
            }

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


            deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

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

        SwapChainSupportDetails findSwapChainSupportDetails(VkPhysicalDevice dev)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &details.capabilities);

            uint32_t formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, nullptr);
            if (formatCount)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, details.formats.data());
            }

            uint32_t modesCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &modesCount, nullptr);
            if (modesCount)
            {
                details.modes.resize(modesCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &modesCount, details.modes.data());
            }

            return details;
        }

        VkSurfaceFormatKHR pickSurfaceFormat(const std::vector<VkSurfaceFormatKHR> formats)
        {
            for (const auto& format : formats) 
            {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return format;
            }
            return formats[0];
        }

        VkSurfaceFormatKHR pickSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) 
        {
            for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return format;
                }
            }
            return formats[0];
        }

        VkPresentModeKHR pickSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
        {
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D pickSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
        {
            return capabilities.currentExtent;
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

            /* Surface*/
            createSurface();

            /* device selection and creation */
            pickPhysicalDevice();
            makeLogicalDevice();

            /* SwapChain */
            makeSwapChain();

            /* pipeline */
            makeRenderPass();
            makePipeline();

            /* Framebuffer */
            makeFrameBuffer();
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
            for (auto& buffer : swapchainFramebuffers)
                vkDestroyFramebuffer(device, buffer, nullptr);
            vkDestroyPipeline(device, graphicsPipeline, nullptr);
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            vkDestroyRenderPass(device, renderPass, nullptr);
            for (auto& imageView : swapchainImagesViews)
                vkDestroyImageView(device, imageView, nullptr);
            vkDestroySwapchainKHR(device, swapchain, nullptr);
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


