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
#include "PageIndirectionTable.h"

#include <Rendering/Texture.h>

PageIndirectionTable::PageIndirectionTable()
    : pageIndirectionTableTexture( nullptr )
    , tableLevels()
    , tableEntryPool( nullptr )
{

}

PageIndirectionTable::~PageIndirectionTable()
{

}

void PageIndirectionTable::destroy( RenderDevice* renderDevice )
{
    pageIndirectionTableTexture->destroy( renderDevice );
}

void PageIndirectionTable::create( RenderDevice* renderDevice )
{
    TextureDescription pageIndirectionTableDesc;
    pageIndirectionTableDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    pageIndirectionTableDesc.format = IMAGE_FORMAT_R32_SINT;
    pageIndirectionTableDesc.width = 2048;
    pageIndirectionTableDesc.height = 2048;
    pageIndirectionTableDesc.depth = 1;
    pageIndirectionTableDesc.arraySize = 1;
    pageIndirectionTableDesc.mipCount = 11;
    pageIndirectionTableDesc.samplerCount = 1;

    pageIndirectionTableTexture.reset( new Texture() );
    pageIndirectionTableTexture->createAsTexture2D( renderDevice, pageIndirectionTableDesc );

    int totalTableEntries = 0;
    uint32_t pageCount = 128;
    for ( int l = 0; l < 11; ++l ) {
        totalTableEntries += ( pageCount * pageCount );

        pageCount >>= 1;
    }

    tableEntryPool.reset( new IndirectionTexel[totalTableEntries] );

    totalTableEntries = 0; 
    pageCount = 128;
    for ( int l = 0; l < 11; ++l ) {
        tableLevels[l] = tableEntryPool.get() + totalTableEntries;
        totalTableEntries += ( pageCount * pageCount );

        pageCount >>= 1;
    }

    const auto scale = ( 128 * 16 ) >> 1;

    pageCount = 128;
    for ( int l = 0; l < 11; ++l ) {
        for ( int e = 0; e < pageCount * pageCount; e++ ) {
            IndirectionTexel& texel = tableLevels[l][e];
            texel.x = 0;
            texel.y = 0;
            texel.scaleLow = scale & 0xff;
            texel.scaleHigh = scale >> 8;
        }

        pageCount >>= 1;
    }
}

void PageIndirectionTable::update( CommandList* cmdList )
{
    for ( int l = 11; l >= 0; --l ) {
        for ( int p = 0; p < 128 * 128; p++ ) {
            
        }

        TextureCopyBox copyBox;
        copyBox.x = 0;
        copyBox.y = 0;
        copyBox.arrayIndex = 0;
        copyBox.mipLevel = l;

        uint32_t levelSize = 2048 >> l;
        pageIndirectionTableTexture->updateSubresource( cmdList, copyBox, levelSize, levelSize, 1, tableLevels[l] );
    }
}
