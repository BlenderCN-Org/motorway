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
#include "PageResolver.h"

#include <Rendering/RenderTarget.h>
#include <unordered_map>

PageResolver::PageResolver()
    : feedbackRenderTarget( nullptr )
{

}

PageResolver::~PageResolver()
{

}

void PageResolver::destroy( RenderDevice* renderDevice )
{
    feedbackRenderTarget->destroy( renderDevice );
}

void PageResolver::create( RenderDevice* renderDevice )
{
    TextureDescription pageTableDesc;
    pageTableDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    pageTableDesc.format = IMAGE_FORMAT_R8G8B8A8_UINT;
    pageTableDesc.width = 256;
    pageTableDesc.height = 128;
    pageTableDesc.depth = 1;
    pageTableDesc.arraySize = 1;
    pageTableDesc.mipCount = 1;
    pageTableDesc.samplerCount = 1;

    feedbackRenderTarget.reset( new RenderTarget() );
    feedbackRenderTarget->createAsRenderTarget2D( renderDevice, pageTableDesc );
    
    indirectionTexels.resize( 256 * 128 * 4 );
}

void PageResolver::readbackFromGPU( RenderDevice* renderDevice )
{
    feedbackRenderTarget->retrieveTexelsLDR( renderDevice, indirectionTexels );

    using fnPageId_t = uint32_t;

    std::unordered_map<fnPageId_t, uint32_t> pageUsage;
    const int pageCount = 256 * 128 * 4;
    for ( int i = 0; i < pageCount; i += 4 ) {
        // Concat four bytes into a single page id
        fnPageId_t pageIndex = ( indirectionTexels[i] << 24 | indirectionTexels[i + 1] << 16 | indirectionTexels[i + 2] << 8 | indirectionTexels[i + 3] );

        if ( pageIndex != ~0 ) {
            pageUsage[pageIndex]++;
        }
    }
}