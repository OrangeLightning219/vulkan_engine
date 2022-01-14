#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

struct Window
{
    GLFWwindow *window;
    int width;
    int height;
    char *windowName;
};

void InitWindow( Window *window );
void DestroyWindow( Window *window );