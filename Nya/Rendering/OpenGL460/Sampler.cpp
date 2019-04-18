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

#if NYA_GL460
#include "Extensions.h"

#include <Rendering/RenderDevice.h>

using namespace nya::rendering;

struct Sampler
{
    GLuint  samplerHandle;
};

static constexpr GLenum GL_SAMPLER_ADDRESS[eSamplerAddress::SAMPLER_ADDRESS_COUNT] = {
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_CLAMP_TO_EDGE,
    GL_CLAMP_TO_BORDER,
    GL_MIRROR_CLAMP_TO_EDGE
};

static constexpr GLenum GL_MIN_FILTER[eSamplerFilter::SAMPLER_FILTER_COUNT] = {
    GL_NEAREST,
    GL_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,

    GL_NEAREST,
    GL_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,
};

static constexpr GLenum GL_MAG_FILTER[eSamplerFilter::SAMPLER_FILTER_COUNT] = {
    GL_NEAREST,
    GL_LINEAR,
    GL_LINEAR,
    GL_LINEAR,
    GL_LINEAR,

    GL_NEAREST,
    GL_LINEAR,
    GL_LINEAR,
    GL_LINEAR,
    GL_LINEAR,
};

static constexpr GLenum GL_COMPARISON_FUNCTION[eComparisonFunction::COMPARISON_FUNCTION_COUNT] =
{
   GL_NEVER,
   GL_ALWAYS,

   GL_LESS,
   GL_GREATER,

   GL_LEQUAL,
   GL_GEQUAL,

   GL_NOTEQUAL,
   GL_EQUAL,
};

Sampler* RenderDevice::createSampler( const SamplerDesc& description )
{
    Sampler* sampler = nya::core::allocate<Sampler>( memoryAllocator );
    glCreateSamplers( 1, &sampler->samplerHandle );

    glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_MIN_FILTER, GL_MIN_FILTER[description.filter] );
    glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_MAG_FILTER, GL_MAG_FILTER[description.filter] );

    glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_WRAP_S, GL_SAMPLER_ADDRESS[description.addressU] );
    glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_WRAP_T, GL_SAMPLER_ADDRESS[description.addressV] );
    glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_WRAP_R, GL_SAMPLER_ADDRESS[description.addressW] );

    if ( description.filter == SAMPLER_FILTER_ANISOTROPIC_8 )
        glSamplerParameterf( sampler->samplerHandle, GL_TEXTURE_MAX_ANISOTROPY, 8.0f );
    else if ( description.filter == SAMPLER_FILTER_ANISOTROPIC_16 )
        glSamplerParameterf( sampler->samplerHandle, GL_TEXTURE_MAX_ANISOTROPY, 16.0f );

    if ( description.comparisonFunction != eComparisonFunction::COMPARISON_FUNCTION_ALWAYS ) {
        glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
        glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_COMPARE_FUNC, GL_COMPARISON_FUNCTION[description.comparisonFunction] );
    } else {
        glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_COMPARE_MODE, GL_NONE );
    }

    glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_MIN_LOD, description.minLOD );
    glSamplerParameteri( sampler->samplerHandle, GL_TEXTURE_MAX_LOD, description.maxLOD );

    return sampler;
}

void RenderDevice::destroySampler( Sampler* sampler )
{
    glDeleteSamplers( 1, &sampler->samplerHandle );
}
#endif
