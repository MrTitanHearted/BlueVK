#include <BlueVK/Builders/DescriptorSet.hpp>

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