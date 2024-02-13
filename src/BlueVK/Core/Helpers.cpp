#include <BlueVK/Core/Helpers.hpp>

namespace BlueVK {
    VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags) {
        return VkCommandBufferBeginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                        .pNext = nullptr,
                                        .flags = flags,
                                        .pInheritanceInfo = nullptr};
    }
    VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd) {
        return VkCommandBufferSubmitInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                                         .pNext = nullptr,
                                         .commandBuffer = cmd,
                                         .deviceMask = 0};
    }
    VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
        return VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = semaphore,
            .value = 1,
            .stageMask = stageMask,
            .deviceIndex = 0,
        };
    }
    VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd,
                              VkSemaphoreSubmitInfo* signalSemaphoreInfo,
                              VkSemaphoreSubmitInfo* waitSemaphoreInfo) {
        uint32_t waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
        uint32_t signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;

        return VkSubmitInfo2{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                             .pNext = nullptr,
                             .waitSemaphoreInfoCount = waitSemaphoreInfoCount,
                             .pWaitSemaphoreInfos = waitSemaphoreInfo,
                             .commandBufferInfoCount = 1,
                             .pCommandBufferInfos = cmd,
                             .signalSemaphoreInfoCount = signalSemaphoreInfoCount,
                             .pSignalSemaphoreInfos = signalSemaphoreInfo};
    }
    VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask) {
        return VkImageSubresourceRange{
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        };
    }
    VkRenderingAttachmentInfo attachment_info(VkImageView view, VkClearValue* clear, VkImageLayout layout) {
        VkRenderingAttachmentInfo info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = view,
            .imageLayout = layout,
            .loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        if (clear) {
            info.clearValue = *clear;
        }
        return info;
    }
    VkRenderingAttachmentInfo depth_attachment_info(VkImageView view, VkImageLayout layout) {
        VkRenderingAttachmentInfo info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = view,
            .imageLayout = layout,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        info.clearValue.depthStencil.depth = 0.0f;
        return info;
    }
    VkRenderingInfo rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment) {
        return VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .renderArea = VkRect2D{VkOffset2D{0, 0}, renderExtent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = colorAttachment,
            .pDepthAttachment = depthAttachment,
            .pStencilAttachment = nullptr,
        };
    }
    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entry) {
        return VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = stage,
            .module = shaderModule,
            .pName = entry,
        };
    }
};  // namespace BlueVK