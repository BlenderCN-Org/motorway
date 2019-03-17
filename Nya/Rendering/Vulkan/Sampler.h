#pragma once

#if NYA_VULKAN
struct VkSampler_T;

struct Sampler
{
    VkSampler_T*    samplerState;
};
#endif
