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
#include "PageCacheManager.h"

PageCacheManager::PageCacheManager()
    : dummyEntry{ 0 }
    , mru( nullptr )
    , lru( nullptr )
    , cachePageTree()
{
    
}

PageCacheManager::~PageCacheManager()
{
    mru = nullptr;
    lru = nullptr;
}

bool PageCacheManager::isPageCached( const fnPageId_t pageIndex )
{
    const int level = ( ( pageIndex & 0x00FF0000 ) >> 16 );
    const int x = ( ( pageIndex & 0x0000FF00 ) >> 8 );
    const int y = ( pageIndex & 0x000000FF );

    CacheEntry* entry = cachePageTree.retrieveFromCache( level, x, y );

    // Not in cache, submit a cache request for next frame
    if ( entry == nullptr )  {
        cachePageTree.addToCache( level, x, y, &dummyEntry );
        return false;
    }

    // Loading in progress (missing one or several mip levels; takes the highest one available)
    if ( entry == &dummyEntry )  {
        return true;
    }

    // In cache; update the head of the cache array if needed
    if ( entry != mru ) {
        if ( entry->next != nullptr ) {
            entry->prev->next = entry->next;
            entry->next->prev = entry->prev;
        } else {
            entry->prev->next = nullptr;
            lru = entry->prev;
        }

        entry->prev = nullptr;
        entry->next = mru;
        mru->prev = entry;

        mru = entry;
    }

    return true;
}
