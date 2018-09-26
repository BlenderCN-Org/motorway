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

#include "ImageFormat.h"

struct TextureDescription
{
    TextureDescription()
        : dimension( DIMENSION_UNKNOWN )
        , format( IMAGE_FORMAT_UNKNOWN )
        , width( 0 )
        , height( 0 )
        , depth( 0 )
        , arraySize( 1 )
        , mipCount( 1 )
        , samplerCount( 1 )
        , flags{ 0 }
    {

    }

    enum Dimension : uint32_t {
        DIMENSION_UNKNOWN = 0,
        DIMENSION_BUFFER,
        DIMENSION_TEXTURE_1D,
        DIMENSION_TEXTURE_2D,
        DIMENSION_TEXTURE_3D,
    } dimension;

    eImageFormat    format;

    unsigned int    width;
    unsigned int    height;
    unsigned int    depth;
    unsigned int    arraySize;

    unsigned int    mipCount;

    struct Flagset {
        uint32_t     isCubeMap : 1;
        uint32_t     isDepthResource : 1;
        uint32_t     useHardwareMipGen : 1;
        uint32_t     useMultisamplePattern : 1;

        const bool operator == ( const TextureDescription::Flagset& flagset ) const
        {
            return isCubeMap == flagset.isCubeMap
                && isDepthResource == flagset.isDepthResource
                && useHardwareMipGen == flagset.useHardwareMipGen
                && useMultisamplePattern == flagset.useMultisamplePattern;
        }
    } flags;

    unsigned int    samplerCount;

    const bool operator == ( const TextureDescription& desc ) const
    {
        return desc.width == width
            && desc.height == height
            && desc.depth == depth
            && desc.format == format
            && desc.flags == flags
            && desc.mipCount == mipCount
            && desc.samplerCount == samplerCount;
    }
};

namespace flan
{
    namespace rendering
    {
        static unsigned int ComputeMipCount( const unsigned int width, const unsigned int height )
        {
            unsigned int mipCount = 0;
            auto mipWidth = width, mipHeight = height;
            
            while ( mipWidth > 0 && mipHeight > 0 ) {
                mipCount++;

                mipWidth >>= 1;
                mipHeight >>= 1;
            }

            return mipCount;
        }
    }
}
