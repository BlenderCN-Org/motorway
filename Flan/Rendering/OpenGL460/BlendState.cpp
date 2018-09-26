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
#include "BlendState.h"
#include "CommandList.h"

#include <Rendering/BlendOperations.h>
#include <Rendering/BlendSources.h>

using namespace flan::rendering;

static constexpr GLenum GL_BLEND_SOURCE[eBlendSource::BlendSource_COUNT] = {
    GL_ZERO,
    GL_ONE,

    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,

    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,

    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,

    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,

    0x0000,

    0x0000,
    0x0000,
};

static constexpr GLenum GL_BLEND_OPERATION[eBlendOperation::BlendOperation_COUNT] = {
    GL_FUNC_ADD,
    GL_FUNC_SUBTRACT,
    GL_MIN,
    GL_MAX,
};

NativeBlendStateObject* flan::rendering::CreateBlendStateImpl( NativeRenderContext* nativeRenderContext, const BlendStateDesc& description )
{
    NativeBlendStateObject* blendState = new NativeBlendStateObject();

    blendState->writeMask[0] = static_cast<GLboolean>( description.writeMask[0] );
    blendState->writeMask[1] = static_cast<GLboolean>( description.writeMask[1] );
    blendState->writeMask[2] = static_cast<GLboolean>( description.writeMask[2] );
    blendState->writeMask[3] = static_cast<GLboolean>( description.writeMask[3] );

    blendState->enableBlend = description.enableBlend;
    blendState->useSeperateAlpha = description.useSeperateAlpha;
    blendState->enableAlphaToCoverage = description.enableAlphaToCoverage;
    blendState->sampleMask = description.sampleMask;

    blendState->blendConfColor.source = GL_BLEND_SOURCE[description.blendConfColor.source];
    blendState->blendConfColor.operation = GL_BLEND_OPERATION[description.blendConfColor.operation];
    blendState->blendConfColor.dest = GL_BLEND_SOURCE[description.blendConfColor.dest];

    blendState->blendConfAlpha.source = GL_BLEND_SOURCE[description.blendConfAlpha.source];
    blendState->blendConfAlpha.operation = GL_BLEND_OPERATION[description.blendConfAlpha.operation];
    blendState->blendConfAlpha.dest = GL_BLEND_SOURCE[description.blendConfAlpha.dest];

    return blendState;
}

void flan::rendering::DestroyBlendStateImpl( NativeRenderContext* nativeRenderContext, NativeBlendStateObject* blendStateObject )
{
    // No handle to destroy; simply set the struct to null
    blendStateObject = {};
}

void flan::rendering::BindBlendStateCmdImpl( NativeCommandList* nativeCmdList, NativeBlendStateObject* blendStateObject )
{
    if ( blendStateObject->enableBlend ) {
        glEnable( GL_BLEND );

        glColorMask( blendStateObject->writeMask[0], blendStateObject->writeMask[1], blendStateObject->writeMask[2], blendStateObject->writeMask[3] );
       // glBlendColor( 1.0f, 1.0f, 1.0f, 1.0f );
        glBlendEquationSeparate( blendStateObject->blendConfColor.operation, blendStateObject->blendConfAlpha.operation );
        glBlendFuncSeparate( blendStateObject->blendConfColor.source,
            blendStateObject->blendConfColor.dest,
            blendStateObject->blendConfAlpha.source,
            blendStateObject->blendConfAlpha.dest );

    } else {
        glDisable( GL_BLEND );
        glColorMask( 1, 1, 1, 1 );
    }

    if ( blendStateObject->enableAlphaToCoverage ) {
        glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
        glSampleCoverage( static_cast<float>( blendStateObject->sampleMask ), GL_FALSE );
    } else {
        glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );
    }
}
#endif
