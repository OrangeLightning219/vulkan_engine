#pragma once
#include "utils/utils.h"
#include "device.h"

struct Pipeline_Config_Info
{
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout = 0;
    VkRenderPass renderPass = 0;
    u32 subpass = 0;
};

struct Pipeline
{
    Device *device;
    VkPipeline graphicsPipeline;
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;
};

struct Read_File_Result
{
    int size;
    u32 *content;
};

static Read_File_Result ReadFile( char *path );

void CreateGraphicsPipline( Pipeline *pipeline, Pipeline_Config_Info *configInfo, char *vertexShaderPath, char *fragmentShaderPath );

void DestroyPipeline( Pipeline *pipeline );

void CreateShaderModule( VkDevice device, Read_File_Result shader, VkShaderModule *module );

Pipeline_Config_Info DefaultPipelineConfigInfo( u32 width, u32 height );

void BindPipeline( Pipeline *pipeline, VkCommandBuffer commandBuffer );
