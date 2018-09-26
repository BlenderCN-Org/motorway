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

#include <Rendering/RenderDevice.h>
#include <Rendering/PipelineState.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderPipeline.h>
#include <Graphics/CBufferIndexes.h>
#include <Shared.h>

#include <Graphics/TextureSaveTools.h>

static fnPipelineMutableResHandle_t AddSaveTexturePass( RenderPipeline* renderPipeline, RenderDevice* renderDevice, fnPipelineMutableResHandle_t input = -1 )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Save Texture",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.input[0] = renderPipelineBuilder->readRenderTarget( input );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            auto inputRenderTarget = renderPipelineResources->getRenderTarget( passData.input[0] );

            std::vector<uint8_t> texels;
            inputRenderTarget->retrieveTexelsLDR( renderDevice, texels );

            FLAN_COUT << "Done";
        }
    );

    return RenderPass.output[0];
}
