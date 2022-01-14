#pragma once
#include "utils.h"

struct Pipeline
{
};

struct Read_File_Result
{
    int size;
    u8 *content;
};

static Read_File_Result ReadFile( char *path );

void CreateGraphicsPipline( char *vertexShaderPath, char *fragmentShaderPath );
