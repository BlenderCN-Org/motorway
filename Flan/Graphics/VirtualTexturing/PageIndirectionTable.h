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

#include <array>

class Texture;
class RenderDevice;
class CommandList;

class PageIndirectionTable
{
public:
            PageIndirectionTable();
            PageIndirectionTable( PageIndirectionTable& ) = default;
            PageIndirectionTable& operator = ( PageIndirectionTable& ) = default;
            ~PageIndirectionTable();

    void    destroy( RenderDevice* renderDevice );
    void    create( RenderDevice* renderDevice );

    void    update( CommandList* cmdList );

private:
    struct IndirectionTexel {
        uint8_t x;
        uint8_t y;
        uint8_t scaleHigh;
        uint8_t scaleLow;
    };

private:
    std::unique_ptr<Texture> pageIndirectionTableTexture;
    std::array<IndirectionTexel*, 11> tableLevels;
    std::unique_ptr<IndirectionTexel[]> tableEntryPool;
};
