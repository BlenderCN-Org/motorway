#pragma once

#if NYA_VULKAN
struct VkSampler;

struct Sampler
{
    VkSampler samplerState;
};
#endif
