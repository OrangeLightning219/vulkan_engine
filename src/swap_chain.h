#pragma once

#include "device.h"
#include "vulkan/vulkan.h"

#include <string> //@TODO: Remove the std garbage
#include <vector>

#define MAX_FRAMES_IN_FLIGHT 2

struct Swap_Chain
{
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector< VkFramebuffer > swapChainFramebuffers;
    VkRenderPass renderPass;

    std::vector< VkImage > depthImages;
    std::vector< VkDeviceMemory > depthImageMemorys;
    std::vector< VkImageView > depthImageViews;
    std::vector< VkImage > swapChainImages;
    std::vector< VkImageView > swapChainImageViews;

    Device *device;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;

    std::vector< VkSemaphore > imageAvailableSemaphores;
    std::vector< VkSemaphore > renderFinishedSemaphores;
    std::vector< VkFence > inFlightFences;
    std::vector< VkFence > imagesInFlight;
    size_t currentFrame = 0;
};

void InitSwapChain( Swap_Chain *swapChain, Device *device, VkExtent2D extent );

void DestroySwapChain( Swap_Chain *swapChain );

float32 ExtentAspectRatio( VkExtent2D extent );

VkFormat FindDepthFormat( Swap_Chain *swapChain );

VkResult AcquireNextImage( Swap_Chain *swapChain, u32 *imageIndex );

VkResult SubmitCommandBuffers( Swap_Chain *swapChain, VkCommandBuffer *buffers, u32 *imageIndex );

void CreateSwapChain( Swap_Chain *swapChain );

void CreateImageViews( Swap_Chain *swapChain );

void CreateDepthResources( Swap_Chain *swapChain );

void CreateRenderPass( Swap_Chain *swapChain );

void CreateFramebuffers( Swap_Chain *swapChain );

void CreateSyncObjects( Swap_Chain *swapChain );

VkSurfaceFormatKHR ChooseSwapSurfaceFormat( std::vector< VkSurfaceFormatKHR > &availableFormats );

VkPresentModeKHR ChooseSwapPresentMode( std::vector< VkPresentModeKHR > &availablePresentModes );

VkExtent2D ChooseSwapExtent( Swap_Chain *swapChain, VkSurfaceCapabilitiesKHR *capabilities );
