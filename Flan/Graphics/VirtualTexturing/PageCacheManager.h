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
#include "PageCacheTree.h"

class PageCacheManager
{
public:
                PageCacheManager();
                PageCacheManager( PageCacheManager& ) = default;
                PageCacheManager& operator = ( PageCacheManager& ) = default;
                ~PageCacheManager();

    bool        isPageCached( const fnPageId_t pageIndex );
    void        accomodatePage( const fnPageId_t pageIndex, uint32_t& pageX, uint32_t& pageY );
    void        purgeCache();

private:
    CacheEntry      dummyEntry;

    CacheEntry*     mru;
    CacheEntry*     lru;

    PageCacheTree   cachePageTree;
    std::array<CacheEntry, PAGE_TABLE_PAGE_COUNT> cacheEntryPool;

private:
    CacheEntry*     allocate();
};
