#pragma once

#include <types.hpp>

namespace bluevk {
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

    void copy_image_to_image(VkCommandBuffer cmd,
                             VkImage source,
                             VkImage destination,
                             VkExtent2D srcSize,
                             VkExtent2D dstSize);
}  // namespace bluevk