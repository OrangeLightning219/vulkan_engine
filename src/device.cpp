#include "device.h"
#include <set> //@TODO: Remove std garbage
#include <unordered_set>
#include <string>

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                     void *userData )
{
    printf( "Validation layer: %s\n", callbackData->pMessage );

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT( VkInstance instance,
                                       VkDebugUtilsMessengerCreateInfoEXT *createInfo,
                                       VkAllocationCallbacks *allocator,
                                       VkDebugUtilsMessengerEXT *debugMessenger )
{
    auto func = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
    if ( func != 0 )
    {
        return func( instance, createInfo, allocator, debugMessenger );
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT( VkInstance instance,
                                    VkDebugUtilsMessengerEXT debugMessenger,
                                    VkAllocationCallbacks *allocator )
{
    auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
    if ( func != 0 )
    {
        func( instance, debugMessenger, allocator );
    }
}

void InitDevice( Device *device, Window *window )
{
    device->window = window;
    CreateInstance( device );
    SetupDebugMessenger( device );
    CreateSurface( device );
    PickPhysicalDevice( device );
    CreateLogicalDevice( device );
    CreateCommandPool( device );
}

void DestroyDevice( Device *device )
{
    vkDestroyCommandPool( device->device, device->commandPool, 0 );
    vkDestroyDevice( device->device, 0 );

    if ( device->enableValidationLayers )
    {
        DestroyDebugUtilsMessengerEXT( device->instance, device->debugMessenger, 0 );
    }

    vkDestroySurfaceKHR( device->instance, device->surface, 0 );
    vkDestroyInstance( device->instance, 0 );
}

void CreateInstance( Device *device )
{
    if ( device->enableValidationLayers && !CheckValidationLayerSupport( device ) )
    {
        printf( "Validation layers requested, but not available!\n" );
        return;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "LittleVulkanEngine App";
    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions( device );
    createInfo.enabledExtensionCount = ( u32 ) extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if ( device->enableValidationLayers )
    {
        createInfo.enabledLayerCount = ( u32 ) device->validationLayers.size();
        createInfo.ppEnabledLayerNames = device->validationLayers.data();

        PopulateDebugMessengerCreateInfo( device, debugCreateInfo );
        createInfo.pNext = ( VkDebugUtilsMessengerCreateInfoEXT * ) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = 0;
    }

    if ( vkCreateInstance( &createInfo, 0, &device->instance ) != VK_SUCCESS )
    {
        printf( "Failed to create instance!\n" );
        return;
    }

    HasGflwRequiredInstanceExtensions( device );
}

void PickPhysicalDevice( Device *device )
{
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices( device->instance, &deviceCount, 0 );
    if ( deviceCount == 0 )
    {
        printf( "Failed to find GPUs with Vulkan support!\n" );
    }
    printf( "Device count: %d\n", deviceCount );
    std::vector< VkPhysicalDevice > devices( deviceCount );
    vkEnumeratePhysicalDevices( device->instance, &deviceCount, devices.data() );

    for ( VkPhysicalDevice &d : devices )
    {
        if ( IsDeviceSuitable( device, d ) )
        {
            device->physicalDevice = d;
            break;
        }
    }

    if ( device->physicalDevice == VK_NULL_HANDLE )
    {
        printf( "Failed to find a suitable GPU!\n" );
        return;
    }

    vkGetPhysicalDeviceProperties( device->physicalDevice, &device->properties );
    printf( "Physical device: %s\n", device->properties.deviceName );
}

void CreateLogicalDevice( Device *device )
{
    Queue_Family_Indices indices = FindQueueFamilies( device, device->physicalDevice );

    std::vector< VkDeviceQueueCreateInfo > queueCreateInfos;
    std::set< u32 > uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

    float queuePriority = 1.0f;
    for ( u32 queueFamily : uniqueQueueFamilies )
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back( queueCreateInfo );
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = ( u32 ) queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = ( u32 ) device->deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = device->deviceExtensions.data();

    // might not really be necessary anymore because device specific validation layers
    // have been deprecated
    if ( device->enableValidationLayers )
    {
        createInfo.enabledLayerCount = ( u32 ) device->validationLayers.size();
        createInfo.ppEnabledLayerNames = device->validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if ( vkCreateDevice( device->physicalDevice, &createInfo, 0, &device->device ) != VK_SUCCESS )
    {
        printf( "Failed to create logical device!\n" );
        return;
    }

    vkGetDeviceQueue( device->device, indices.graphicsFamily, 0, &device->graphicsQueue );
    vkGetDeviceQueue( device->device, indices.presentFamily, 0, &device->presentQueue );
}

void CreateCommandPool( Device *device )
{
    Queue_Family_Indices queueFamilyIndices = FindPhysicalQueueFamilies( device );

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if ( vkCreateCommandPool( device->device, &poolInfo, 0, &device->commandPool ) != VK_SUCCESS )
    {
        printf( "Failed to create command pool!\n" );
    }
}

void CreateSurface( Device *device )
{
    CreateWindowSurface( device->window, device->instance, &device->surface );
}

Swap_Chain_Support_Details GetSwapChainSupport( Device *device )
{
    return QuerySwapChainSupport( device, device->physicalDevice );
}

Queue_Family_Indices FindPhysicalQueueFamilies( Device *device )
{
    return FindQueueFamilies( device, device->physicalDevice );
}

bool IsDeviceSuitable( Device *device, VkPhysicalDevice physicalDevice )
{
    Queue_Family_Indices indices = FindQueueFamilies( device, physicalDevice );

    bool extensionsSupported = CheckDeviceExtensionSupport( device, physicalDevice );

    bool swapChainAdequate = false;
    if ( extensionsSupported )
    {
        Swap_Chain_Support_Details swapChainSupport = QuerySwapChainSupport( device, physicalDevice );
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures( physicalDevice, &supportedFeatures );

    return QueueFamilyIndiciesIsComplete( &indices ) && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void PopulateDebugMessengerCreateInfo( Device *device, VkDebugUtilsMessengerCreateInfoEXT &createInfo )
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = ( PFN_vkDebugUtilsMessengerCallbackEXT ) DebugCallback;
    createInfo.pUserData = 0; // Optional
}

void SetupDebugMessenger( Device *device )
{
    if ( !device->enableValidationLayers ) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo( device, createInfo );
    if ( CreateDebugUtilsMessengerEXT( device->instance, &createInfo, 0, &device->debugMessenger ) != VK_SUCCESS )
    {
        printf( "Failed to set up debug messenger!\n" );
    }
}

bool CheckValidationLayerSupport( Device *device )
{
    u32 layerCount;
    vkEnumerateInstanceLayerProperties( &layerCount, 0 );

    std::vector< VkLayerProperties > availableLayers( layerCount );
    vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

    for ( char *layerName : device->validationLayers )
    {
        bool layerFound = false;

        for ( auto &layerProperties : availableLayers )
        {
            if ( strcmp( layerName, layerProperties.layerName ) == 0 )
            {
                layerFound = true;
                break;
            }
        }

        if ( !layerFound )
        {
            return false;
        }
    }

    return true;
}

std::vector< const char * > GetRequiredExtensions( Device *device )
{
    u32 glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

    std::vector< const char * > extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

    if ( device->enableValidationLayers )
    {
        extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    }

    return extensions;
}

void HasGflwRequiredInstanceExtensions( Device *device )
{
    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties( 0, &extensionCount, 0 );
    std::vector< VkExtensionProperties > extensions( extensionCount );
    vkEnumerateInstanceExtensionProperties( 0, &extensionCount, extensions.data() );

    printf( "Available extensions:\n" );
    std::unordered_set< std::string > available;
    for ( auto &extension : extensions )
    {
        printf( "\t%s\n", extension.extensionName );
        available.insert( extension.extensionName );
    }

    printf( "Required extensions:\n" );
    auto requiredExtensions = GetRequiredExtensions( device );
    for ( auto &required : requiredExtensions )
    {
        printf( "\t%s\n", required );
        if ( available.find( required ) == available.end() )
        {
            printf( "Missing required glfw extension: %s\n", required );
            return;
        }
    }
}

bool CheckDeviceExtensionSupport( Device *device, VkPhysicalDevice physicalDevice )
{
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties( physicalDevice, 0, &extensionCount, 0 );

    std::vector< VkExtensionProperties > availableExtensions( extensionCount );
    vkEnumerateDeviceExtensionProperties( physicalDevice, 0, &extensionCount, availableExtensions.data() );

    std::set< std::string > requiredExtensions( device->deviceExtensions.begin(), device->deviceExtensions.end() );

    for ( auto &extension : availableExtensions )
    {
        requiredExtensions.erase( extension.extensionName );
    }

    return requiredExtensions.empty();
}

Queue_Family_Indices FindQueueFamilies( Device *device, VkPhysicalDevice physicalDevice )
{
    Queue_Family_Indices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, 0 );

    std::vector< VkQueueFamilyProperties > queueFamilies( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data() );

    int i = 0;
    for ( auto &queueFamily : queueFamilies )
    {
        if ( queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
        {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, device->surface, &presentSupport );
        if ( queueFamily.queueCount > 0 && presentSupport )
        {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }
        if ( QueueFamilyIndiciesIsComplete( &indices ) )
        {
            break;
        }

        i++;
    }

    return indices;
}

Swap_Chain_Support_Details QuerySwapChainSupport( Device *device, VkPhysicalDevice physicalDevice )
{
    VkSurfaceKHR &surface = device->surface;
    Swap_Chain_Support_Details details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &details.capabilities );

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, 0 );

    if ( formatCount != 0 )
    {
        details.formats.resize( formatCount );
        vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, details.formats.data() );
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, 0 );

    if ( presentModeCount != 0 )
    {
        details.presentModes.resize( presentModeCount );
        vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, details.presentModes.data() );
    }
    return details;
}

VkFormat FindSupportedFormat( Device *device, std::vector< VkFormat > &candidates, VkImageTiling tiling, VkFormatFeatureFlags features )
{
    for ( VkFormat format : candidates )
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties( device->physicalDevice, format, &props );

        if ( tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features )
        {
            return format;
        }
        else if (
        tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features )
        {
            return format;
        }
    }
    printf( "Failed to find supported format!\n" );
    return {};
}

u32 FindMemoryType( Device *device, u32 typeFilter, VkMemoryPropertyFlags properties )
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties( device->physicalDevice, &memProperties );
    for ( u32 i = 0; i < memProperties.memoryTypeCount; i++ )
    {
        if ( ( typeFilter & ( 1 << i ) ) &&
             ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties )
        {
            return i;
        }
    }

    printf( "Failed to find suitable memory type!\n" );
    return 0xFFFFFFFF;
}

