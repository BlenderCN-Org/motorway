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
}
