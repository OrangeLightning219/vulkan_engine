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
    }

    fseek( file, 0, SEEK_END );
    long size = ftell( file );
    fseek( file, 0, SEEK_SET );

    u8 *content = ( u8 * ) malloc( size );
    fread( content, size, 1, file );
    fclose( file );

    Read_File_Result result;
    result.size = size;
    result.content = content;

    return result;
}

void CreateGraphicsPipline( char *vertexShaderPath, char *fragmentShaderPath )
{
    Read_File_Result vertexShader = ReadFile( vertexShaderPath );
    Read_File_Result fragmentShader = ReadFile( fragmentShaderPath );

    printf( "Vertex shader size: %d\n", vertexShader.size );
    printf( "Fragment shader size: %d\n", fragmentShader.size );
}
