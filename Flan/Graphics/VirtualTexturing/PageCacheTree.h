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

#include "PageConstants.h"
#include "PageIndex.h"
#include "PageCacheEntry.h"

#include <vector>
#include <array>

class PageCacheTree
{
public:
                        PageCacheTree();
                        PageCacheTree( PageCacheTree& ) = default;
                        PageCacheTree& operator = ( PageCacheTree& ) = default;
                        ~PageCacheTree();

    void                addToCache( const int level, const int x, const int y, CacheEntry* entry );
    CacheEntry*         retrieveFromCache( const int level, const int x, const int y ) const;

private:
    std::vector<CacheEntry *>       cacheEntryPtrPool;
    std::array<CacheEntry**, 11>    levels;

    // Per-level page counts:
    std::array<int, 11> numPagesX;
    std::array<int, 11> numPagesY;
};
