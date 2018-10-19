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
#include "PageTable.h"

#include "PageConstants.h"

#include <Rendering/Texture.h>

PageTable::PageTable()
    : pageTableTexture( nullptr )
{

}

PageTable::~PageTable()
{

}

void PageTable::destroy( RenderDevice* renderDevice )
{
    pageTableTexture->destroy( renderDevice );
}

void PageTable::create( RenderDevice* renderDevice )
{
    TextureDescription pageTableDesc;
    pageTableDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
    pageTableDesc.format = IMAGE_FORMAT_R16G16B16A16_FLOAT;
    pageTableDesc.width = ( PAGE_SIZE_IN_PIXELS * PAGE_TABLE_PAGE_COUNT );
    pageTableDesc.height = ( PAGE_SIZE_IN_PIXELS * PAGE_TABLE_PAGE_COUNT ) / 2;
    pageTableDesc.depth = 1;
    pageTableDesc.arraySize = 1;
    pageTableDesc.mipCount = PAGE_TABLE_MIP_COUNT;
    pageTableDesc.samplerCount = 1;

    pageTableTexture.reset( new Texture() );
    pageTableTexture->createAsTexture2D( renderDevice, pageTableDesc );
}

void PageTable::uploadPage( CommandList* cmdList, const uint32_t x, const uint32_t y, const uint32_t mipLevel, const void* pageData )
{
    TextureCopyBox copyBox;
    copyBox.x = ( x * PAGE_SIZE_IN_PIXELS );
    copyBox.y = ( y * PAGE_SIZE_IN_PIXELS );
    copyBox.mipLevel = mipLevel;
    copyBox.arrayIndex = 0;

    pageTableTexture->updateSubresource( cmdList, copyBox, PAGE_SIZE_IN_PIXELS, PAGE_SIZE_IN_PIXELS, 4, pageData );
}

void PageTable::bind( CommandList* cmdList, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    pageTableTexture->bind( cmdList, bindingIndex, shaderStagesToBindTo );
}
