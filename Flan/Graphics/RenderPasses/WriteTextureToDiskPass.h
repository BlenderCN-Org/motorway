/*
    Copyright (C) 2018 Team Horsepower
    https://github.com/ProjectHorsepower

    This file is part of Project Horsepower source code.

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
#include <Rendering/Direct3D11/CommandList.h>

#include <Rendering/CommandList.h>
#include <Rendering/Direct3D11/RenderContext.h>
#include <Rendering/Direct3D11/Texture.h>

#include <DirectXTex/DirectXTex.h>

static void AddWriteBufferedTextureToDiskPass( RenderPipeline* renderPipeline, RenderDevice* renderDevice, const fnPipelineResHandle_t renderTarget, const fnString_t& filename = FLAN_STRING( "" ) )
{
    auto RenderPass = renderPipeline->addRenderPass(
        "Write Buffered Texture To Disk",
        [&]( RenderPipelineBuilder* renderPipelineBuilder, RenderPassData& passData ) {
            passData.input[0] = renderPipelineBuilder->readRenderTarget( renderTarget );
        },
        [=]( CommandList* cmdList, const RenderPipelineResources* renderPipelineResources, const RenderPassData& passData ) {
            // Get Input Target
            auto texture = renderPipelineResources->getRenderTarget( passData.input[0] );

#if FLAN_D3D11
            // Save the texture using DirectXTex
            // TODO Make this API independant
            DirectX::ScratchImage textureToSave;
            auto test1 = DirectX::CaptureTexture( renderDevice->getNativeRenderContext()->nativeDevice, renderDevice->getNativeRenderContext()->nativeDeviceContext, texture->getNativeTextureObject()->textureResource, textureToSave );
            auto test = DirectX::SaveToDDSFile( textureToSave.GetImages(),
                                    textureToSave.GetImageCount(),
                                    textureToSave.GetMetadata(),
                                    static_cast<DWORD>( 0 ),
                                    ( FLAN_STRING( "./data/Textures/" ) + ( ( filename.empty() ) ? FLAN_STRING( "PipelineResource_" ) + std::to_wstring( passData.input[0] ) : filename ) + FLAN_STRING( ".dds" ) ).c_str() );
#endif
        }
    );
}
