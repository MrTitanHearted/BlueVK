#include <BlueVK/Builders/CommandBuffer.hpp>

CommandPoolBuilder& CommandPoolBuilder::set_queue_family_index(uint32_t queueFamilyIndex) {
    info.queueFamilyIndex = queueFamilyIndex;
    return *this;
}
CommandPoolBuilder& CommandPoolBuilder::set_create_flags(VkCommandPoolCreateFlags flags) {
    info.flags = flags;
    return *this;
}
VkCommandPool CommandPoolBuilder::build(VkDevice device) {
    VkCommandPool pool;
    VK_CHECK(vkCreateCommandPool(device, &info, nullptr, &pool));
    return pool;
}
CommandBufferAllocator& CommandBufferAllocator::set_command_pool(VkCommandPool pool) {
    info.commandPool = pool;
    return *this;
}
CommandBufferAllocator& CommandBufferAllocator::set_level(VkCommandBufferLevel level) {
    info.level = level;
    return *this;
}
VkCommandBuffer CommandBufferAllocator::allocate(VkDevice device) {
    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(device, &info, &cmd));
    return cmd;
}