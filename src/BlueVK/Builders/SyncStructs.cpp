#include <BlueVK/Builders/SyncStructs.hpp>

FenceBuilder& FenceBuilder::set_create_flags(VkFenceCreateFlags flags) {
    info.flags = flags;
    return *this;
}
VkFence FenceBuilder::build(VkDevice device) {
    VkFence fence;
    VK_CHECK(vkCreateFence(device, &info, nullptr, &fence));
    return fence;
}
SemaphoreBuilder& SemaphoreBuilder::set_create_flags(VkSemaphoreCreateFlags flags) {
    info.flags = flags;
    return *this;
}
VkSemaphore SemaphoreBuilder::build(VkDevice device) {
    VkSemaphore semaphore;
    VK_CHECK(vkCreateSemaphore(device, &info, nullptr, &semaphore));
    return semaphore;
}