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
#include "RasterizerState.h"

#include <Rendering/FillModes.h>
#include <Rendering/CullModes.h>

using namespace flan::rendering;

static constexpr GLenum GL_FILL_MODE[eFillMode::FillMode_COUNT] = {
    GL_FILL,
    GL_LINE,
};

static constexpr GLenum GL_CULL_MODE[eCullMode::CullMode_COUNT] = {
    GL_NONE,
    GL_FRONT,
    GL_BACK,
    GL_FRONT_AND_BACK,
};

NativeRasterizerStateObject*  flan::rendering::CreateRasterizerStateImpl( NativeRenderContext* nativeRenderContext, const RasterizerStateDesc& description )
{
    NativeRasterizerStateObject* rasterizerState = new NativeRasterizerStateObject();
    rasterizerState->cullMode = GL_CULL_MODE[description.cullMode];
    rasterizerState->fillMode = GL_FILL_MODE[description.fillMode];

    rasterizerState->depthBias = description.depthBias;
    rasterizerState->slopeScale = description.slopeScale;
    rasterizerState->depthBiasClamp = description.depthBiasClamp;
    rasterizerState->triangleWielding = description.useTriangleCCW ? GL_CCW : GL_CW;

    return rasterizerState;
}

void flan::rendering::DestroyRasterizerStateImpl( NativeRenderContext* nativeRenderContext, NativeRasterizerStateObject* rasterizerStateObject )
{
    rasterizerStateObject = {};
}

void flan::rendering::BindRasterizerStateCmdImpl( NativeCommandList* nativeCmdList, NativeRasterizerStateObject* rasterizerStateObject )
{
    glPolygonMode( GL_FRONT_AND_BACK, rasterizerStateObject->fillMode );

    glFrontFace( rasterizerStateObject->triangleWielding );

    if ( rasterizerStateObject->cullMode != eCullMode::CULL_MODE_NONE ) {
        glEnable( GL_CULL_FACE );
        glCullFace( rasterizerStateObject->cullMode );
    } else {
        glDisable( GL_CULL_FACE );
    }

    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( rasterizerStateObject->depthBias, 1.0f );
}
#endif
