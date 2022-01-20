#include "window.h"
#include "pipeline.h"
#include "stdio.h"
#include "swap_chain.h"

void CreatePipelineLayout( Pipeline *pipeline, VkPipelineLayout *pipelineLayout )
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = 0;

    if ( vkCreatePipelineLayout( pipeline->device->device, &pipelineLayoutInfo, 0, pipelineLayout ) != VK_SUCCESS )
    {
        printf( "Failed to create pipeline layout!\n" );
        return;
    }
}

void CreatePipeline( Pipeline *pipeline, Device *device, Swap_Chain *swapChain, VkPipelineLayout *pipelineLayout )
{
    pipeline->device = device;
    CreatePipelineLayout( pipeline, pipelineLayout );
    Pipeline_Config_Info pipelineConfig = DefaultPipelineConfigInfo( swapChain->swapChainExtent.width,
                                                                     swapChain->swapChainExtent.height );
    pipelineConfig.renderPass = swapChain->renderPass;
    pipelineConfig.pipelineLayout = *pipelineLayout;
    CreateGraphicsPipline( pipeline, &pipelineConfig, "shaders/simple.vert.spv", "shaders/simple.frag.spv" );
}

void CreateCommandBuffers( std::vector< VkCommandBuffer > &commandBuffers, Device *device, Swap_Chain *swapChain, Pipeline *pipeline )
{
    commandBuffers.resize( swapChain->swapChainImages.size() );

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device->commandPool;
    allocInfo.commandBufferCount = ( u32 ) commandBuffers.size();

    if ( vkAllocateCommandBuffers( device->device, &allocInfo, commandBuffers.data() ) != VK_SUCCESS )
    {
        printf( "Failed to allocate command buffers!\n" );
        return;
    }

    for ( u32 i = 0; i < commandBuffers.size(); ++i )
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if ( vkBeginCommandBuffer( commandBuffers[ i ], &beginInfo ) != VK_SUCCESS )
        {
            printf( "Failed to begin recording command buffer!\n" );
            return;
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapChain->renderPass;
        renderPassInfo.framebuffer = swapChain->swapChainFramebuffers[ i ];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChain->swapChainExtent;

        VkClearValue clearValues[ 2 ] = {};
        clearValues[ 0 ].color = { 0.3f, 0.0f, 0.3f, 1.0f };
        clearValues[ 1 ].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass( commandBuffers[ i ], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        BindPipeline( pipeline, commandBuffers[ i ] );
        vkCmdDraw( commandBuffers[ i ], 3, 1, 0, 0 );

        vkCmdEndRenderPass( commandBuffers[ i ] );
        if ( vkEndCommandBuffer( commandBuffers[ i ] ) != VK_SUCCESS )
        {
            printf( "Failed to record command buffer!\n" );
            return;
        }
    }
}

void DrawFrame( Swap_Chain *swapChain, std::vector< VkCommandBuffer > &commandBuffers )
{
    u32 imageIndex;
    auto result = AcquireNextImage( swapChain, &imageIndex );

    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        printf( "Failed to acquire swap chain image!\n" );
        return;
    }

    result = SubmitCommandBuffers( swapChain, &commandBuffers[ imageIndex ], &imageIndex );

    if ( result != VK_SUCCESS )
    {
        printf( "Failed to present swap chain image!\n" );
        return;
    }
}

int main()
{
    int width = 1920;
    int height = 1080;

    Window window;
    window.width = width;
    window.height = height;
    window.windowName = "Vulkan Engine";
    InitWindow( &window );

    Device device;
    InitDevice( &device, &window );
    defer { DestroyDevice( &device ); };

    Swap_Chain swapChain;
    InitSwapChain( &swapChain, &device, GetWindowExtent( &window ) );
    defer { DestroySwapChain( &swapChain ); };

    Pipeline pipeline;
    VkPipelineLayout pipelineLayout;
    CreatePipeline( &pipeline, &device, &swapChain, &pipelineLayout );

    std::vector< VkCommandBuffer > commandBuffers;
    CreateCommandBuffers( commandBuffers, &device, &swapChain, &pipeline );

    defer
    {
        DestroyPipeline( &pipeline );
        vkDestroyPipelineLayout( device.device, pipelineLayout, 0 );
    };

    while ( !glfwWindowShouldClose( window.window ) )
    {
        glfwPollEvents();
        DrawFrame( &swapChain, commandBuffers );
    }

    vkDeviceWaitIdle( device.device );
    // DestroyPipeline( &pipeline );
    // DestroyDevice( &device );
    return 0;
}
