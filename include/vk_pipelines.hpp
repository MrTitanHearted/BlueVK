#pragma once

#include <types.hpp>
#include <fstream>

namespace bluevk {
    VkShaderModule load_shader_module(VkDevice device, const char *path);

    struct PipelineLayoutBuilder {
        std::vector<VkDescriptorSetLayout> setLayouts{};
        std::vector<VkPushConstantRange> pcRanges{};
        PipelineLayoutBuilder &add_set_layout(VkDescriptorSetLayout setLayout);
        PipelineLayoutBuilder &add_pc_range(VkPushConstantRange pcRange);
        void clear_set_layouts();
        void clear_pc_ranges();
        VkPipelineLayout build(VkDevice device);
    };
    struct ComputePipelineBuilder {
        VkComputePipelineCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
        };
        ComputePipelineBuilder &set_layout(VkPipelineLayout layout);
        ComputePipelineBuilder &set_shader(VkShaderModule shader, const char *name = "main");
        VkPipeline build(VkDevice device);
    };
    struct GraphicsPipelineBuilder {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineLayout pipelineLayout;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipelineRenderingCreateInfo renderInfo;
        VkFormat colorAttachmentFormat;
        GraphicsPipelineBuilder() { clear(); };
        void clear();
        GraphicsPipelineBuilder &set_layout(VkPipelineLayout layout);
        GraphicsPipelineBuilder &set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
        GraphicsPipelineBuilder &set_input_topology(VkPrimitiveTopology topology);
        GraphicsPipelineBuilder &set_polygon_mode(VkPolygonMode mode);
        GraphicsPipelineBuilder &set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
        GraphicsPipelineBuilder &set_multisampling_none();
        GraphicsPipelineBuilder &set_color_attachment_format(VkFormat format);
        GraphicsPipelineBuilder &set_depth_format(VkFormat format);
        GraphicsPipelineBuilder &set_pipeline_layout(VkPipelineLayout layout);
        GraphicsPipelineBuilder &disable_depthtest();
        GraphicsPipelineBuilder &enable_depthtest(bool depthWriteEnable, VkCompareOp op);
        GraphicsPipelineBuilder &disable_blending();
        GraphicsPipelineBuilder &enable_blending_additive();
        GraphicsPipelineBuilder &enable_blending_alphablend();
        VkPipeline build(VkDevice device);
    };
}  // namespace bluevk