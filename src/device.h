#pragma once

#include "window.h"
#include "utils.h"
#include <vector>

struct Swap_Chain_Support_Details
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector< VkSurfaceFormatKHR > formats;
    std::vector< VkPresentModeKHR > presentModes;
};

struct Queue_Family_Indices
{
    u32 graphicsFamily;
    u32 presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
};

inline bool QueueFamilyIndiciesIsComplete( Queue_Family_Indices *queue )
{
    return queue->graphicsFamilyHasValue && queue->presentFamilyHasValue;
}

struct Device
{
#ifdef NDEBUG
    bool enableValidationLayers = false;
#else
    bool enableValidationLayers = true;
#endif

    VkPhysicalDeviceProperties properties;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    Window *window;
    VkCommandPool commandPool;

    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    std::vector< char * > validationLayers = { "VK_LAYER_KHRONOS_validation" };
    std::vector< char * > deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

void InitDevice( Device *device, Window *window );
void DestroyDevice( Device *device );

Swap_Chain_Support_Details GetSwapChainSupport( Device *device );

u32 FindMemoryType( Device *device, u32 typeFilter, VkMemoryPropertyFlags properties );

Queue_Family_Indices FindPhysicalQueueFamilies( Device *device );

VkFormat FindSupportedFormat( const std::vector< VkFormat > &candidates, VkImageTiling tiling, VkFormatFeatureFlags features );

// Buffer Helper Functions
void CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                   VkBuffer &buffer, VkDeviceMemory &bufferMemory );

VkCommandBuffer BeginSingleTimeCommands( Device *device );

void EndSingleTimeCommands( Device *device, VkCommandBuffer commandBuffer );

void CopyBuffer( Device *device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size );

void CopyBufferToImage( Device *device, VkBuffer buffer, VkImage image, u32 width, u32 height, u32 layerCount );

void CreateImageWithInfo( Device *device, VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory );

void CreateInstance( Device *device );

void SetupDebugMessenger( Device *device );

void CreateSurface( Device *device );

void PickPhysicalDevice( Device *device );

void CreateLogicalDevice( Device *device );

void CreateCommandPool( Device *device );

bool IsDeviceSuitable( Device *device, VkPhysicalDevice physicalDevice );

std::vector< const char * > GetRequiredExtensions( Device *device );

bool CheckValidationLayerSupport( Device *device );

Queue_Family_Indices FindQueueFamilies( Device *device, VkPhysicalDevice physicalDevice );

void PopulateDebugMessengerCreateInfo( Device *device, VkDebugUtilsMessengerCreateInfoEXT &createInfo );

void HasGflwRequiredInstanceExtensions( Device *device );

bool CheckDeviceExtensionSupport( Device *device, VkPhysicalDevice physicalDevice );

Swap_Chain_Support_Details QuerySwapChainSupport( Device *device, VkPhysicalDevice physicalDevice );
