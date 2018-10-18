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

#include "PageIndex.h"

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
        fnPageId_t pageIndex = MakePageId( indirectionTexels[i], indirectionTexels[i + 1], indirectionTexels[i + 2], indirectionTexels[i + 3] );

        if ( pageIndex != INVALID_PAGEID ) {
            pageUsage[pageIndex]++;
        }
    }

    std::vector<fnPageId_t> sortedPages( pageUsage.size() );
    for ( const auto& usedPage : pageUsage ) {
        sortedPages.push_back( usedPage.first );
    }

    // Sort page by its priority (highest mip goes first; the most used one goes first aswell)
    std::sort( std::begin( sortedPages ), std::end( sortedPages ),
        [&]( fnPageId_t a, fnPageId_t b ) -> bool {
        const int aLevel = ( ( a & 0x00FF0000 ) >> 16 );
        const int bLevel = ( ( b & 0x00FF0000 ) >> 16 );

        if ( aLevel > bLevel ) {
            return true;
        }

        if ( aLevel == bLevel ) {
            return pageUsage[a] > pageUsage[b];
        }

        return false;
    } );

    size_t newRequests = 0;
    for ( size_t r = 0; r < sortedPages.size(); ++r ) {
        const fnPageId_t requestId = sortedPages[r];
        const size_t textureIndex = ( ( requestId & 0xFF000000 ) >> 24 );

        PageCacheMgr* pageCache = registeredTextures[textureIndex]->getPageCache();

        newRequests += processPageRequest( requestId, *pageCache );
    }
}
