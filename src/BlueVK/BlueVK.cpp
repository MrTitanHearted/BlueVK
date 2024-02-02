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

    vmaDestroyAllocator(_vmaAllocator);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_device, nullptr);
    vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
    vkDestroyInstance(_instance, nullptr);
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
                                             physicalDeviceReturn.error().value()));
    }
    vkb::PhysicalDevice vkbPhysicalDevice = physicalDeviceReturn.value();
    vkb::Result<vkb::Device> deviceReturn = vkb::DeviceBuilder{vkbPhysicalDevice}.build();
    if (!deviceReturn.has_value()) {
        throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to build device:\n{}",
                                             deviceReturn.error().value()));
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
}
