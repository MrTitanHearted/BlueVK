#pragma once

#include <BlueVK/Core/Types.hpp>

struct BlueVKParams {
    std::string windowTitle;
    VkExtent2D windowSize;
    bool isResizable;
    void *windowHandle;
};

class BlueVK {
   public:
    BlueVK(const BlueVK &) = delete;
    BlueVK &operator=(const BlueVK &) = delete;

    static void Init(BlueVKParams params);

    static void Shutdown();

   private:
    static BlueVK *blueVK;

    VkExtent2D _windowSize;
    std::string _windowTitle;
    bool _isResizable;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkSurfaceKHR _surface;
    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueIndex;
    VmaAllocator _vmaAllocator;

    BlueVK(BlueVKParams params);
    ~BlueVK();

    void init_vulkan(void *windowHandle);
};
