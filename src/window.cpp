#include "window.h"
#include "glfw3.h"
#include <stdio.h>

void InitWindow( Window *window )
{
    glfwInit();
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    window->window = glfwCreateWindow( window->width, window->height, window->windowName, 0, 0 );
}

void DestroyWindow( Window *window )
{
    glfwDestroyWindow( window->window );
    glfwTerminate();
}

void CreateWindowSurface( Window *window, VkInstance instance, VkSurfaceKHR *surface )
{
    if ( glfwCreateWindowSurface( instance, window->window, 0, surface ) != VK_SUCCESS )
    {
        printf( "Failed to create window surface!\n" );
    }
}
