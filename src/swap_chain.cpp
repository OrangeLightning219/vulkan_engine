
#include "swap_chain.h"
#include "stdlib.h"

void InitSwapChain( Swap_Chain *swapChain, Device *device, VkExtent2D extent )
{
    swapChain->device = device;
    swapChain->windowExtent = extent;
    CreateSwapChain( swapChain );
    CreateImageViews( swapChain );
    CreateRenderPass( swapChain );
    CreateDepthResources( swapChain );
    CreateFramebuffers( swapChain );
    CreateSyncObjects( swapChain );
}

void DestroySwapChain( Swap_Chain *swapChain )
{
    VkDevice device = swapChain->device->device;
    for ( auto imageView : swapChain->swapChainImageViews )
    {
        vkDestroyImageView( device, imageView, 0 );
    }
    swapChain->swapChainImageViews.clear();

    if ( swapChain->swapChain != 0 )
    {
        vkDestroySwapchainKHR( device, swapChain->swapChain, 0 );
        swapChain->swapChain = 0;
    }

    for ( u32 i = 0; i < swapChain->depthImages.size(); i++ )
    {
        vkDestroyImageView( device, swapChain->depthImageViews[ i ], 0 );
        vkDestroyImage( device, swapChain->depthImages[ i ], 0 );
        vkFreeMemory( device, swapChain->depthImageMemorys[ i ], 0 );
    }

    for ( auto framebuffer : swapChain->swapChainFramebuffers )
    {
        vkDestroyFramebuffer( device, framebuffer, 0 );
    }

    vkDestroyRenderPass( device, swapChain->renderPass, 0 );

    // cleanup synchronization objects
    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
        vkDestroySemaphore( device, swapChain->renderFinishedSemaphores[ i ], 0 );
        vkDestroySemaphore( device, swapChain->imageAvailableSemaphores[ i ], 0 );
        vkDestroyFence( device, swapChain->inFlightFences[ i ], 0 );
    }
}

float32 ExtentAspectRatio( VkExtent2D extent )
{
    return ( float32 ) extent.width / ( float32 ) extent.height;
}

VkResult AcquireNextImage( Swap_Chain *swapChain, u32 *imageIndex )
{
    vkWaitForFences( swapChain->device->device, 1, &swapChain->inFlightFences[ swapChain->currentFrame ],
                     VK_TRUE, 0xFFFFFFFFFFFFFFFF );

    VkResult result = vkAcquireNextImageKHR( swapChain->device->device, swapChain->swapChain, 0xFFFFFFFFFFFFFFFF,
                                             swapChain->imageAvailableSemaphores[ swapChain->currentFrame ],
                                             VK_NULL_HANDLE, imageIndex );

    return result;
}

