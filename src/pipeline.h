#pragma once
#include "utils.h"
#include "device.h"

struct Pipeline_Config_Info
{
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

void CreateGraphicsPipline( Pipeline *pipeline, Device *device, Pipeline_Config_Info *configInfo, char *vertexShaderPath, char *fragmentShaderPath );

void CreateShaderModule( Pipeline *pipeline, Read_File_Result shader, VkShaderModule *module );

Pipeline_Config_Info DefaultPipelineConfigInfo( u32 width, u32 height );
