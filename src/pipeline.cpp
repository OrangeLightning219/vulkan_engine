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

void CreateGraphicsPipline( Pipeline *pipeline, Device *device, Pipeline_Config_Info *configInfo,
                            char *vertexShaderPath, char *fragmentShaderPath )
{
    pipeline->device = device;
    Read_File_Result vertexShader = ReadFile( vertexShaderPath );
    Read_File_Result fragmentShader = ReadFile( fragmentShaderPath );

    printf( "Vertex shader size: %d\n", vertexShader.size );
    printf( "Fragment shader size: %d\n", fragmentShader.size );
}

void CreateShaderModule( Pipeline *pipeline, Read_File_Result shader, VkShaderModule *module )
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader.size;
    createInfo.pCode = shader.content;

    if ( vkCreateShaderModule( pipeline->device->device, &createInfo, 0, module ) != VK_SUCCESS )
    {
        printf( "Failed to create shader module!\n" );
    }
}

Pipeline_Config_Info DefaultPipelineConfigInfo( u32 width, u32 height )
{
    Pipeline_Config_Info configInfo = {};

    return configInfo;
}
