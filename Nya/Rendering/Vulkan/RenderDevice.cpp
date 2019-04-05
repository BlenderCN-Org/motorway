/*
    Project Motorway Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Motorway source code.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <Shared.h>

#if NYA_VULKAN
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#if NYA_WIN
#include <Display/DisplaySurfaceWin32.h>

#define VK_USE_PLATFORM_WIN32_KHR 1
#define NYA_VK_SURF_EXT VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif NYA_UNIX
#include <Display/DisplaySurfaceXcb.h>

#define VK_USE_PLATFORM_XCB_KHR 1
#define NYA_VK_SURF_EXT VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif

#include "RenderDevice.h"

#include "CommandList.h"
#include "RenderTarget.h"

#include <Core/Allocators/PoolAllocator.h>

#include <Display/DisplayMode.h>
#include <Display/DisplaySurface.h>

#include <Maths/Helpers.h>

#include <vulkan/vulkan.h>
#include <limits>

using namespace nya::rendering;
using namespace nya::maths;

#define NYA_GET_INSTANCE_PROC_ADDR( inst, entrypoint )\
        auto entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr( inst, "vk" #entrypoint );\
        if ( entrypoint == nullptr ) {\
            NYA_CERR << "vkGetInstanceProcAddr failed to find vk" << #entrypoint << std::endl; \
        }

bool IsInstanceExtensionAvailable( const nyaStringHash_t* extensions, const uint32_t extensionCount, const nyaStringHash_t extensionHashcode )
{
    for ( uint32_t i = 0; i < extensionCount; i++ ) {
        if ( extensions[i] == extensionHashcode ) {
            return true;
        }
    }

    return false;
}

VkResult CreateVkInstance( VkInstance& instance )
{
    constexpr const char* EXTENSIONS[3] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        NYA_VK_SURF_EXT,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    constexpr VkApplicationInfo appInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "NyaEd",
        VK_MAKE_VERSION( 1, 0, 0 ),
        "Nya",
        VK_MAKE_VERSION( 0, 0, 2 ),
        VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
    instanceInfo.enabledExtensionCount = 3;
    instanceInfo.ppEnabledExtensionNames = EXTENSIONS;
    instanceInfo.pApplicationInfo = &appInfo;

#if NYA_DEVBUILD
    constexpr const char* validationLayers[1] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    instanceInfo.enabledLayerCount = 1u;
    instanceInfo.ppEnabledLayerNames = validationLayers;
#endif

    return vkCreateInstance( &instanceInfo, nullptr, &instance );
}

#if NYA_DEVBUILD
static VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData )
{
    NYA_UNUSED_VARIABLE( messageType );
    NYA_UNUSED_VARIABLE( pUserData );

    if ( messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT )
        NYA_COUT << "[LOG] ";
    else if ( messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
        NYA_COUT << "[INFO] ";
    else if ( messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
        NYA_COUT << "[WARN] ";
    else if ( messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
        NYA_COUT << "[ERROR] ";

    NYA_COUT << NYA_FILENAME << ":" << __LINE__ << " >> " << pCallbackData->pMessageIdName << " : " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
#endif

VkPresentModeKHR chooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes )
{
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for ( const auto& availablePresentMode : availablePresentModes ) {
        if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR ) {
            return availablePresentMode;
        } else if ( availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR ) {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

bool IsDeviceExtAvailable( RenderContext* renderContext, const nyaStringHash_t extNameHashcode )
{
    if ( renderContext->deviceExtensionList.empty() ) {
        uint32_t devExtCount = 0;
        vkEnumerateDeviceExtensionProperties( renderContext->physicalDevice, nullptr, &devExtCount, nullptr );

        if ( devExtCount == 0 ) {
            return false;
        }

        renderContext->deviceExtensionList.resize( devExtCount );
        vkEnumerateDeviceExtensionProperties( renderContext->physicalDevice, nullptr, &devExtCount, &renderContext->deviceExtensionList[0] );

        NYA_CLOG << "Found " << devExtCount << " device extension(s)" << std::endl;
    }

    for ( auto& extension : renderContext->deviceExtensionList ) {
        auto hashcode = nya::core::CRC32( extension.extensionName );

        if ( hashcode == extNameHashcode ) {
            return true;
        }
    }

    return false;
}

VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities )
{
    if ( capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = { 640, 480 };

        actualExtent.width = max( capabilities.minImageExtent.width, min( capabilities.maxImageExtent.width, actualExtent.width ) );
        actualExtent.height = max( capabilities.minImageExtent.height, min( capabilities.maxImageExtent.height, actualExtent.height ) );

        return actualExtent;
    }
}

RenderContext::RenderContext()
    : instance( nullptr )
    , physicalDevice( nullptr )
    , device( nullptr )
    , currentFrameIndex( 0u )
{

}

RenderContext::~RenderContext()
{
    vkDestroyDescriptorPool( device, descriptorPool, nullptr );

    vkDestroySwapchainKHR( device, swapChain, nullptr );
    vkDestroySurfaceKHR( instance, displaySurface, nullptr );
    vkDestroyInstance( instance, nullptr );
    vkDestroyDevice( device, nullptr );
    vkDestroyInstance( instance, nullptr );
}

RenderDevice::~RenderDevice()
{
    nya::core::free( memoryAllocator, renderContext );
}

void RenderDevice::create( DisplaySurface* surface )
{
    NYA_CLOG << "Creating RenderDevice (Vulkan)" << std::endl;

    renderContext = nya::core::allocate<RenderContext>( memoryAllocator );

    VkExtensionProperties* instanceExtensionList;
    vkEnumerateInstanceExtensionProperties( nullptr, &renderContext->instanceExtensionCount, nullptr );

    instanceExtensionList = nya::core::allocateArray<VkExtensionProperties>( memoryAllocator, renderContext->instanceExtensionCount );
    vkEnumerateInstanceExtensionProperties( nullptr, &renderContext->instanceExtensionCount, instanceExtensionList );

    renderContext->instanceExtensionHashes = nya::core::allocateArray<nyaStringHash_t>( memoryAllocator, renderContext->instanceExtensionCount );
    for ( uint32_t i = 0; i < renderContext->instanceExtensionCount; i++ ) {
        renderContext->instanceExtensionHashes[i] = nya::core::CRC32( instanceExtensionList[i].extensionName );
    }
    nya::core::freeArray( memoryAllocator, instanceExtensionList );

    NYA_CLOG << "Found " << renderContext->instanceExtensionCount << " instance extension(s)" << std::endl;

    NYA_ASSERT( IsInstanceExtensionAvailable( renderContext->instanceExtensionHashes, renderContext->instanceExtensionCount, NYA_STRING_HASH( VK_KHR_SURFACE_EXTENSION_NAME ) ),
        "Missing extension '%s'!",
        VK_KHR_SURFACE_EXTENSION_NAME );

    NYA_ASSERT( IsInstanceExtensionAvailable( renderContext->instanceExtensionHashes, renderContext->instanceExtensionCount, NYA_STRING_HASH( NYA_VK_SURF_EXT ) ),
        "Missing extension '%s'!",
        NYA_VK_SURF_EXT );

    // Create Vulkan Instance
    VkInstance vulkanInstance;
    VkResult instCreationResult = CreateVkInstance( vulkanInstance );
    NYA_ASSERT( instCreationResult == VK_SUCCESS, "Failed to create Vulkan instance! (error code %i)", instCreationResult );
    
    // Enumerate and select a physical device
    NYA_CLOG << "Created Vulkan Instance! Enumerating devices..." << std::endl;

    uint32_t devCount = 0;
    vkEnumeratePhysicalDevices( vulkanInstance, &devCount, nullptr );
    NYA_ASSERT( ( devCount != 0 ), "No device available! (devCount = %i)", devCount );

    NYA_CLOG << "Found " << devCount << " device(s)" << std::endl;

    std::vector<VkPhysicalDevice> devList( devCount );

    vkEnumeratePhysicalDevices( vulkanInstance, &devCount, &devList[0] );

    VkPhysicalDeviceProperties tmpProperties = {};
    int deviceIdx = 0, bestDeviceIdx = 0;

    uint32_t bestMemoryAllocationCount = 0, bestUniformBufferRange = 0, bestQueueCount = 0;
    bool bestHasCompute = false, bestHasGraphics = false;
    for ( auto& dev : devList ) {
        vkGetPhysicalDeviceProperties( dev, &tmpProperties );

        NYA_CLOG << "-Device[" << deviceIdx << "] = {" << tmpProperties.deviceID << ", " << tmpProperties.deviceName << ", ";
        switch ( tmpProperties.deviceType ) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            NYA_COUT << "INTEGRATED_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            NYA_COUT << "DISCRETE_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            NYA_COUT << "VIRTUAL_GPU";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            NYA_COUT << "TYPE_CPU";
            break;
        default:
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            NYA_COUT << "Unknown Device Type";
            break;
        }

        NYA_COUT << " }" << std::endl;

        // Check device queues
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( dev, &queueCount, nullptr );

        if ( queueCount == 0 ) {
            NYA_CERR << "No queues available!" << std::endl;
            continue;
        }

        std::vector<VkQueueFamilyProperties> queueList( queueCount );
        vkGetPhysicalDeviceQueueFamilyProperties( dev, &queueCount, &queueList[0] );

        bool hasComputeQueue = false, hasGraphicsQueue = false;

        for ( auto& queue : queueList ) {
            if ( queue.queueCount > 0 && queue.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
                hasGraphicsQueue = true;
            } else if ( queue.queueCount > 0 && queue.queueFlags & VK_QUEUE_COMPUTE_BIT ) {
                hasComputeQueue = true;
            }
        }

        if ( tmpProperties.limits.maxMemoryAllocationCount > bestMemoryAllocationCount
            && tmpProperties.limits.maxUniformBufferRange > bestUniformBufferRange ) {
            // If best device has more render queues than the current one; skip it
            if ( ( bestHasGraphics && !hasGraphicsQueue )
                || ( bestHasCompute && !hasComputeQueue ) ) {
                continue;
            }

            bestMemoryAllocationCount = tmpProperties.limits.maxMemoryAllocationCount;
            bestUniformBufferRange = tmpProperties.limits.maxUniformBufferRange;

            bestQueueCount = queueCount;
            bestDeviceIdx = deviceIdx;
        }
        deviceIdx++;
    }

    NYA_CLOG << "Selected: Device[" << bestDeviceIdx << "]" << std::endl;
    const VkPhysicalDevice physicalDevice = devList[bestDeviceIdx];
    renderContext->physicalDevice = physicalDevice;

    // Create display surface
    NYA_ASSERT( IsDeviceExtAvailable( renderContext, NYA_STRING_HASH( VK_KHR_SWAPCHAIN_EXTENSION_NAME ) ), "Missing extension: %s", VK_KHR_SWAPCHAIN_EXTENSION_NAME );

    VkSurfaceKHR displaySurf = nullptr;

#if NYA_WIN
    auto nativeDispSurf = surface->nativeDisplaySurface;

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = nativeDispSurf->Instance;
    createInfo.hwnd = nativeDispSurf->Handle;
    auto CreateWin32SurfaceKHR = ( PFN_vkCreateWin32SurfaceKHR )vkGetInstanceProcAddr( vulkanInstance, "vkCreateWin32SurfaceKHR" );
    VkResult surfCreateResult = CreateWin32SurfaceKHR( vulkanInstance, &createInfo, nullptr, &displaySurf );

    NYA_ASSERT( ( surfCreateResult == VK_SUCCESS ), "Failed to create display surface! (error code: %i)", surfCreateResult );
#elif NYA_UNIX
    auto nativeDispSurf = surface->nativeDisplaySurface;

    VkXcbSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.window = nativeDispSurf->WindowInstance;
    createInfo.connection = nativeDispSurf->Connection;
    auto CreateXcbSurfaceKHR = ( PFN_vkCreateXcbSurfaceKHR )vkGetInstanceProcAddr( vulkanInstance, "vkCreateXcbSurfaceKHR" );
    VkResult surfCreateResult = CreateXcbSurfaceKHR( vulkanInstance, &createInfo, nullptr, &displaySurf );

    NYA_ASSERT( ( surfCreateResult == VK_SUCCESS ), "Failed to create display surface! (error code: %i)", surfCreateResult );
#endif

    NYA_GET_INSTANCE_PROC_ADDR( vulkanInstance, GetPhysicalDeviceSurfaceSupportKHR );

#if NYA_DEVBUILD
    NYA_GET_INSTANCE_PROC_ADDR( vulkanInstance, CreateDebugUtilsMessengerEXT );

    VkDebugUtilsMessengerCreateInfoEXT dbgMessengerCreateInfos = {};
    dbgMessengerCreateInfos.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbgMessengerCreateInfos.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbgMessengerCreateInfos.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbgMessengerCreateInfos.pfnUserCallback = VkDebugCallback;
    VkResult dbgMessengerCreateResult = CreateDebugUtilsMessengerEXT( vulkanInstance, &dbgMessengerCreateInfos, nullptr, &renderContext->debugCallback );

    NYA_ASSERT( ( dbgMessengerCreateResult == VK_SUCCESS ), "Failed to create debug callback! (error code: %i)", dbgMessengerCreateResult );
#else
    NYA_CLOG << "Debug Layer is disabled (build was compiled with NYA_NO_DEBUG_DEVICE)" << std::endl;
#endif

    std::vector<VkBool32> presentSupport( bestQueueCount );
    for ( uint32_t i = 0; i < bestQueueCount; i++ ) {
        GetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, displaySurf, &presentSupport[i] );
    }

    std::vector<VkQueueFamilyProperties> queueList( bestQueueCount );
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &bestQueueCount, &queueList[0] );

    uint32_t computeQueueFamilyIndex = UINT32_MAX;
    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t presentQueueFamilyIndex = UINT32_MAX;
    for ( uint32_t i = 0; i < queueList.size(); i++ ) {
        if ( queueList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
            if ( graphicsQueueFamilyIndex == UINT32_MAX ) {
                graphicsQueueFamilyIndex = i;
            }

            if ( presentSupport[i] == VK_TRUE ) {
                graphicsQueueFamilyIndex = i;
                presentQueueFamilyIndex = i;
            }
        } else if ( queueList[i].queueFlags & VK_QUEUE_COMPUTE_BIT ) {
            if ( computeQueueFamilyIndex == UINT32_MAX ) {
                computeQueueFamilyIndex = i;
            }
        }
    }

    if ( presentQueueFamilyIndex == UINT32_MAX ) {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for ( uint32_t i = 0; i < queueList.size(); i++ ) {
            if ( presentSupport[i] == VK_TRUE ) {
                presentQueueFamilyIndex = i;
                break;
            }
        }
    }

    NYA_ASSERT( ( graphicsQueueFamilyIndex != UINT32_MAX && presentQueueFamilyIndex != UINT32_MAX ),
                "Could not find both graphics queue, present queue or graphics/present queue (graphicsQueueFamilyIndex %i, presentQueueFamilyIndex %i)",
                graphicsQueueFamilyIndex, presentQueueFamilyIndex );

    // Create queues
    constexpr float queuePrios = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queues;

    const VkDeviceQueueCreateInfo graphicsQueueInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        graphicsQueueFamilyIndex,
        1,
        &queuePrios,
    };

    queues.push_back( graphicsQueueInfo );

    if ( computeQueueFamilyIndex != UINT32_MAX ) {
        const VkDeviceQueueCreateInfo computeQueueInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            nullptr,
            0,
            computeQueueFamilyIndex,
            1,
            &queuePrios,
        };

        queues.push_back( computeQueueInfo );
    }

    constexpr const char* const DEVICE_EXTENSIONS[1] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.imageCubeArray = VK_TRUE;
    deviceFeatures.shaderStorageImageExtendedFormats = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo deviceCreationInfos;
    deviceCreationInfos.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreationInfos.pNext = nullptr;
    deviceCreationInfos.flags = 0;
    deviceCreationInfos.queueCreateInfoCount = static_cast<uint32_t>( queues.size() );
    deviceCreationInfos.pQueueCreateInfos = queues.data();
    deviceCreationInfos.enabledLayerCount = 0u;
    deviceCreationInfos.ppEnabledLayerNames = nullptr;
    deviceCreationInfos.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
    deviceCreationInfos.enabledExtensionCount = 1;
    deviceCreationInfos.pEnabledFeatures = &deviceFeatures;

    VkDevice device;
    VkResult deviceCreationResult = vkCreateDevice( physicalDevice, &deviceCreationInfos, nullptr, &device );
    NYA_ASSERT( ( deviceCreationResult == VK_SUCCESS ), "Device creation failed! (error code: %i)", deviceCreationResult );

    vkGetDeviceQueue( device, graphicsQueueFamilyIndex, 0, &renderContext->graphicsQueue );
    vkGetDeviceQueue( device, presentQueueFamilyIndex, 0, &renderContext->presentQueue );
    vkGetDeviceQueue( device, ( computeQueueFamilyIndex == UINT32_MAX ) ? graphicsQueueFamilyIndex : computeQueueFamilyIndex, 0, &renderContext->computeQueue );

    uint32_t surfFormatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, displaySurf, &surfFormatsCount, VK_NULL_HANDLE );
    NYA_CLOG << "Found " << surfFormatsCount << " surface format(s)" << std::endl;

    std::vector<VkSurfaceFormatKHR> formatList( surfFormatsCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, displaySurf, &surfFormatsCount, formatList.data() );

    // Select surface format
    // TODO Let the user pick the format? (if the user has a fancy display)
    VkSurfaceFormatKHR selectedFormat;
    for ( auto& format : formatList ) {
        if ( format.format != VK_FORMAT_UNDEFINED ) {
            NYA_CLOG << "Selected Format: " << format.format << " (colorspace: " << format.colorSpace << ")" << std::endl;
            selectedFormat = format;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR surfCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, displaySurf, &surfCapabilities );

    // Create swapchain
    VkSwapchainCreateInfoKHR swapChainCreation = {};
    swapChainCreation.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreation.pNext = nullptr;
    swapChainCreation.surface = displaySurf;

    uint32_t imageCount = surfCapabilities.minImageCount + 1;
    if ( surfCapabilities.maxImageCount > 0
        && imageCount > surfCapabilities.maxImageCount ) {
        imageCount = surfCapabilities.maxImageCount;
    }

    auto extent = chooseSwapExtent( surfCapabilities );
    swapChainCreation.minImageCount = imageCount;
    swapChainCreation.imageFormat = selectedFormat.format;
    swapChainCreation.imageColorSpace = selectedFormat.colorSpace;
    swapChainCreation.imageExtent = extent;
    swapChainCreation.imageArrayLayers = 1;
    swapChainCreation.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vkGetDeviceQueue( device, graphicsQueueFamilyIndex, 0, &renderContext->graphicsQueue );
    vkGetDeviceQueue( device, presentQueueFamilyIndex, 0, &renderContext->presentQueue );

    uint32_t queueIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
    if ( graphicsQueueFamilyIndex != presentQueueFamilyIndex ) {
        swapChainCreation.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreation.queueFamilyIndexCount = 2;
        swapChainCreation.pQueueFamilyIndices = queueIndices;
    } else {
        swapChainCreation.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, displaySurf, &presentModeCount, nullptr );

    std::vector<VkPresentModeKHR> presentModes;
    if ( presentModeCount != 0 ) {
        presentModes.resize( presentModeCount );
        vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, displaySurf, &presentModeCount, presentModes.data() );
    }

    swapChainCreation.preTransform = surfCapabilities.currentTransform;
    swapChainCreation.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreation.presentMode = chooseSwapPresentMode( presentModes );
    swapChainCreation.clipped = VK_TRUE;

    swapChainCreation.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain = {};
    VkResult swapChainCreationResult = vkCreateSwapchainKHR( device, &swapChainCreation, nullptr, &swapChain );
    NYA_ASSERT( ( swapChainCreationResult == VK_SUCCESS ), "Failed to create swapchain! (error code: %i)", swapChainCreationResult );

    vkGetSwapchainImagesKHR( device, swapChain, &imageCount, nullptr );
    renderContext->swapChainImages.resize( imageCount );
    vkGetSwapchainImagesKHR( device, swapChain, &imageCount, renderContext->swapChainImages.data() );

    for ( auto& swapChainImage : renderContext->swapChainImages ) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImage;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = selectedFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        RenderTarget* renderTarget = nya::core::allocate<RenderTarget>( memoryAllocator );
        vkCreateImageView(device, &createInfo, nullptr, &renderTarget->textureRenderTargetView);

        renderTarget->textureRenderTargetViewPerSlice = nya::core::allocateArray<VkImageView>( memoryAllocator, 1 );
        renderTarget->textureRenderTargetViewPerSliceAndMipLevel = nya::core::allocateArray<VkImageView*>( memoryAllocator, 1 );
        renderTarget->textureRenderTargetViewPerSliceAndMipLevel[0] = nya::core::allocateArray<VkImageView>( memoryAllocator, 1 );

        renderTarget->textureRenderTargetViewPerSlice[0] = renderTarget->textureRenderTargetView;
        renderTarget->textureRenderTargetViewPerSliceAndMipLevel[0][0] = renderTarget->textureRenderTargetView;

        renderContext->swapChainRenderTargets.push_back( renderTarget );
    }

    // Set RenderContext pointers
    renderContext->instance = vulkanInstance;
    renderContext->device = device;
    renderContext->displaySurface = displaySurf;
    renderContext->swapChain = swapChain;
    renderContext->displaySurface = displaySurf;
    renderContext->swapChainExtent = extent;
    renderContext->swapChainFormat = selectedFormat.format;

    renderContext->computeQueueIndex = computeQueueFamilyIndex;
    renderContext->graphicsQueueIndex = graphicsQueueFamilyIndex;
    renderContext->presentQueueIndex = presentQueueFamilyIndex;

    NYA_CLOG << "Allocate CommandPools..." << std::endl;

    // Allocate cmdList Pools
    VkCommandPoolCreateInfo gfxCmdPoolDesc;
    gfxCmdPoolDesc.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    gfxCmdPoolDesc.pNext = nullptr;
    gfxCmdPoolDesc.queueFamilyIndex = renderContext->graphicsQueueIndex;
    gfxCmdPoolDesc.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool( renderContext->device, &gfxCmdPoolDesc, nullptr, &renderContext->graphicsCommandPool );

    VkCommandPoolCreateInfo computeCmdPoolDesc;
    computeCmdPoolDesc.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    computeCmdPoolDesc.pNext = nullptr;
    computeCmdPoolDesc.queueFamilyIndex = renderContext->computeQueueIndex;
    computeCmdPoolDesc.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool( renderContext->device, &computeCmdPoolDesc, nullptr, &renderContext->computeCommandPool );

    VkCommandBuffer gfxCmdBuffers[16];
    VkCommandBufferAllocateInfo cmdBufferAlloc;
    cmdBufferAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAlloc.pNext = nullptr;
    cmdBufferAlloc.commandPool = renderContext->graphicsCommandPool;
    cmdBufferAlloc.commandBufferCount = 16u;
    cmdBufferAlloc.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers( renderContext->device, &cmdBufferAlloc, gfxCmdBuffers );

    renderContext->cmdListPool = nya::core::allocateArray<CommandList>( memoryAllocator, 16, memoryAllocator );
    for ( size_t i = 0; i < 16; i++ ) {
        renderContext->cmdListPool[i].CommandListObject = nya::core::allocate<NativeCommandList>( memoryAllocator );
        renderContext->cmdListPool[i].CommandListObject->cmdBuffer = gfxCmdBuffers[i];
        renderContext->cmdListPool[i].CommandListObject->device = device;
        renderContext->cmdListPool[i].CommandListObject->isRenderPassInProgress = false;
        renderContext->cmdListPool[i].CommandListObject->resourcesBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
    }

    renderContext->cmdListPoolCapacity = 16;
    renderContext->cmdListPoolIndex = 0;

    VkCommandBuffer computeCmdBuffers[16];
    VkCommandBufferAllocateInfo computeCmdBufferAlloc;
    computeCmdBufferAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    computeCmdBufferAlloc.pNext = nullptr;
    computeCmdBufferAlloc.commandPool = renderContext->computeCommandPool;
    computeCmdBufferAlloc.commandBufferCount = 16u;
    computeCmdBufferAlloc.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers( renderContext->device, &computeCmdBufferAlloc, computeCmdBuffers );

    renderContext->cmdListComputePool = nya::core::allocateArray<CommandList>( memoryAllocator, 16, memoryAllocator );
    for ( size_t i = 0; i < 16; i++ ) {
        renderContext->cmdListComputePool[i].CommandListObject = nya::core::allocate<NativeCommandList>( memoryAllocator );
        renderContext->cmdListComputePool[i].CommandListObject->cmdBuffer = computeCmdBuffers[i];
        renderContext->cmdListComputePool[i].CommandListObject->device = device;
        renderContext->cmdListComputePool[i].CommandListObject->isRenderPassInProgress = false;
        renderContext->cmdListComputePool[i].CommandListObject->resourcesBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    renderContext->cmdListComputePoolIndex = 16;
    renderContext->cmdListComputePoolCapacity = 16;

    NYA_CLOG << "Allocate DescriptorPools..." << std::endl;

    VkDescriptorPoolCreateInfo descriptorPoolDesc;
    descriptorPoolDesc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolDesc.pNext = nullptr;
    descriptorPoolDesc.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolDesc.maxSets = 64u;

    VkDescriptorPoolSize descriptorPoolSizes[7] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 64u },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 64u },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 64u },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 64u },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 64u },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 64u },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 64u },
    };

    descriptorPoolDesc.pPoolSizes = descriptorPoolSizes;
    descriptorPoolDesc.poolSizeCount = 7u;

    vkCreateDescriptorPool( renderContext->device, &descriptorPoolDesc, nullptr, &renderContext->descriptorPool );
}

void RenderDevice::enableVerticalSynchronisation( const bool enabled )
{
    renderContext->presentMode = ( enabled ) ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
}

CommandList& RenderDevice::allocateGraphicsCommandList() const
{
    renderContext->cmdListPoolIndex = ( ++renderContext->cmdListPoolIndex % renderContext->cmdListPoolCapacity );

    CommandList& cmdList = renderContext->cmdListPool[renderContext->cmdListPoolIndex];
    vkResetCommandBuffer( cmdList.CommandListObject->cmdBuffer, 0u );

    cmdList.CommandListObject->isRenderPassInProgress = false;

    return cmdList;
}

CommandList& RenderDevice::allocateComputeCommandList() const
{
    renderContext->cmdListComputePoolIndex = ( ++renderContext->cmdListComputePoolIndex % renderContext->cmdListComputePoolCapacity );

    CommandList& cmdList = renderContext->cmdListComputePool[renderContext->cmdListComputePoolIndex];
    vkResetCommandBuffer( cmdList.CommandListObject->cmdBuffer, 0u );

    cmdList.CommandListObject->isRenderPassInProgress = false;

    return cmdList;
}

void RenderDevice::submitCommandList( CommandList* commandList )
{
    const NativeCommandList* cmdList = commandList->CommandListObject;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdList->cmdBuffer;

    VkQueue submitQueue = ( cmdList->resourcesBindPoint == VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS ) ? renderContext->graphicsQueue : renderContext->computeQueue;
    vkQueueSubmit( submitQueue, 1u, &submitInfo, nullptr );
}

RenderTarget* RenderDevice::getSwapchainBuffer()
{
    return renderContext->swapChainRenderTargets.at( renderContext->currentFrameIndex );
}

void RenderDevice::present()
{
    vkAcquireNextImageKHR( renderContext->device, renderContext->swapChain, std::numeric_limits<uint64_t>::max(), nullptr, VK_NULL_HANDLE, &renderContext->currentFrameIndex );

    VkSwapchainKHR swapChains[1] = { renderContext->swapChain };

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.pWaitSemaphores = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &renderContext->currentFrameIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR( renderContext->presentQueue, &presentInfo );
}

const nyaChar_t* RenderDevice::getBackendName() const
{
    return NYA_STRING( "Vulkan" );
}
#endif
