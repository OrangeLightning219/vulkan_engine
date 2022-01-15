#include "window.h"
#include "pipeline.h"

int main()
{
    Window window;
    window.width = 1920;
    window.height = 1080;
    window.windowName = "Vulkan Engine";
    InitWindow( &window );

    Device device;
    InitDevice( &device, &window );

    Pipeline pipeline;
    Pipeline_Config_Info configInfo = DefaultPipelineConfigInfo( 1920, 1920 );
    CreateGraphicsPipline( &pipeline, &device, &configInfo, "shaders/simple.vert.spv", "shaders/simple.frag.spv" );

    while ( !glfwWindowShouldClose( window.window ) )
    {
        glfwPollEvents();
    }

    DestroyDevice( &device );
    return 0;
}
