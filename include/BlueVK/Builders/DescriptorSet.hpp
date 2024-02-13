#pragma once

#include <BlueVK/Core/Types.hpp>

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