#include <vk_pipelines.hpp>
#include <vk_initializers.hpp>

namespace bluevk {
    VkShaderModule load_shader_module(const char* path, VkDevice device) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to load shader '{}'!", path));
        }

        size_t fileSize = file.tellg();
        std::vector<char> source(fileSize);

        file.seekg(0);
        file.read(source.data(), fileSize);
        file.close();

        VkShaderModuleCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = fileSize,
            .pCode = reinterpret_cast<uint32_t*>(source.data()),
        };

        VkShaderModule shader;
        VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &shader));
        return shader;
    }
    PipelineLayoutBuilder& PipelineLayoutBuilder::add_set_layout(VkDescriptorSetLayout setLayout) {
        setLayouts.push_back(setLayout);
        return *this;
    }
    PipelineLayoutBuilder& PipelineLayoutBuilder::add_pc_range(VkPushConstantRange pcRange) {
        pcRanges.push_back(pcRange);
        return *this;
    }
    void PipelineLayoutBuilder::clear_set_layouts() {
        setLayouts.clear();
    }
    void PipelineLayoutBuilder::clear_pc_ranges() {
        pcRanges.clear();
    }
    VkPipelineLayout PipelineLayoutBuilder::build(VkDevice device) {
        VkPipelineLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = (uint32_t)setLayouts.size(),
            .pSetLayouts = setLayouts.data(),
            .pushConstantRangeCount = (uint32_t)pcRanges.size(),
            .pPushConstantRanges = pcRanges.data(),
        };
        VkPipelineLayout layout;
        VK_CHECK(vkCreatePipelineLayout(device, &info, nullptr, &layout));
        return layout;
    }
    ComputePipelineBuilder& ComputePipelineBuilder::set_layout(VkPipelineLayout layout) {
        info.layout = layout;
        return *this;
    }
    ComputePipelineBuilder& ComputePipelineBuilder::set_shader_stage(VkShaderModule shader, const char* name) {
        info.stage = VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .module = shader,
            .pName = name,
        };
        return *this;
    }
    VkPipeline ComputePipelineBuilder::build(VkDevice device) {
        VkPipeline compute;
        VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &compute));
        return compute;
    }

    void GraphicsPipelineBuilder::clear() {
        inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        colorBlendAttachment = {};
        multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        pipelineLayout = {};
        depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        shaderStages.clear();
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_layout(VkPipelineLayout layout) {
        pipelineLayout = layout;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader) {
        shaderStages.clear();
        shaderStages.push_back(pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
        shaderStages.push_back(pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
        inputAssembly.topology = topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
        rasterizer.polygonMode = mode;
        rasterizer.lineWidth = 1.0f;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace) {
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = frontFace;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_multisampling_none() {
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::disable_blending() {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                              VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT |
                                              VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_color_attachment_format(VkFormat format) {
        colorAttachmentFormat = format;
        renderInfo.colorAttachmentCount = 1;
        renderInfo.pColorAttachmentFormats = &colorAttachmentFormat;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_depth_format(VkFormat format) {
        renderInfo.depthAttachmentFormat = format;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::disable_depthtest() {
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_pipeline_layout(VkPipelineLayout layout) {
        pipelineLayout = layout;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::enable_depthtest(bool depthWriteEnable, VkCompareOp op) {
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = depthWriteEnable;
        depthStencil.depthCompareOp = op;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};
        depthStencil.minDepthBounds = 0.f;
        depthStencil.maxDepthBounds = 1.f;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::enable_blending_additive() {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        return *this;
    }
    GraphicsPipelineBuilder& GraphicsPipelineBuilder::enable_blending_alphablend() {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        return *this;
    }
    VkPipeline GraphicsPipelineBuilder::build(VkDevice device) {
        //! Due to dynamic rendering, we don't have to specify the viewport and scissor
        VkPipelineViewportStateCreateInfo viewportState{.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                                                        .pNext = nullptr,
                                                        .viewportCount = 1,
                                                        .scissorCount = 1};
        VkPipelineColorBlendStateCreateInfo colorBlending{.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                          .pNext = nullptr,
                                                          .logicOpEnable = VK_FALSE,
                                                          .logicOp = VK_LOGIC_OP_COPY,
                                                          .attachmentCount = 1,
                                                          .pAttachments = &colorBlendAttachment};
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        std::vector<VkDynamicState> states{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                                     .pNext = nullptr,
                                                     .dynamicStateCount = (uint32_t)states.size(),
                                                     .pDynamicStates = states.data()};
        VkGraphicsPipelineCreateInfo pipelineInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                  .pNext = &renderInfo,
                                                  .stageCount = (uint32_t)shaderStages.size(),
                                                  .pStages = shaderStages.data(),
                                                  .pVertexInputState = &vertexInputInfo,
                                                  .pInputAssemblyState = &inputAssembly,
                                                  .pViewportState = &viewportState,
                                                  .pRasterizationState = &rasterizer,
                                                  .pMultisampleState = &multisampling,
                                                  .pDepthStencilState = &depthStencil,
                                                  .pColorBlendState = &colorBlending,
                                                  .pDynamicState = &dynamicInfo,
                                                  .layout = pipelineLayout};
        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Failed to create graphics pipeline!\n"));
        }
        return pipeline;
    }
}  // namespace bluevk