VkResult SubmitCommandBuffers( Swap_Chain *swapChain, VkCommandBuffer *buffers, u32 *imageIndex )
{
    if ( swapChain->imagesInFlight[ *imageIndex ] != VK_NULL_HANDLE )
    {
        vkWaitForFences( swapChain->device->device, 1, &swapChain->imagesInFlight[ *imageIndex ], VK_TRUE, UINT64_MAX );
    }
    swapChain->imagesInFlight[ *imageIndex ] = swapChain->inFlightFences[ swapChain->currentFrame ];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { swapChain->imageAvailableSemaphores[ swapChain->currentFrame ] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = { swapChain->renderFinishedSemaphores[ swapChain->currentFrame ] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences( swapChain->device->device, 1, &swapChain->inFlightFences[ swapChain->currentFrame ] );
    if ( vkQueueSubmit( swapChain->device->graphicsQueue, 1, &submitInfo,
                        swapChain->inFlightFences[ swapChain->currentFrame ] ) != VK_SUCCESS )
    {
        printf( "Failed to submit draw command buffer!\n" );
        return {};
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain->swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR( swapChain->device->presentQueue, &presentInfo );

    swapChain->currentFrame = ( swapChain->currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void CreateSwapChain( Swap_Chain *swapChain )
{
    Swap_Chain_Support_Details swapChainSupport = GetSwapChainSupport( swapChain->device );

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat( swapChainSupport.formats );
    VkPresentModeKHR presentMode = ChooseSwapPresentMode( swapChainSupport.presentModes );
    VkExtent2D extent = ChooseSwapExtent( swapChain, &swapChainSupport.capabilities );

    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if ( swapChainSupport.capabilities.maxImageCount > 0 &&
         imageCount > swapChainSupport.capabilities.maxImageCount )
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = swapChain->device->surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Queue_Family_Indices indices = FindPhysicalQueueFamilies( swapChain->device );
    u32 queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if ( indices.graphicsFamily != indices.presentFamily )
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = 0;   // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if ( vkCreateSwapchainKHR( swapChain->device->device, &createInfo, 0, &swapChain->swapChain ) != VK_SUCCESS )
    {
        printf( "Failed to create swap chain!\n" );
        return;
    }

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR( swapChain->device->device, swapChain->swapChain, &imageCount, 0 );
    swapChain->swapChainImages.resize( imageCount );
    vkGetSwapchainImagesKHR( swapChain->device->device, swapChain->swapChain, &imageCount, swapChain->swapChainImages.data() );

    swapChain->swapChainImageFormat = surfaceFormat.format;
    swapChain->swapChainExtent = extent;
}

void CreateImageViews( Swap_Chain *swapChain )
{
    swapChain->swapChainImageViews.resize( swapChain->swapChainImages.size() );
    for ( size_t i = 0; i < swapChain->swapChainImages.size(); i++ )
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapChain->swapChainImages[ i ];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapChain->swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if ( vkCreateImageView( swapChain->device->device, &viewInfo, 0, &swapChain->swapChainImageViews[ i ] ) != VK_SUCCESS )
        {
            printf( "Failed to create texture image view!\n" );
            return;
        }
    }
}

void CreateRenderPass( Swap_Chain *swapChain )
{
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat( swapChain );
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChain->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector< VkAttachmentDescription > attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if ( vkCreateRenderPass( swapChain->device->device, &renderPassInfo, 0, &swapChain->renderPass ) != VK_SUCCESS )
    {
        printf( "Failed to create render pass!\n" );
        return;
    }
}

void CreateFramebuffers( Swap_Chain *swapChain )
{
    swapChain->swapChainFramebuffers.resize( swapChain->swapChainImages.size() );
    for ( size_t i = 0; i < swapChain->swapChainImages.size(); i++ )
    {
        std::vector< VkImageView > attachments = { swapChain->swapChainImageViews[ i ], swapChain->depthImageViews[ i ] };

        VkExtent2D swapChainExtent = swapChain->swapChainExtent;
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = swapChain->renderPass;
        framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if ( vkCreateFramebuffer( swapChain->device->device, &framebufferInfo, 0,
                                  &swapChain->swapChainFramebuffers[ i ] ) != VK_SUCCESS )
        {
            printf( "Failed to create framebuffer!\n" );
            return;
        }
    }
}

void CreateDepthResources( Swap_Chain *swapChain )
{
    VkFormat depthFormat = FindDepthFormat( swapChain );
    VkExtent2D swapChainExtent = swapChain->swapChainExtent;

    size_t imageCount = swapChain->swapChainImages.size();
    swapChain->depthImages.resize( imageCount );
    swapChain->depthImageMemorys.resize( imageCount );
    swapChain->depthImageViews.resize( imageCount );

    for ( u32 i = 0; i < swapChain->depthImages.size(); i++ )
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        CreateImageWithInfo( swapChain->device, imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             swapChain->depthImages[ i ], swapChain->depthImageMemorys[ i ] );

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapChain->depthImages[ i ];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if ( vkCreateImageView( swapChain->device->device, &viewInfo, 0, &swapChain->depthImageViews[ i ] ) != VK_SUCCESS )
        {
            printf( "Failed to create texture image view!\n" );
        }
    }
}

void CreateSyncObjects( Swap_Chain *swapChain )
{
    swapChain->imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
    swapChain->renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
    swapChain->inFlightFences.resize( MAX_FRAMES_IN_FLIGHT );
    swapChain->imagesInFlight.resize( swapChain->swapChainImages.size(), VK_NULL_HANDLE );

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
    {
        if ( vkCreateSemaphore( swapChain->device->device, &semaphoreInfo, 0,
                                &swapChain->imageAvailableSemaphores[ i ] ) != VK_SUCCESS ||
             vkCreateSemaphore( swapChain->device->device, &semaphoreInfo, 0,
                                &swapChain->renderFinishedSemaphores[ i ] ) != VK_SUCCESS ||
             vkCreateFence( swapChain->device->device, &fenceInfo, 0, &swapChain->inFlightFences[ i ] ) != VK_SUCCESS )
        {
            printf( "Failed to create synchronization objects for a frame!\n" );
            return;
        }
    }
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat( std::vector< VkSurfaceFormatKHR > &availableFormats )
{
    for ( const auto &availableFormat : availableFormats )
    {
        if ( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
             availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
        {
            return availableFormat;
        }
    }

    return availableFormats[ 0 ];
}

VkPresentModeKHR ChooseSwapPresentMode( std::vector< VkPresentModeKHR > &availablePresentModes )
{
    for ( const auto &availablePresentMode : availablePresentModes )
    {
        if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
        {
            printf( "Present mode: Mailbox\n" );
            return availablePresentMode;
        }
    }

    // for (const auto &availablePresentMode : availablePresentModes) {
    //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     std::cout << "Present mode: Immediate" << std::endl;
    //     return availablePresentMode;
    //   }
    // }

    printf( "Present mode: V-Sync" );
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent( Swap_Chain *swapChain, VkSurfaceCapabilitiesKHR *capabilities )
{
    if ( capabilities->currentExtent.width != 0xFFFFFFFF )
    {
        return capabilities->currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = swapChain->windowExtent;
        actualExtent.width = ( u32 ) fmax( capabilities->minImageExtent.width,
                                           fmin( capabilities->maxImageExtent.width, actualExtent.width ) );
        actualExtent.height = ( u32 ) fmax( capabilities->minImageExtent.height,
                                            fmin( capabilities->maxImageExtent.height, actualExtent.height ) );

        return actualExtent;
    }
}

VkFormat FindDepthFormat( Swap_Chain *swapChain )
{
    std::vector< VkFormat > canditates{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    return FindSupportedFormat( swapChain->device,
                                canditates,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
}
