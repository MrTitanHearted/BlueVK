#pragma once

#include <SFML/Graphics.hpp>

#include <types.hpp>
#include <vk_builders.hpp>
#include <vk_pipelines.hpp>

constexpr uint32_t FRAME_OVERLAP = 2;

namespace bluevk {
    struct BlueVKEngineParams {
        VkExtent2D windowSize = {1700, 1000};
        std::string windowTitle = "BlueVK Engine";
        bool isResizable = false;
    };

    class BlueVKEngine {
       public:
        BlueVKEngine(const BlueVKEngine &) = delete;
        BlueVKEngine &operator=(const BlueVKEngine &) = delete;

        static BlueVKEngine &getInstance();
        static void Initialize(BlueVKEngineParams params);
        static void Shutdown();

        void run();

       private:
        struct DeletionQueue {
            std::deque<std::function<void()>> deletors;
            void push_back(std::function<void()> &&function);
            void flush();
        };
        struct FrameData {
            VkCommandPool _commandPool;
            VkCommandBuffer _mainCommandBuffer;
            VkFence _renderFence;
            VkSemaphore _swapchainSemaphore;
            VkSemaphore _renderSemaphore;
        };

        static BlueVKEngine *Engine;

        DeletionQueue _mainDeletionQueue;
        VkExtent2D _windowSize;
        std::string _windowTitle;
        bool _isResizable;
        sf::RenderWindow _window;
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
        FrameData _frames[FRAME_OVERLAP];
        size_t _frameNumber;
        BlueVKImage _drawImage;
        VkExtent2D _drawExtent;
        DescriptorSetAllocator _mainDescriptorAllocator;
        VkFence _immFence;
        VkCommandPool _immCommandPool;
        VkCommandBuffer _immCommandBuffer;

        BlueVKEngine(BlueVKEngineParams &params);
        ~BlueVKEngine();

        void init_vulkan();
        void init_swapchain();
        void init_commands();
        void init_sync_structures();
        void init_imgui();

        void draw();
        void draw_background(VkCommandBuffer cmd);
        void draw_imgui(VkCommandBuffer cmd, VkImageView view);

        void create_swapchain(VkExtent2D size);
        void destroy_swapchain();

        FrameData &get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; }

        void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);
    };
}  // namespace bluevk