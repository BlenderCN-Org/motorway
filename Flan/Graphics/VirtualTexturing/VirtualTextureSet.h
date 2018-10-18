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

class Texture;
class RenderDevice;
class CommandList;
class PageTable;

#include <Io/VirtualTexture.h>


class VirtualTextureSet
{
public:
                VirtualTextureSet();
                VirtualTextureSet( VirtualTextureSet& ) = default;
                VirtualTextureSet& operator = ( VirtualTextureSet& ) = default;
                ~VirtualTextureSet();

    void        addTextureToSet( const VirtualTexture& virtualTexture );
    void        update( PageTable* pageTable );

private:
    std::vector<VirtualTexture> virtualTextures;
};
