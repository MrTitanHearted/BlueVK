#pragma once

#include <BlueVK/Core/Types.hpp>

struct CommandPoolBuilder {
    VkCommandPoolCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    CommandPoolBuilder &set_queue_family_index(uint32_t queueFamilyIndex);
    CommandPoolBuilder &set_create_flags(VkCommandPoolCreateFlags flags);
    VkCommandPool build(VkDevice device);
};
struct CommandBufferAllocator {
    VkCommandBufferAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    CommandBufferAllocator &set_command_pool(VkCommandPool pool);
    CommandBufferAllocator &set_level(VkCommandBufferLevel level);
    VkCommandBuffer allocate(VkDevice device);
};