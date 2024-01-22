#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <engine.hpp>

#include <thread>

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
                    case sf::Event::LostFocus:
                        _freezRendering = true;
                        break;
                    case sf::Event::GainedFocus:
                        _freezRendering = false;
                        break;
                    default:
                }
            }
            if (_freezRendering) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            } else if (render) {
                if (_resizeRequested) {
                    resize_swapchain();
                }

                ImGui_ImplVulkan_NewFrame();
                ImGui::NewFrame();

                if (ImGui::Begin("Background")) {
                    ImGui::SliderFloat("Render Scale", &_renderScale, 0.3f, 1.f);

                    ComputeEffect &selected = _computeEffects[_currentComputeEffect];

                    ImGui::Text("Selected effect: %s", selected.name);

                    ImGui::SliderInt("Effect Index", &_currentComputeEffect, 0, _computeEffects.size() - 1);

                    ImGui::InputFloat4("data1", (float *)&selected.data.data1);
                    ImGui::InputFloat4("data2", (float *)&selected.data.data2);
                    ImGui::InputFloat4("data3", (float *)&selected.data.data3);
                    ImGui::InputFloat4("data4", (float *)&selected.data.data4);

                    ImGui::End();
                }

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
        init_descriptors();
        init_pipelines();
    }
    BlueVKEngine::~BlueVKEngine() {
        fmt::println("Destroying BlueVKEngine!");
        vkDeviceWaitIdle(_device);
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
            destroy_draw_images();
            destroy_swapchain();
            vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
            vkDestroyInstance(_instance, nullptr);
        });
    }
    void BlueVKEngine::init_swapchain() {
        create_swapchain(_windowSize);
        create_draw_images();
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
        ImGui_ImplVulkan_CreateFontsTexture();
        _mainDeletionQueue.push_back([&]() {
            //! I think ImGui_ImplVulkan_Shutdown is already
            // vkDestroyDescriptorPool(_device, _imguiPool, nullptr);
            ImGui_ImplVulkan_Shutdown();
            ImGui::SFML::Shutdown();
            ImGui::DestroyContext();
        });
    }
    void BlueVKEngine::init_descriptors() {
        _drawImageDescriptorLayout = DescriptorSetLayoutBuilder{}
                                         .add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                                         .build(_device, VK_SHADER_STAGE_COMPUTE_BIT);

        std::vector<VkDescriptorPoolSize> sizes = {{.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 10}};
        _drawImageDescriptorSet = _mainDescriptorAllocator
                                      .init_pool(_device, 10, sizes)
                                      .allocate(_device, _drawImageDescriptorLayout);

        VkDescriptorImageInfo imgInfo{
            .imageView = _drawImage.view,
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        VkWriteDescriptorSet drawImageWrite{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _drawImageDescriptorSet,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &imgInfo,
        };

        vkUpdateDescriptorSets(_device, 1, &drawImageWrite, 0, nullptr);

        _mainDeletionQueue.push_back([&]() {
            _mainDescriptorAllocator.destroy_pool(_device);
        });
    }
    void BlueVKEngine::init_pipelines() {
        init_pipelines_gradient();
        init_pipelines_triangle();
    }
    void BlueVKEngine::init_pipelines_gradient() {
        VkPipelineLayout pipelineLayout = PipelineLayoutBuilder{}
                                              .add_pc_range(VkPushConstantRange{
                                                  .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                                                  .offset = 0,
                                                  .size = sizeof(glm::vec4) * 4,
                                              })
                                              .add_set_layout(_drawImageDescriptorLayout)
                                              .build(_device);
        VkShaderModule computeShader = load_shader_module(_device, "assets/shaders/gradient_color.comp.spv");
        VkPipeline gradientPipeline = ComputePipelineBuilder{}
                                          .set_layout(pipelineLayout)
                                          .set_shader(computeShader)
                                          .build(_device);
        vkDestroyShaderModule(_device, computeShader, nullptr);

        computeShader = load_shader_module(_device, "assets/shaders/sky.comp.spv");
        VkPipeline skyPipeline = ComputePipelineBuilder{}
                                     .set_layout(pipelineLayout)
                                     .set_shader(computeShader)
                                     .build(_device);
        vkDestroyShaderModule(_device, computeShader, nullptr);

        _computeEffects.push_back(ComputeEffect{
            .name = "Gradient Effect",
            .layout = pipelineLayout,
            .pipeline = gradientPipeline,
            .data = {glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)},
        });
        _computeEffects.push_back(ComputeEffect{
            .name = "Night Sky Effect",
            .layout = pipelineLayout,
            .pipeline = skyPipeline,
            .data = {glm::vec4(0.1, 0.2, 0.4, 0.97)},
        });

        _mainDeletionQueue.push_back([&]() {
            vkDestroyPipelineLayout(_device, _computeEffects[0].layout, nullptr);

            for (uint32_t i = 0; i < _computeEffects.size(); i++) {
                vkDestroyPipeline(_device, _computeEffects[i].pipeline, nullptr);
            }
        });
    }
    void BlueVKEngine::init_pipelines_triangle() {
        VkShaderModule vertShader = load_shader_module(_device, "assets/shaders/triangle.vert.spv");
        VkShaderModule fragShader = load_shader_module(_device, "assets/shaders/triangle.frag.spv");

        _triangleLayout = PipelineLayoutBuilder{}.build(_device);
        _trianglePipeline = GraphicsPipelineBuilder{}
                                .set_layout(_triangleLayout)
                                .set_shaders(vertShader, fragShader)
                                .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                                .set_polygon_mode(VK_POLYGON_MODE_FILL)
                                .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
                                .set_multisampling_none()
                                .disable_blending()
                                .disable_depthtest()
                                .set_color_attachment_format(_drawImage.format)
                                .set_depth_format(VK_FORMAT_UNDEFINED)
                                .build(_device);

        vkDestroyShaderModule(_device, vertShader, nullptr);
        vkDestroyShaderModule(_device, fragShader, nullptr);

        _mainDeletionQueue.push_back([&]() {
            vkDestroyPipelineLayout(_device, _triangleLayout, nullptr);
            vkDestroyPipeline(_device, _trianglePipeline, nullptr);
        });
    }
    void BlueVKEngine::draw() {
        FrameData &frame = get_current_frame();
        VK_CHECK(vkWaitForFences(_device, 1, &frame._renderFence, true, 1000000000));

        uint32_t swapchainImageIndex;
        VkResult nextImageResult = vkAcquireNextImageKHR(_device, _swapchain, 1000000000, frame._swapchainSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);
        if (nextImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
            _resizeRequested = true;
            return;
        } else {
            VK_CHECK(nextImageResult);
        }
        VkImage swapchainImage = _swapchainImages[swapchainImageIndex];
        // _drawExtent = _drawImage.extent;
        _drawExtent.width = std::min(_swapchainExtent.width, _drawImage.extent.width) * _renderScale;
        _drawExtent.height = std::min(_swapchainExtent.height, _drawImage.extent.height) * _renderScale;

        VK_CHECK(vkResetFences(_device, 1, &frame._renderFence));
        VkCommandBuffer cmd = frame._mainCommandBuffer;
        VK_CHECK(vkResetCommandBuffer(cmd, 0));
        VkCommandBufferBeginInfo cmdBeginInfo = command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

        draw_background(cmd);

        transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        draw_geometry(cmd);

        transition_image(cmd, _drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
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
        VkResult presentResult = vkQueuePresentKHR(_graphicsQueue, &presentInfo);
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
            _resizeRequested = true;
            return;
        } else {
            VK_CHECK(presentResult);
        }

        _frameNumber++;
    }
    void BlueVKEngine::draw_background(VkCommandBuffer cmd) {
        ComputeEffect &effect = _computeEffects[_currentComputeEffect];

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, effect.layout, 0, 1, &_drawImageDescriptorSet, 0, nullptr);

        vkCmdPushConstants(cmd, effect.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(effect.data), &effect.data);

        vkCmdDispatch(cmd, std::ceil(_drawExtent.width / 16.0), std::ceil(_drawExtent.height / 16.0), 1);
    }
    void BlueVKEngine::draw_geometry(VkCommandBuffer cmd) {
        VkRenderingAttachmentInfo colorAttachment = attachment_info(_drawImage.view, nullptr, VK_IMAGE_LAYOUT_GENERAL);

        VkRenderingInfo renderInfo = rendering_info(_drawExtent, &colorAttachment, nullptr);
        vkCmdBeginRendering(cmd, &renderInfo);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);

        VkViewport viewport{
            .x = 0,
            .y = 0,
            .width = (float)_drawExtent.width,
            .height = (float)_drawExtent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        VkRect2D scissor{
            .offset = VkOffset2D{0, 0},
            .extent = _drawExtent,
        };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdDraw(cmd, 3, 1, 0, 0);

        vkCmdEndRendering(cmd);
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
                .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
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
    void BlueVKEngine::create_draw_images() {
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
    }
    void BlueVKEngine::resize_swapchain() {
        vkDeviceWaitIdle(_device);
        vkQueueWaitIdle(_graphicsQueue);

        destroy_swapchain();
        destroy_draw_images();

        sf::Vector2u newSize = _window.getSize();

        create_swapchain(VkExtent2D{newSize.x, newSize.y});
        create_draw_images();

        _resizeRequested = false;
    }
    void BlueVKEngine::destroy_swapchain() {
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);

        for (uint32_t i = 0; i < _swapchainImageViews.size(); i++) {
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        }
    }
    void BlueVKEngine::destroy_draw_images() {
        vkDestroyImageView(_device, _drawImage.view, nullptr);
        vmaDestroyImage(_vmaAllocator, _drawImage.image, _drawImage.allocation);
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