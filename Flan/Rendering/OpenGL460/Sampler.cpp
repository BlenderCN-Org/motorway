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

#if FLAN_GL460
#include "Sampler.h"

#include <Rendering/SamplerAddresses.h>
#include <Rendering/SamplerFilters.h>

#include "ComparisonFunctions.h"

using namespace flan::rendering;

static constexpr GLenum GL_SAMPLER_ADDRESS[eSamplerAddress::SamplerAddress_COUNT] = {
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_CLAMP_TO_EDGE,
    GL_CLAMP_TO_BORDER,
    GL_MIRROR_CLAMP_TO_EDGE
};

static constexpr GLenum GL_MIN_FILTER[eSamplerFilter::SamplerFilter_COUNT] = {
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

static constexpr GLenum GL_MAG_FILTER[eSamplerFilter::SamplerFilter_COUNT] = {
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

NativeSamplerObject* flan::rendering::CreateSamplerImpl( NativeRenderContext* nativeRenderContext, const SamplerDesc& description )
{
    NativeSamplerObject* sampler = new NativeSamplerObject();

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

void flan::rendering::DestroySamplerImpl( NativeRenderContext* nativeRenderContext, NativeSamplerObject* samplerObject )
{
    glDeleteSamplers( 1, &samplerObject->samplerHandle );
}

void flan::rendering::BindSamplerCmdImpl( NativeCommandList* nativeCmdList, NativeSamplerObject* samplerObject, const uint32_t bindingIndex )
{
    glBindSampler( bindingIndex, samplerObject->samplerHandle );
}
#endif
