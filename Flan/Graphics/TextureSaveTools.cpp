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
#include "Shared.h"
#include "TextureSaveTools.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <Io/DirectDrawSurface.h>

void flan::graphics::SaveTextureLDR( const std::string& diskPath, const unsigned int width, const unsigned int height, const unsigned int channelCount, const std::vector<uint8_t>& texels )
{
    //stbi_write_tga( diskPath.c_str(), width, height, 4, texels.data() );
    stbi_write_jpg( diskPath.c_str(), width, height, channelCount, texels.data(), 0 );
}

void flan::graphics::SaveTextureHDR( const std::string& diskPath, const unsigned int width, const unsigned int height, const unsigned int channelCount, const std::vector<float>& texels )
{
    stbi_write_hdr( diskPath.c_str(), width, height, channelCount, texels.data() );
}
