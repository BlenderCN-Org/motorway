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
#pragma once

#if FLAN_VULKAN
class DisplaySurface;
class RenderTarget;
class BlendState;
class PipelineState;
class Texture;

#include <Rendering/Viewport.h>

#if FLAN_WIN
#define VK_USE_PLATFORM_WIN32_KHR
#define FLAN_VK_SURF_EXT VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif FLAN_UNIX
#define VK_USE_PLATFORM_XCB_KHR
#define FLAN_VK_SURF_EXT VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif
#include <Core/SystemIncludes.h>
#include <vulkan/vulkan.hpp>


struct NativeRenderContext
{
    std::vector<VkExtensionProperties>  instanceExtensionList;
    VkInstance                          instance;
    VkPhysicalDevice                    physicalDevice;
    VkDevice                            device;
    uint32_t                            physicalDeviceQueueCount;
    std::vector<VkExtensionProperties>  deviceExtensionList;

    VkQueue                             graphicsQueue;
    VkQueue                             computeQueue;
    VkQueue                             presentQueue;

    uint32_t                            graphicsQueueIndex;
    uint32_t                            computeQueueIndex;
    uint32_t                            presentQueueIndex;

    VkSurfaceKHR                        displaySurface;
    VkSwapchainKHR                      swapChain;
    std::vector<VkImage>                swapChainImages;
    VkExtent2D                          swapChainExtent;
    VkFormat                            swapChainFormat;

#if FLAN_DEVBUILD
    VkDebugUtilsMessengerEXT            debugCallback;
#endif

    ~NativeRenderContext()
    {
        vkDestroySwapchainKHR( device, swapChain, nullptr );
        vkDestroySurfaceKHR( instance, displaySurface, nullptr );
        vkDestroyInstance( instance, nullptr );
        vkDestroyDevice( device, nullptr );
        vkDestroyInstance( instance, nullptr );
    }
};

namespace flan
{
    namespace rendering
    {
        NativeRenderContext*    CreateRenderContextImpl( DisplaySurface* surface );
        void                    PresentImpl( NativeRenderContext* renderContext );
        void                    SetVSyncStateImpl( NativeRenderContext* nativeRenderContext, const bool enabled = false );
    }
}
#endif
