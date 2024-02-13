#pragma once

#include <BlueVK/Core/Types.hpp>

struct Frame {
    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;

    VkFence _renderFence;
    VkSemaphore _swapchainSemaphore;
    VkSemaphore _renderSemaphore;
};