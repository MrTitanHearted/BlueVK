#pragma once

#include <BlueVK/Core/Types.hpp>

struct FenceBuilder {
    VkFenceCreateInfo info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    FenceBuilder &set_create_flags(VkFenceCreateFlags flags);
    VkFence build(VkDevice device);
};
struct SemaphoreBuilder {
    VkSemaphoreCreateInfo info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    SemaphoreBuilder &set_create_flags(VkSemaphoreCreateFlags flags);
    VkSemaphore build(VkDevice device);
};