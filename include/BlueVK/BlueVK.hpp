#pragma once

#include <BlueVK/Core/Types.hpp>
#include <BlueVK/Core/DeletionQueue.hpp>
#include <BlueVK/Core/Frame.hpp>
#include <BlueVK/Builders/Image.hpp>

constexpr uint32_t FRAME_OVERLAP = 3;

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
    VkSwapchainKHR _swapchain;
    VkFormat _swapchainImageFormat;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    VkExtent2D _swapchainExtent;
    Frame _frames[FRAME_OVERLAP];
    size_t _frameNumber;
    BlueVKImage _drawImage;
    VkExtent2D _drawExtent;

    DeletionQueue _mainDeletionQueue;

    BlueVK(BlueVKParams params);
    ~BlueVK();

    void init_vulkan(void *windowHandle);
    void init_swapchain();

    void create_swapchain(VkExtent2D size);
    void create_draw_images();

    void destroy_swapchain();
    void destroy_draw_images();

    Frame &get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; }
};
