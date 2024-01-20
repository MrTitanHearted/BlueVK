#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <engine.hpp>

#include <imgui.h>
#include <imgui-SFML.h>
#include <imgui_impl_vulkan.h>
#include <SFML/Graphics.hpp>
#include <VkBootstrap.h>

#include <vk_builders.hpp>
#include <vk_initializers.hpp>
#include <vk_images.hpp>

namespace bluevk {
    BlueVKEngine *BlueVKEngine::Engine = nullptr;

    BlueVKEngine &BlueVKEngine::getInstance() {
        if (Engine == nullptr) {
            throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Engine is not initialized! Call 'bluevk::BlueVKEngine::Initialize(params)' first.\n"));
        }
        return *Engine;
    }
    void BlueVKEngine::Initialize(BlueVKEngineParams params) {
        if (Engine == nullptr) {
            Engine = new BlueVKEngine{params};
        }
    }
    void BlueVKEngine::Shutdown() {
        delete Engine;
    }
    void BlueVKEngine::run() {
        sf::Clock deltaClock{};
        sf::Event event;
        bool render = true;
        while (_window.isOpen()) {
            float dt = deltaClock.getElapsedTime().asSeconds();
            while (_window.pollEvent(event)) {
                ImGui::SFML::ProcessEvent(event);

                switch (event.type) {
                    case sf::Event::Closed: {
                        _window.close();
                        render = false;
                    } break;
                    case sf::Event::KeyPressed:
                        switch (event.key.code) {
                            case sf::Keyboard::Escape: {
                                _window.close();
                                render = false;
                            } break;
                            default:
                        }
                        break;
                    default:
                }
            }
            if (render) {
                ImGui_ImplVulkan_NewFrame();
                ImGui::NewFrame();

                ImGui::ShowDemoWindow();

                ImGui::EndFrame();
                ImGui::Render();

                draw();
            }
        }
    }
    BlueVKEngine::BlueVKEngine(BlueVKEngineParams &params) {
        fmt::println("Constructoring BlueVKEngine!");
        _windowSize = params.windowSize;
        _windowTitle = params.windowTitle;
        _isResizable = params.isResizable;
        _window.create(sf::VideoMode{_windowSize.width, _windowSize.height},
                       _windowTitle,
                       _isResizable
                           ? sf::Style::Default
                           : sf::Style::Close | sf::Style::Titlebar,
                       sf::ContextSettings(0));
        init_vulkan();
        init_swapchain();
        init_commands();
        init_sync_structures();
        init_imgui();
    }
    BlueVKEngine::~BlueVKEngine() {
        fmt::println("Destroying BlueVKEngine!");
        vkDeviceWaitIdle(_device);
        vkQueueWaitIdle(_graphicsQueue);
        destroy_swapchain();
        _mainDeletionQueue.flush();
    }
    void BlueVKEngine::init_vulkan() {
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
        _window.createVulkanSurface(_instance, _surface);
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
        _mainDeletionQueue.push_back([&]() {
            vmaDestroyAllocator(_vmaAllocator);
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
            vkDestroyDevice(_device, nullptr);
            vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
            vkDestroyInstance(_instance, nullptr);
        });
    }
    void BlueVKEngine::init_swapchain() {
        create_swapchain(_windowSize);
        _drawImage.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        _drawImage.extent = _windowSize;
        VmaAllocationCreateInfo allocCreateInfo{
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags = VkMemoryPropertyFlags{VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
        };
        _drawImage.image = ImageBuilder{}
                               .set_extent(_windowSize)
                               .set_format(_drawImage.format)
                               .set_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                          VK_IMAGE_USAGE_STORAGE_BIT |
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                               .vmaBuild(_vmaAllocator, &allocCreateInfo, &_drawImage.allocation, nullptr);
        _drawImage.view = ImageViewBuilder{}
                              .set_format(_drawImage.format)
                              .set_image(_drawImage.image)
                              .build(_device);

        _mainDeletionQueue.push_back([&]() {
            vkDestroyImageView(_device, _drawImage.view, nullptr);
            vmaDestroyImage(_vmaAllocator, _drawImage.image, _drawImage.allocation);
        });
    }
    void BlueVKEngine::init_commands() {
        CommandPoolBuilder poolBuilder = CommandPoolBuilder{}
                                             .set_create_flags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
                                             .set_queue_family_index(_graphicsQueueIndex);
        for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
            _frames[i]._commandPool = poolBuilder.build(_device);
            _frames[i]._mainCommandBuffer = CommandBufferAllocator{}
                                                .set_command_pool(_frames[i]._commandPool)
                                                .allocate(_device);
        }
        _immCommandPool = poolBuilder.build(_device);
        _immCommandBuffer = CommandBufferAllocator{}
                                .set_command_pool(_immCommandPool)
                                .allocate(_device);
        _mainDeletionQueue.push_back([&]() {
            for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
                vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
            }
            vkDestroyCommandPool(_device, _immCommandPool, nullptr);
        });
    }
    void BlueVKEngine::init_sync_structures() {
        FenceBuilder fenceBuilder = FenceBuilder{}.set_create_flags(VK_FENCE_CREATE_SIGNALED_BIT);
        for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
            _frames[i]._renderFence = fenceBuilder.build(_device);
            _frames[i]._swapchainSemaphore = SemaphoreBuilder{}.build(_device);
            _frames[i]._renderSemaphore = SemaphoreBuilder{}.build(_device);
        }
        _immFence = fenceBuilder.build(_device);
        _mainDeletionQueue.push_back([&]() {
            for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
                vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
                vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
                vkDestroySemaphore(_device, _frames[i]._swapchainSemaphore, nullptr);
            }
            vkDestroyFence(_device, _immFence, nullptr);
        });
    }
    void BlueVKEngine::init_imgui() {
        std::vector<VkDescriptorPoolSize> poolSizes{
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
        };
        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1000,
            .poolSizeCount = (uint32_t)poolSizes.size(),
            .pPoolSizes = poolSizes.data(),
        };
        VkDescriptorPool imguiPool;
        VK_CHECK(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &imguiPool));
        if (!ImGui::SFML::Init(_window)) {
            throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to init ImGui for SFML window!\n"));
        }
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui_ImplVulkan_InitInfo initInfo{.Instance = _instance,
                                           .PhysicalDevice = _physicalDevice,
                                           .Device = _device,
                                           .QueueFamily = _graphicsQueueIndex,
                                           .Queue = _graphicsQueue,
                                           .DescriptorPool = imguiPool,
                                           .MinImageCount = 3,
                                           .ImageCount = 3,
                                           .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
                                           .UseDynamicRendering = true,
                                           .ColorAttachmentFormat = _swapchainImageFormat};

        ImGui_ImplVulkan_Init(&initInfo, VK_NULL_HANDLE);
        // ImGui_ImplVulkan_CreateFontsTexture();
        _mainDeletionQueue.push_back([&]() {
            // vkDestroyDescriptorPool(_device, imguiPool, nullptr);
            ImGui_ImplVulkan_Shutdown();
            ImGui::SFML::Shutdown();
            ImGui::DestroyContext();
        });
    }
    void BlueVKEngine::draw() {
        FrameData &frame = get_current_frame();
        VK_CHECK(vkWaitForFences(_device, 1, &frame._renderFence, true, 1000000000));
        VK_CHECK(vkResetFences(_device, 1, &frame._renderFence));
        uint32_t swapchainImageIndex;
        VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, frame._swapchainSemaphore, VK_NULL_HANDLE, &swapchainImageIndex));
        VkImage swapchainImage = _swapchainImages[swapchainImageIndex];
        VkCommandBuffer cmd = frame._mainCommandBuffer;
        VK_CHECK(vkResetCommandBuffer(cmd, 0));
        VkCommandBufferBeginInfo cmdBeginInfo = command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
        _drawExtent = _drawImage.extent;

        transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        draw_background(cmd);

        transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        transition_image(cmd, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copy_image_to_image(cmd, _drawImage.image, swapchainImage, _drawExtent, _drawExtent);
        transition_image(cmd, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        
        draw_imgui(cmd, _swapchainImageViews[swapchainImageIndex]);
        
        transition_image(cmd, swapchainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        VK_CHECK(vkEndCommandBuffer(cmd));
        VkCommandBufferSubmitInfo cmdInfo = command_buffer_submit_info(cmd);
        VkSemaphoreSubmitInfo waitInfo = semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame._swapchainSemaphore);
        VkSemaphoreSubmitInfo signalInfo = semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame._renderSemaphore);
        VkSubmitInfo2 submit = submit_info(&cmdInfo, &signalInfo, &waitInfo);
        VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, frame._renderFence));
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame._renderSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &_swapchain,
            .pImageIndices = &swapchainImageIndex,
        };
        VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

        _frameNumber++;
    }
    void BlueVKEngine::draw_background(VkCommandBuffer cmd) {
        float flash = std::abs(std::sin(_frameNumber / 120.0f));
        VkClearColorValue clearValue{0.0f, 0.0f, flash, 1.0f};
        VkImageSubresourceRange clearRange = image_subresource_range();
        vkCmdClearColorImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
    }
    void BlueVKEngine::draw_imgui(VkCommandBuffer cmd, VkImageView view) {
        VkRenderingAttachmentInfo colorAttachment = attachment_info(view, nullptr, VK_IMAGE_LAYOUT_GENERAL);
        VkRenderingInfo renderInfo = rendering_info(_swapchainExtent, &colorAttachment, nullptr);
        vkCmdBeginRendering(cmd, &renderInfo);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        vkCmdEndRendering(cmd);
    }
    void BlueVKEngine::create_swapchain(VkExtent2D size) {
        _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

        vkb::Result<vkb::Swapchain> swapchainReturn =
            vkb::SwapchainBuilder{_physicalDevice, _device, _surface}
                .set_desired_format(VkSurfaceFormatKHR{.format = _swapchainImageFormat,
                                                       .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                .set_desired_extent(size.width, size.height)
                .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                .build();
        if (!swapchainReturn.has_value()) {
            throw std::runtime_error(fmt::format("[BLueVK]::[ERROR]: Failed to create a swapchain:\n{}", swapchainReturn.error().value()));
        }
        vkb::Swapchain vkbSwapchain = swapchainReturn.value();

        _swapchain = vkbSwapchain;
        _swapchainExtent = vkbSwapchain.extent;
        _swapchainImages = vkbSwapchain.get_images().value();
        _swapchainImageViews = vkbSwapchain.get_image_views().value();
    }
    void BlueVKEngine::destroy_swapchain() {
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);

        for (uint32_t i = 0; i < _swapchainImageViews.size(); i++) {
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        }
    }
    void BlueVKEngine::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function) {
        VK_CHECK(vkResetFences(_device, 1, &_immFence));
        VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));
        VkCommandBuffer cmd = _immCommandBuffer;
        VkCommandBufferBeginInfo cmdBeginInfo = command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        function(cmd);

        VK_CHECK(vkEndCommandBuffer(cmd));
        VkCommandBufferSubmitInfo cmdinfo = command_buffer_submit_info(cmd);
        VkSubmitInfo2 submit = submit_info(&cmdinfo, nullptr, nullptr);
        VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, _immFence));
        VK_CHECK(vkWaitForFences(_device, 1, &_immFence, true, 9999999999));
    }

    void BlueVKEngine::DeletionQueue::push_back(std::function<void()> &&function) {
        deletors.push_back(function);
    }
    void BlueVKEngine::DeletionQueue::flush() {
        for (std::deque<std::function<void()>>::reverse_iterator it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)();
        }
        deletors.clear();
    }
}  // namespace bluevk