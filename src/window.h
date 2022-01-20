#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#include "utils/utils.h"

struct Window
{
    GLFWwindow *window;
    int width;
    int height;
    char *windowName;
};

void InitWindow( Window *window );
void DestroyWindow( Window *window );

void CreateWindowSurface( Window *window, VkInstance instance, VkSurfaceKHR *surface );

VkExtent2D GetWindowExtent( Window *window );
