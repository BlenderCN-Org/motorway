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

#if NYA_VULKAN
class PoolAllocator;
class CommandList;

struct  RenderTarget;

#include <vector>
#include <vulkan/vulkan.h>

struct RenderContext
{
                                        RenderContext();
                                        ~RenderContext();

    nyaStringHash_t*                    instanceExtensionHashes;
    uint32_t                            instanceExtensionCount;

    VkInstance                          instance;
    VkPhysicalDevice                    physicalDevice;
    VkDevice                            device;
    VkSurfaceKHR                        displaySurface;
    VkSwapchainKHR                      swapChain;
    VkPresentModeKHR                    presentMode;
    uint32_t                            physicalDeviceQueueCount;
    std::vector<VkExtensionProperties>  deviceExtensionList;

    CommandList*                        cmdListPool;
    size_t                              cmdListPoolIndex;
    size_t                              cmdListPoolCapacity;

    CommandList*                        cmdListComputePool;
    size_t                              cmdListComputePoolIndex;
    size_t                              cmdListComputePoolCapacity;

#if NYA_DEVBUILD
    PFN_vkDebugMarkerSetObjectNameEXT   debugObjectMarker;
    VkDebugUtilsMessengerEXT_T*         debugCallback;
#endif

    VkQueue                             graphicsQueue;
    VkQueue                             computeQueue;
    VkQueue                             presentQueue;

    uint32_t                            graphicsQueueIndex;
    uint32_t                            computeQueueIndex;
    uint32_t                            presentQueueIndex;

    uint32_t                            currentFrameIndex;
    std::vector<VkImage>                swapChainImages;
    std::vector<RenderTarget*>          swapChainRenderTargets;
    VkExtent2D                          swapChainExtent;
    VkFormat                            swapChainFormat;

    VkDescriptorPool                    descriptorPool;

    VkCommandPool                       graphicsCommandPool;
    VkCommandPool                       computeCommandPool;

    VkSemaphore                         presentSemaphore;
};
#endif
