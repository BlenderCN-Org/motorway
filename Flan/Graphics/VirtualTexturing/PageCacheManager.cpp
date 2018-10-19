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
    , cacheEntryPool{}
{
    for ( int x = 0; x < PAGE_TABLE_PER_LINE_PAGE_COUNT; x++ ) {
        for ( int y = 0; y < PAGE_TABLE_PER_LINE_PAGE_COUNT; y++ ) {
            const auto pageIndex = x * PAGE_TABLE_PER_LINE_PAGE_COUNT + y;

            cacheEntryPool[pageIndex].prev = nullptr;
            cacheEntryPool[pageIndex].next = nullptr;
            cacheEntryPool[pageIndex].x = x;
            cacheEntryPool[pageIndex].y = y;
            cacheEntryPool[pageIndex].pageId = INVALID_PAGEID;
        }
    }

    purgeCache();
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

void PageCacheManager::accomodatePage( const fnPageId_t pageIndex, uint32_t& pageX, uint32_t& pageY )
{
    const int x = ( ( pageIndex & 0x000000FF ) >> 0 );
    const int y = ( ( pageIndex & 0x0000FF00 ) >> 8 );
    const int level = ( ( pageIndex & 0x00FF0000 ) >> 16 );

    CacheEntry* entry = allocate();

    cachePageTree.addToCache( level, x, y, entry );
    entry->pageId = pageIndex;

    pageX = entry->x;
    pageY = entry->y;
}

void PageCacheManager::purgeCache()
{
    for ( int i = 0; i < PAGE_TABLE_PAGE_COUNT; i++ ) {
        cacheEntryPool[i].prev = ( i > 0 ) ? &cacheEntryPool[i - 1] : nullptr;
        cacheEntryPool[i].next = ( i < ( PAGE_TABLE_PAGE_COUNT - 1 ) ) ? &cacheEntryPool[i + 1] : nullptr;
        cacheEntryPool[i].pageId = INVALID_PAGEID;
    }

    mru = &cacheEntryPool[0];
    lru = &cacheEntryPool[( PAGE_TABLE_PAGE_COUNT - 1 )];
}

CacheEntry* PageCacheManager::allocate()
{
    if ( lru->pageId != INVALID_PAGEID ) {
        const int x = ( ( lru->pageId & 0x000000FF ) >> 0 );
        const int y = ( ( lru->pageId & 0x0000FF00 ) >> 8 );
        const int level = ( ( lru->pageId & 0x00FF0000 ) >> 16 );

        cachePageTree.addToCache( level, x, y, nullptr );
    }

    CacheEntry* entry = lru;
    entry->prev->next = nullptr;
    lru = entry->prev;

    entry->prev = nullptr;
    entry->next = mru;
    mru->prev = entry;
    mru = entry;

    return entry;
}
