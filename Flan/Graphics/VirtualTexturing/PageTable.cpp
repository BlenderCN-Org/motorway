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
    pageTableDesc.width = 16384;
    pageTableDesc.height = 8192;
    pageTableDesc.depth = 1;
    pageTableDesc.arraySize = 1;
    pageTableDesc.mipCount = 1;
    pageTableDesc.samplerCount = 1;

    pageTableTexture.reset( new Texture() );
    pageTableTexture->createAsTexture2D( renderDevice, pageTableDesc );
}
