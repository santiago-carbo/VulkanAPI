#include "GraphicsPipeline.hpp"
#include "Model.hpp"

#include <fstream>

GraphicsPipeline::GraphicsPipeline(
    VulkanDevice& device,
    const std::string& vertexPath,
    const std::string& fragmentPath,
    const PipelineConfig& config)
    : device{device} 
{
    create(vertexPath, fragmentPath, config);
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyShaderModule(device.getDevice(), vertexModule, nullptr);
    vkDestroyShaderModule(device.getDevice(), fragmentModule, nullptr);
    vkDestroyPipeline(device.getDevice(), pipeline, nullptr);
}

std::vector<char> GraphicsPipeline::readFile(const std::string& path) 
{
    std::string fullPath = "../" + path;
    std::ifstream file{fullPath, std::ios::ate | std::ios::binary};

    if (!file.is_open()) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to open file: " + fullPath + ".");
    }

    size_t size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return (buffer);
}

void GraphicsPipeline::create(
    const std::string& vertexPath,
    const std::string& fragmentPath,
    const PipelineConfig& config) 
{

    assert(config.layout != VK_NULL_HANDLE && "💥[Vulkan API] No pipeline layout provided.");
    assert(config.renderPass != VK_NULL_HANDLE && "💥[Vulkan API] No render pass provided.");

    auto vertexCode = readFile(vertexPath);
    auto fragmentCode = readFile(fragmentPath);

    createShader(vertexCode, &vertexModule);
    createShader(fragmentCode, &fragmentModule);

    VkPipelineShaderStageCreateInfo shaderStages[2] {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexModule;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragmentModule;
    shaderStages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 
        static_cast<uint32_t>(config.bindings.size());

    vertexInputInfo.pVertexBindingDescriptions = config.bindings.data();

    vertexInputInfo.vertexAttributeDescriptionCount = 
        static_cast<uint32_t>(config.attributes.size());

    vertexInputInfo.pVertexAttributeDescriptions = config.attributes.data();

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &config.inputAssembly;
    pipelineInfo.pViewportState = &config.viewport;
    pipelineInfo.pRasterizationState = &config.rasterizer;
    pipelineInfo.pMultisampleState = &config.multisampling;
    pipelineInfo.pColorBlendState = &config.colorBlending;
    pipelineInfo.pDepthStencilState = &config.depthStencil;
    pipelineInfo.pDynamicState = &config.dynamicState;
    pipelineInfo.layout = config.layout;
    pipelineInfo.renderPass = config.renderPass;
    pipelineInfo.subpass = config.subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(
        device.getDevice(),
        VK_NULL_HANDLE,
        1,
        &pipelineInfo,
        nullptr,
        &pipeline) != VK_SUCCESS) 
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create graphics pipeline.");
    }
}

void GraphicsPipeline::createShader(const std::vector<char>& code, VkShaderModule* module) 
{
    VkShaderModuleCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(device.getDevice(), &info, nullptr, module) != VK_SUCCESS)
    {
        throw std::runtime_error("💥[Vulkan API] Failed to create shader module.");
    }
}

void GraphicsPipeline::bind(VkCommandBuffer commandBuffer) 
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void GraphicsPipeline::defaultConfig(PipelineConfig& config) 
{
    config.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.inputAssembly.primitiveRestartEnable = VK_FALSE;

    config.viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config.viewport.viewportCount = 1;
    config.viewport.pViewports = nullptr;
    config.viewport.scissorCount = 1;
    config.viewport.pScissors = nullptr;

    config.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterizer.depthClampEnable = VK_FALSE;
    config.rasterizer.rasterizerDiscardEnable = VK_FALSE;
    config.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    config.rasterizer.lineWidth = 1.0f;
    config.rasterizer.cullMode = VK_CULL_MODE_NONE;
    config.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config.rasterizer.depthBiasEnable = VK_FALSE;

    config.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisampling.sampleShadingEnable = VK_FALSE;
    config.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    config.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    config.colorBlendAttachment.blendEnable = VK_FALSE;

    config.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config.colorBlending.logicOpEnable = VK_FALSE;
    config.colorBlending.logicOp = VK_LOGIC_OP_COPY;
    config.colorBlending.attachmentCount = 1;
    config.colorBlending.pAttachments = &config.colorBlendAttachment;

    config.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config.depthStencil.depthTestEnable = VK_TRUE;
    config.depthStencil.depthWriteEnable = VK_TRUE;
    config.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    config.depthStencil.depthBoundsTestEnable = VK_FALSE;
    config.depthStencil.stencilTestEnable = VK_FALSE;

    config.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    config.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config.dynamicState.pDynamicStates = config.dynamicStates.data();
    config.dynamicState.dynamicStateCount = static_cast<uint32_t>(config.dynamicStates.size());

    config.bindings = Model::Vertex::bindingDescriptions();
    config.attributes = Model::Vertex::attributeDescriptions();
}

void GraphicsPipeline::enableAlphaBlending(PipelineConfig& config) 
{
    config.colorBlendAttachment.blendEnable = VK_TRUE;
    config.colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}
