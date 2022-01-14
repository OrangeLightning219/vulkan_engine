#include "window.h"
#include "glfw3.h"

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
