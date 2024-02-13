#include <BlueVK/Builders/Image.hpp>

ImageBuilder& ImageBuilder::set_format(VkFormat format) {
    info.format = format;
    return *this;
}
ImageBuilder& ImageBuilder::set_extent(VkExtent2D extent) {
    info.extent = {extent.width, extent.height, 1};
    return *this;
}
ImageBuilder& ImageBuilder::set_usage(VkImageUsageFlags usage) {
    info.usage = usage;
    return *this;
}
VkImage ImageBuilder::build(VkDevice device) {
    VkImage image;
    VK_CHECK(vkCreateImage(device, &info, nullptr, &image));
    return image;
}
VkImage ImageBuilder::vmaBuild(VmaAllocator allocator, VmaAllocationCreateInfo* allocCreateInfo, VmaAllocation* alloc, VmaAllocationInfo* allocInfo) {
    VkImage image;
    vmaCreateImage(allocator, &info, allocCreateInfo, &image, alloc, allocInfo);
    return image;
}
ImageViewBuilder& ImageViewBuilder::set_image(VkImage image) {
    info.image = image;
    return *this;
}
ImageViewBuilder& ImageViewBuilder::set_format(VkFormat format) {
    info.format = format;
    return *this;
}
ImageViewBuilder& ImageViewBuilder::set_subresource_range_aspect(VkImageAspectFlags aspectMask) {
    info.subresourceRange.aspectMask = aspectMask;
    return *this;
}
VkImageView ImageViewBuilder::build(VkDevice device) {
    VkImageView view;
    VK_CHECK(vkCreateImageView(device, &info, nullptr, &view));
    return view;
}
ImageAllocator& ImageAllocator::set_format(VkFormat format) {
    imageInfo.format = format;
    viewInfo.format = format;
    return *this;
}
ImageAllocator& ImageAllocator::set_extent(VkExtent2D extent) {
    imageInfo.extent = VkExtent3D{extent.width, extent.height, 1};
    return *this;
}
ImageAllocator& ImageAllocator::set_usage(VkImageUsageFlags usage) {
    imageInfo.usage = usage;
    return *this;
}
ImageAllocator& ImageAllocator::set_subresource_range_aspect(VkImageAspectFlags aspectMask) {
    viewInfo.subresourceRange.aspectMask = aspectMask;
    return *this;
}
BlueVKImage ImageAllocator::allocate(VkDevice device,
                                     VmaAllocator allocator,
                                     VmaAllocationCreateInfo* allocCreateInfo) {
    BlueVKImage image{
        .extent = {imageInfo.extent.width, imageInfo.extent.height},
        .format = imageInfo.format,
    };

    vmaCreateImage(allocator,
                   &imageInfo,
                   allocCreateInfo,
                   &image.image,
                   &image.allocation,
                   &image.allocationInfo);

    VK_CHECK(vkCreateImageView(device, &viewInfo, nullptr, &image.view));

    return image;
}
