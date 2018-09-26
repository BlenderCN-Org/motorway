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

#include <vector>
#include "TextStreamHelpers.h"

struct FontDescriptor
{
    fnString_t  Name;
    uint32_t    AtlasWidth;
    uint32_t    AtlasHeight;

    struct Glyph
    {
        int32_t PositionX;
        int32_t PositionY;
        int32_t Width;
        int32_t Height;
        int32_t OffsetX;
        int32_t OffsetY;
        int32_t AdvanceX;
    };

    std::vector<Glyph> Glyphes;
};

namespace flan
{
    namespace core
    {
        void    LoadFontFile( FileSystemObject* file, FontDescriptor& data );
    }
}
