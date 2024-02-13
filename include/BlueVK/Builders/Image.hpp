#pragma once

#include <BlueVK/Core/Types.hpp>

struct BlueVKImage {
    VkImage image;
    VkImageView view;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkExtent2D extent;
    VkFormat format;
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
struct ImageAllocator {
    VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .imageType = VK_IMAGE_TYPE_2D,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
    };
    VkImageViewCreateInfo viewInfo{
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

    ImageAllocator &set_format(VkFormat format);
    ImageAllocator &set_extent(VkExtent2D extent);
    ImageAllocator &set_usage(VkImageUsageFlags usage);
    ImageAllocator &set_subresource_range_aspect(VkImageAspectFlags aspectMask);
    BlueVKImage allocate(VkDevice device, VmaAllocator allocator, VmaAllocationCreateInfo *allocCreateInfo);
};