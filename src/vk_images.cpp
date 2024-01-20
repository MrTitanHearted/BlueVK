#include <vk_images.hpp>

namespace bluevk {
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
        VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                                            ? VK_IMAGE_ASPECT_DEPTH_BIT
                                            : VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageMemoryBarrier2 imageBarrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                           .pNext = nullptr,
                                           .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                           .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                                           .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                           .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
                                           .oldLayout = currentLayout,
                                           .newLayout = newLayout,
                                           .image = image,
                                           .subresourceRange = VkImageSubresourceRange{
                                               .aspectMask = aspectMask,
                                               .baseMipLevel = 0,
                                               .levelCount = VK_REMAINING_MIP_LEVELS,
                                               .baseArrayLayer = 0,
                                               .layerCount = VK_REMAINING_ARRAY_LAYERS,
                                           }};
        VkDependencyInfo depInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                 .pNext = nullptr,
                                 .imageMemoryBarrierCount = 1,
                                 .pImageMemoryBarriers = &imageBarrier};
        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

    void copy_image_to_image(VkCommandBuffer cmd,
                             VkImage source,
                             VkImage destination,
                             VkExtent2D srcSize,
                             VkExtent2D dstSize) {
        VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                                .pNext = nullptr,
                                .srcSubresource = VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                                           .mipLevel = 0,
                                                                           .baseArrayLayer = 0,
                                                                           .layerCount = 1},
                                .dstSubresource = VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                                           .mipLevel = 0,
                                                                           .baseArrayLayer = 0,
                                                                           .layerCount = 1}};
        blitRegion.srcOffsets[1] = VkOffset3D{.x = (int32_t)srcSize.width,
                                              .y = (int32_t)srcSize.height,
                                              .z = 1};
        blitRegion.dstOffsets[1] = VkOffset3D{.x = (int32_t)dstSize.width,
                                              .y = (int32_t)dstSize.height,
                                              .z = 1};

        VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                                  .pNext = nullptr,
                                  .srcImage = source,
                                  .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  .dstImage = destination,
                                  .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  .regionCount = 1,
                                  .pRegions = &blitRegion,
                                  .filter = VK_FILTER_LINEAR};
        vkCmdBlitImage2(cmd, &blitInfo);
    }
}  // namespace bluevk