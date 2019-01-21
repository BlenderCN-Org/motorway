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

#include "RenderDevice.h"
#include "CommandList.h"

#include <Display/DisplayMode.h>
#include <Display/DisplaySurface.h>

#if NYA_WIN
#include <Display/DisplaySurfaceWin32.h>

#define VK_USE_PLATFORM_WIN32_KHR 1
#define NYA_VK_SURF_EXT VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif NYA_UNIX
#include <Display/DisplaySurfaceXcb.h>

#define VK_USE_PLATFORM_XCB_KHR 1
#define NYA_VK_SURF_EXT VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif

#include <vulkan/vulkan.h>

using namespace nya::rendering;

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
    constexpr char* const EXTENSIONS[3] = {
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
        VK_MAKE_VERSION( 1, 0, 0 ),
        VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
    instanceInfo.enabledExtensionCount = 3;
    instanceInfo.ppEnabledExtensionNames = EXTENSIONS;
    instanceInfo.pApplicationInfo = &appInfo;

#if NYA_DEVBUILD
#ifndef NYA_NO_DEBUG_DEVICE
    constexpr char* validationLayers[1] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    instanceInfo.enabledLayerCount = 1u;
    instanceInfo.ppEnabledLayerNames = validationLayers;
#endif
#endif

    return vkCreateInstance( &instanceInfo, 0, &instance );
}

RenderContext::RenderContext()
    : instance( nullptr )
    , physicalDevice( nullptr )
    , device( nullptr )
{

}

RenderContext::~RenderContext()
{
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

    NYA_ASSERT( !IsInstanceExtensionAvailable( renderContext->instanceExtensionHashes, renderContext->instanceExtensionCount, NYA_STRING_HASH( VK_KHR_SURFACE_EXTENSION_NAME ) ),
        "Missing extension '%s'!",
        VK_KHR_SURFACE_EXTENSION_NAME );

    NYA_ASSERT( !IsInstanceExtensionAvailable( renderContext->instanceExtensionHashes, renderContext->instanceExtensionCount, NYA_STRING_HASH( NYA_VK_SURF_EXT ) ),
        "Missing extension '%s'!",
        NYA_VK_SURF_EXT );

    // Create Vulkan Instance
    VkInstance vulkanInstance;
    VkResult instCreationResult = CreateVkInstance( vulkanInstance );
    NYA_ASSERT( instCreationResult != VK_SUCCESS, "Failed to create Vulkan instance! (error code %i)", instCreationResult );
    
    // Enumerate and select a physical device
    NYA_CLOG << "Created Vulkan Instance! Enumerating devices..." << std::endl;

    uint32_t devCount = 0;
    vkEnumeratePhysicalDevices( vulkanInstance, &devCount, nullptr );
    NYA_ASSERT( ( devCount != 0 ), "No device available!" );

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
}

void RenderDevice::enableVerticalSynchronisation( const bool enabled )
{
    renderContext->presentMode = ( enabled ) ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
}

RenderTarget* RenderDevice::getSwapchainBuffer()
{
    return nullptr;
}

CommandList& RenderDevice::allocateGraphicsCommandList() const
{
  
}

CommandList& RenderDevice::allocateComputeCommandList() const
{

}

void RenderDevice::submitCommandList( CommandList* commandList )
{

}

void RenderDevice::present()
{

}

const nyaChar_t* RenderDevice::getBackendName() const
{
    return NYA_STRING( "Vulkan" );
}
#endif
