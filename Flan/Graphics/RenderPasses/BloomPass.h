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

#include <Core/Factory.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/PipelineState.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/CBufferIndexes.h>
#include <Shared.h>

#include "DownsamplingPass.h"

static fnPipelineMutableResHandle_t AddBloomPass( RenderPipeline* renderPipeline )
{
    renderPipeline->addPipelineSetupPass(
        [&]( RenderPipeline* renderPipeline, RenderPipelineBuilder* renderPipelineBuilder ) {
            auto mip0 = renderPipelineBuilder->getWellKnownResource( FLAN_STRING_HASH( "MainColorRT" ) );

            auto mip1 = AddStabilizedDownsamplingPass( renderPipeline, mip0, 2 );
            auto mip2 = AddWeightedDownsamplingPass( renderPipeline, mip0, 4 );
            auto mip3 = AddWeightedDownsamplingPass( renderPipeline, mip0, 8 );
            auto mip4 = AddWeightedDownsamplingPass( renderPipeline, mip0, 16 );
            auto mip5 = AddWeightedDownsamplingPass( renderPipeline, mip0, 32 );

            mip4 = AddWeightedUpsamplingPass( renderPipeline, mip4, 1.0f, mip5 );
            mip3 = AddWeightedUpsamplingPass( renderPipeline, mip3, 1.0f, mip4 );
            mip2 = AddWeightedUpsamplingPass( renderPipeline, mip2, 1.0f, mip3 );
            mip1 = AddWeightedUpsamplingPass( renderPipeline, mip1, 1.0f, mip2 );
            mip0 = AddWeightedUpsamplingPass( renderPipeline, mip0, 1.0f, mip1 );

            renderPipelineBuilder->registerWellKnownResource( FLAN_STRING_HASH( "BloomRT" ), mip0 );
        }
    );

    return -1;
}

FLAN_REGISTER_RENDERPASS( BloomPass, AddBloomPass )
