#include "stdio.h"
#include "stdlib.h"
#include "pipeline.h"

static Read_File_Result ReadFile( char *path )
{
    FILE *file;
    errno_t error = fopen_s( &file, path, "rb" );

    if ( error )
    {
        printf( "Could not read file: %s\n", path );
        return {};
    }

    fseek( file, 0, SEEK_END );
    long size = ftell( file );
    fseek( file, 0, SEEK_SET );

    u32 *content = ( u32 * ) malloc( size );
    fread( content, size, 1, file );
    fclose( file );

    Read_File_Result result;
    result.size = size;
    result.content = content;

    return result;
}

void CreateGraphicsPipline( Pipeline *pipeline, Pipeline_Config_Info *configInfo,
                            char *vertexShaderPath, char *fragmentShaderPath )
{
    Assert( configInfo->pipelineLayout != VK_NULL_HANDLE );
    Assert( configInfo->renderPass != VK_NULL_HANDLE );

    Read_File_Result vertexShader = ReadFile( vertexShaderPath );
    Read_File_Result fragmentShader = ReadFile( fragmentShaderPath );

    CreateShaderModule( pipeline->device->device, vertexShader, &pipeline->vertexShaderModule );
    CreateShaderModule( pipeline->device->device, fragmentShader, &pipeline->fragmentShaderModule );

    VkPipelineShaderStageCreateInfo shaderStages[ 2 ];
    shaderStages[ 0 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[ 0 ].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[ 0 ].module = pipeline->vertexShaderModule;
    shaderStages[ 0 ].pName = "main";
    shaderStages[ 0 ].flags = 0;
    shaderStages[ 0 ].pNext = 0;
    shaderStages[ 0 ].pSpecializationInfo = 0;

    shaderStages[ 1 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[ 1 ].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[ 1 ].module = pipeline->fragmentShaderModule;
    shaderStages[ 1 ].pName = "main";
    shaderStages[ 1 ].flags = 0;
    shaderStages[ 1 ].pNext = 0;
    shaderStages[ 1 ].pSpecializationInfo = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = 0;
    vertexInputInfo.pVertexBindingDescriptions = 0;

    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &configInfo->viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &configInfo->scissor;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &configInfo->colorBlendAttachment;
    colorBlendInfo.blendConstants[ 0 ] = 0.0f; // Optional
    colorBlendInfo.blendConstants[ 1 ] = 0.0f; // Optional
    colorBlendInfo.blendConstants[ 2 ] = 0.0f; // Optional
    colorBlendInfo.blendConstants[ 3 ] = 0.0f; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo->inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &configInfo->rasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo->multisampleInfo;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDepthStencilState = &configInfo->depthStencilInfo;
    pipelineInfo.pDynamicState = 0;

    pipelineInfo.layout = configInfo->pipelineLayout;
    pipelineInfo.renderPass = configInfo->renderPass;
    pipelineInfo.subpass = configInfo->subpass;

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if ( vkCreateGraphicsPipelines( pipeline->device->device, VK_NULL_HANDLE, 1, &pipelineInfo, 0, &pipeline->graphicsPipeline ) != VK_SUCCESS )
    {
        printf( "Failed to create graphics pipeline!\n" );
        return;
    }
}

void DestroyPipeline( Pipeline *pipeline )
{
    vkDestroyShaderModule( pipeline->device->device, pipeline->vertexShaderModule, 0 );
    vkDestroyShaderModule( pipeline->device->device, pipeline->fragmentShaderModule, 0 );
    vkDestroyPipeline( pipeline->device->device, pipeline->graphicsPipeline, 0 );
}

void CreateShaderModule( VkDevice device, Read_File_Result shader, VkShaderModule *module )
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader.size;
    createInfo.pCode = shader.content;

    if ( vkCreateShaderModule( device, &createInfo, 0, module ) != VK_SUCCESS )
    {
        printf( "Failed to create shader module!\n" );
    }
}

Pipeline_Config_Info DefaultPipelineConfigInfo( u32 width, u32 height )
{
    Pipeline_Config_Info configInfo = {};

    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.viewport.x = 0.0f;
    configInfo.viewport.y = 0.0f;
    configInfo.viewport.width = ( float32 ) width;
    configInfo.viewport.height = ( float32 ) height;
    configInfo.viewport.minDepth = 0.0f;
    configInfo.viewport.maxDepth = 1.0f;

    configInfo.scissor.offset = { 0, 0 };
    configInfo.scissor.extent = { width, height };

    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
    configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 1.0f;          // Optional
    configInfo.multisampleInfo.pSampleMask = 0;                  // Optional
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

    configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {}; // Optional
    configInfo.depthStencilInfo.back = {};  // Optional

    return configInfo;
}

void BindPipeline( Pipeline *pipeline, VkCommandBuffer commandBuffer )
{
    vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphicsPipeline );
}
