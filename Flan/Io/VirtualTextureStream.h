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
#include <Graphics/VirtualTexturing/PageIndex.h>
#include <Graphics/VirtualTexturing/PageCacheManager.h>

class FileSystemObject;

struct MipLevelHeader
{
    uint32_t width;
    uint32_t height;
    uint16_t pageCountX;
    uint16_t pageCountY;
};

struct PageEntry
{
    uint64_t pageRelativeOffset;
    uint32_t pageSizeInBytes;
};

struct VirtualTextureStream
{
    fnStringHash_t hashcode;
    uint32_t pageCountX[11];
    uint32_t pageCountY[11];
    std::vector<PageEntry*> levels;
    std::vector<PageEntry> pageEntries;
    FileSystemObject* stream;
};

namespace flan
{
    namespace core
    {
        void LoadVirtualTextureFile( FileSystemObject* stream, VirtualTextureStream& outputStream );
    }
}