void CreateBuffer( Device *device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                   VkBuffer &buffer, VkDeviceMemory &bufferMemory )
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if ( vkCreateBuffer( device->device, &bufferInfo, 0, &buffer ) != VK_SUCCESS )
    {
        printf( "Failed to create vertex buffer!\n" );
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements( device->device, buffer, &memRequirements );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType( device, memRequirements.memoryTypeBits, properties );

    if ( vkAllocateMemory( device->device, &allocInfo, 0, &bufferMemory ) != VK_SUCCESS )
    {
        printf( "Failed to allocate vertex buffer memory!\n" );
        return;
    }

    vkBindBufferMemory( device->device, buffer, bufferMemory, 0 );
}

VkCommandBuffer BeginSingleTimeCommands( Device *device )
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers( device->device, &allocInfo, &commandBuffer );

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer( commandBuffer, &beginInfo );
    return commandBuffer;
}

void EndSingleTimeCommands( Device *device, VkCommandBuffer commandBuffer )
{
    vkEndCommandBuffer( commandBuffer );

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit( device->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( device->graphicsQueue );

    vkFreeCommandBuffers( device->device, device->commandPool, 1, &commandBuffer );
}

void CopyBuffer( Device *device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size )
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands( device );

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

    EndSingleTimeCommands( device, commandBuffer );
}

void CopyBufferToImage( Device *device, VkBuffer buffer, VkImage image, u32 width, u32 height, u32 layerCount )
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands( device );

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
    EndSingleTimeCommands( device, commandBuffer );
}

void CreateImageWithInfo( Device *device, VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties,
                          VkImage &image, VkDeviceMemory &imageMemory )
{
    if ( vkCreateImage( device->device, &imageInfo, 0, &image ) != VK_SUCCESS )
    {
        printf( "Failed to create image!\n" );
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements( device->device, image, &memRequirements );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType( device, memRequirements.memoryTypeBits, properties );

    if ( vkAllocateMemory( device->device, &allocInfo, 0, &imageMemory ) != VK_SUCCESS )
    {
        printf( "Failed to allocate image memory!\n" );
        return;
    }

    if ( vkBindImageMemory( device->device, image, imageMemory, 0 ) != VK_SUCCESS )
    {
        printf( "Failed to bind image memory!\n" );
        return;
    }
}
