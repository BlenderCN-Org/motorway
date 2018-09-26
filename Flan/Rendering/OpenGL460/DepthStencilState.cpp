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
#include "DepthStencilState.h"

#include <Rendering/StencilOperation.h>

#include "ComparisonFunctions.h"

using namespace flan::rendering;

static constexpr GLenum GL_STENCIL_OPERATION[eStencilOperation::StencilOperation_COUNT] =
{
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,
    GL_INCR_WRAP,
    GL_DECR,
    GL_DECR_WRAP,
    GL_INVERT
};

NativeDepthStencilStateObject* flan::rendering::CreateDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, const DepthStencilStateDesc& description )
{
    NativeDepthStencilStateObject* depthStencilState = new NativeDepthStencilStateObject();
    depthStencilState->enableDepthTest = description.enableDepthTest;
    depthStencilState->enableDepthWrite = ( description.enableDepthWrite ) ? GL_TRUE : GL_FALSE;

    depthStencilState->enableStencilTest = description.enableStencilTest;

    depthStencilState->enableDepthBoundsTest = description.enableDepthBoundsTest;
    depthStencilState->depthBoundsMin = description.depthBoundsMin;
    depthStencilState->depthBoundsMax = description.depthBoundsMax;
    depthStencilState->depthComparisonFunc = GL_COMPARISON_FUNCTION[description.depthComparisonFunc];

    depthStencilState->stencilRefValue = description.stencilRefValue;
    depthStencilState->stencilWriteMask = description.stencilWriteMask;
    depthStencilState->stencilReadMask = description.stencilReadMask;

    depthStencilState->front.comparisonFunc = GL_COMPARISON_FUNCTION[description.front.comparisonFunc];
    depthStencilState->front.passOp = GL_STENCIL_OPERATION[description.front.passOp];
    depthStencilState->front.failOp = GL_STENCIL_OPERATION[description.front.failOp];
    depthStencilState->front.zFailOp = GL_STENCIL_OPERATION[description.front.zFailOp];

    depthStencilState->back.comparisonFunc = GL_COMPARISON_FUNCTION[description.back.comparisonFunc];
    depthStencilState->back.passOp = GL_STENCIL_OPERATION[description.back.passOp];
    depthStencilState->back.failOp = GL_STENCIL_OPERATION[description.back.failOp];
    depthStencilState->back.zFailOp = GL_STENCIL_OPERATION[description.back.zFailOp];

    return depthStencilState;
}

void flan::rendering::DestroyDepthStencilStateImpl( NativeRenderContext* nativeRenderContext, NativeDepthStencilStateObject* depthStencilStateObject )
{
    depthStencilStateObject = {};
}

void flan::rendering::BindDepthStencilStateCmdImpl( NativeCommandList* nativeCommandList, NativeDepthStencilStateObject* depthStencilStateObject )
{
    if ( depthStencilStateObject->enableDepthTest ) {
         glEnable( GL_DEPTH_TEST );
         glDepthFunc( depthStencilStateObject->depthComparisonFunc );
    } else {
        glDisable( GL_DEPTH_TEST );
    }

    glDepthMask( depthStencilStateObject->enableDepthWrite );

    if ( depthStencilStateObject->enableDepthBoundsTest ) {
        /*glEnable( GL_DEPTH_BOUNDS );
        glDepthBoundsEXT( depthStencilStateObject->depthBoundsMin, depthStencilStateObject->depthBoundsMax );*/
    }

    if ( depthStencilStateObject->enableStencilTest ) {
        glEnable( GL_STENCIL_TEST );

        glStencilFuncSeparate( GL_FRONT, depthStencilStateObject->front.comparisonFunc, depthStencilStateObject->stencilRefValue, depthStencilStateObject->stencilWriteMask );
        glStencilOpSeparate( GL_FRONT, depthStencilStateObject->front.failOp, depthStencilStateObject->front.zFailOp, depthStencilStateObject->front.passOp );

        glStencilFuncSeparate( GL_BACK, depthStencilStateObject->back.comparisonFunc, depthStencilStateObject->stencilRefValue, depthStencilStateObject->stencilWriteMask );
        glStencilOpSeparate( GL_BACK, depthStencilStateObject->back.failOp, depthStencilStateObject->back.zFailOp, depthStencilStateObject->back.passOp );

        glStencilMask( depthStencilStateObject->stencilReadMask );
    } else {
        glDisable( GL_STENCIL_TEST );
    }
}
#endif
