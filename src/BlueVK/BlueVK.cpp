#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <BlueVK/BlueVK.hpp>
#include <BlueVK/Platform/Platform.hpp>

#include <thread>

#include <VkBootstrap.h>

BlueVK *BlueVK::blueVK = nullptr;

void BlueVK::Init(BlueVKParams params) {
    if (blueVK == nullptr) {
        blueVK = new BlueVK{params};
    }
}

void BlueVK::Shutdown() {
    delete blueVK;
    blueVK = nullptr;
}

BlueVK::BlueVK(BlueVKParams params) {
    fmt::println("[BlueVK]: BlueVK Init");
    _windowSize = params.windowSize;
    _windowTitle = params.windowTitle;
    _isResizable = params.isResizable;

    init_vulkan(params.windowHandle);
}

BlueVK::~BlueVK() {
    fmt::println("[BlueVK]: BlueVK Shutdown");

    vkDeviceWaitIdle(_device);
    _mainDeletionQueue.flush();
}

void BlueVK::init_vulkan(void *windowHandle) {
    vkb::Result<vkb::Instance> instanceReturn =
        vkb::InstanceBuilder{}
            .set_app_name(_windowTitle.c_str())
            .set_engine_name("BlueVK Engine")
            .require_api_version(1, 3, 0)
            .request_validation_layers(true)
            .use_default_debug_messenger()
            .build();
    if (!instanceReturn.has_value()) {
        throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to create a vulkan instance!"));
    }
    vkb::Instance vkbInstance = instanceReturn.value();
    _instance = vkbInstance;
    _debugMessenger = vkbInstance.debug_messenger;
    _surface = BlueVKCreateSurface(_instance, windowHandle);
    VkPhysicalDeviceVulkan13Features features13{
        .synchronization2 = true,
        .dynamicRendering = true,
    };
    VkPhysicalDeviceVulkan12Features features12{
        .descriptorIndexing = true,
        .bufferDeviceAddress = true,
    };
    vkb::Result<vkb::PhysicalDevice> physicalDeviceReturn =
        vkb::PhysicalDeviceSelector{vkbInstance}
            .set_minimum_version(1, 3)
            .set_required_features_13(features13)
            .set_required_features_12(features12)
            .set_surface(_surface)
            .select();
    if (!physicalDeviceReturn.has_value()) {
        throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to select compatiable GPU:\n{}",
                                             physicalDeviceReturn.error().message()));
    }
    vkb::PhysicalDevice vkbPhysicalDevice = physicalDeviceReturn.value();
    vkb::Result<vkb::Device> deviceReturn = vkb::DeviceBuilder{vkbPhysicalDevice}.build();
    if (!deviceReturn.has_value()) {
        throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to build device:\n{}",
                                             deviceReturn.error().message()));
    }
    vkb::Device vkbDevice = deviceReturn.value();
    _physicalDevice = vkbPhysicalDevice;
    _device = vkbDevice;
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    VmaAllocatorCreateInfo allocatorInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = _physicalDevice,
        .device = _device,
        .instance = _instance,
    };
    vmaCreateAllocator(&allocatorInfo, &_vmaAllocator);

    _mainDeletionQueue.push_back([&]() {
        vmaDestroyAllocator(_vmaAllocator);
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);
        vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
        vkDestroyInstance(_instance, nullptr);
    });
}
void BlueVK::init_swapchain() {
    create_swapchain(_windowSize);
    create_draw_images();

    _mainDeletionQueue.push_back([&]() {
        destroy_swapchain();
        destroy_draw_images();
    });
}

void BlueVK::create_swapchain(VkExtent2D size) {
    _windowSize = size;
    _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Result<vkb::Swapchain> swapchainReturn =
        vkb::SwapchainBuilder{_physicalDevice, _device, _surface}
            .set_desired_format(VkSurfaceFormatKHR{.format = _swapchainImageFormat,
                                                   .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
            .set_desired_extent(size.width, size.height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build();
    if (!swapchainReturn.has_value()) {
        throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to build swapchain:\n{}",
                                             swapchainReturn.error().message()));
    }
    vkb::Swapchain vkbSwapchain = swapchainReturn.value();

    _swapchain = vkbSwapchain;
    _swapchainExtent = vkbSwapchain.extent;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void BlueVK::create_draw_images() {
    VmaAllocationCreateInfo allocCreateInfo = VmaAllocationCreateInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags{VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
    };
    _drawImage = ImageAllocator{}
                     .set_format(VK_FORMAT_R16G16B16A16_SFLOAT)
                     .set_extent(_windowSize)
                     .set_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                VK_IMAGE_USAGE_STORAGE_BIT |
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                     .allocate(_device, _vmaAllocator, &allocCreateInfo);
}

void BlueVK::destroy_swapchain() {
    for (VkImageView imageView : _swapchainImageViews) {
        vkDestroyImageView(_device, imageView, nullptr);
    }
    for (VkImage image : _swapchainImages) {
        vkDestroyImage(_device, image, nullptr);
    }
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

void BlueVK::destroy_draw_images() {
    vkDestroyImageView(_device, _drawImage.view, nullptr);
    vmaDestroyImage(_vmaAllocator, _drawImage.image, _drawImage.allocation);
}
