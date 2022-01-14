#include "window.h"
#include "pipeline.h"

int main()
{
    Window window;
    window.width = 1920;
    window.height = 1080;
    window.windowName = "Vulkan Engine";
    InitWindow( &window );

    CreateGraphicsPipline( "shaders/simple.vert.spv", "shaders/simple.frag.spv" );

    while ( !glfwWindowShouldClose( window.window ) )
    {
        glfwPollEvents();
    }

    return 0;
}
