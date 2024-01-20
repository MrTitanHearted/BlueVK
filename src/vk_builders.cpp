#include <vk_builders.hpp>

namespace bluevk {
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
    DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type) {
        bindings.push_back(VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = 1,
        });
        return *this;
    }
    DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::clear() {
        bindings.clear();
        return *this;
    }
    VkDescriptorSetLayout DescriptorSetLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages) {
        for (VkDescriptorSetLayoutBinding& binding : bindings) {
            binding.stageFlags |= shaderStages;
        }
        VkDescriptorSetLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = (uint32_t)bindings.size(),
            .pBindings = bindings.data(),
        };
        VkDescriptorSetLayout layout;
        VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &layout));
        return layout;
    }
    DescriptorSetAllocator& DescriptorSetAllocator::init_pool(VkDevice device, uint32_t maxSets, std::vector<VkDescriptorPoolSize> poolSizes) {
        VkDescriptorPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = maxSets,
            .poolSizeCount = (uint32_t)poolSizes.size(),
            .pPoolSizes = poolSizes.data(),
        };
        VK_CHECK(vkCreateDescriptorPool(device, &info, nullptr, &pool));
        return *this;
    }
    DescriptorSetAllocator& DescriptorSetAllocator::clear(VkDevice device) {
        vkResetDescriptorPool(device, pool, 0);
        return *this;
    }
    void DescriptorSetAllocator::destroy_pool(VkDevice device) {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }
    VkDescriptorSet DescriptorSetAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) {
        VkDescriptorSetAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout,
        };
        VkDescriptorSet set;
        VK_CHECK(vkAllocateDescriptorSets(device, &info, &set));
        return set;
    }
}  // namespace bluevk
