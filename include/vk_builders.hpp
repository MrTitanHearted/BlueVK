#pragma once

#include <types.hpp>

namespace bluevk {
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
    struct ImageBuilder {
        VkImageCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .imageType = VK_IMAGE_TYPE_2D,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
        };
        ImageBuilder &set_format(VkFormat format);
        ImageBuilder &set_extent(VkExtent2D extent);
        ImageBuilder &set_usage(VkImageUsageFlags usage);
        VkImage build(VkDevice device);
        VkImage vmaBuild(VmaAllocator allocator, VmaAllocationCreateInfo *allocCreateInfo, VmaAllocation *alloc, VmaAllocationInfo *allocInfo);
    };
    struct ImageViewBuilder {
        VkImageViewCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        ImageViewBuilder &set_image(VkImage image);
        ImageViewBuilder &set_format(VkFormat format);
        ImageViewBuilder &set_subresource_range_aspect(VkImageAspectFlags aspectMask);
        VkImageView build(VkDevice device);
    };
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
    struct DescriptorSetLayoutBuilder {
        std::vector<VkDescriptorSetLayoutBinding> bindings{};
        DescriptorSetLayoutBuilder &add_binding(uint32_t binding, VkDescriptorType type);
        DescriptorSetLayoutBuilder &clear();
        VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages);
    };
    struct DescriptorSetAllocator {
        VkDescriptorPool pool;
        DescriptorSetAllocator &init_pool(VkDevice device, uint32_t maxSets, std::vector<VkDescriptorPoolSize> poolRatios);
        DescriptorSetAllocator &clear(VkDevice device);
        void destroy_pool(VkDevice device);
        VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
    };
}  // namespace bluevk