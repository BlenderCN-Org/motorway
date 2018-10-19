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
class TaskManager;
class PageIndirectionTable;
class PageCacheManager;

struct VirtualTexture;

#include <vector>
#include <queue>
#include <mutex>

#include "PageTable.h"
#include "PageIndex.h"

class PageStreaming
{
public:
                        PageStreaming();
                        PageStreaming( PageStreaming& ) = default;
                        PageStreaming& operator = ( PageStreaming& ) = default;
                        ~PageStreaming();

    void                create( TaskManager* taskManagerInstance );
    
    void                update( CommandList* cmdList );
    PageTable*          allocatePageTable( RenderDevice* renderDevice );

    void                registerVirtualTexture( VirtualTexture* virtualTexture );

    void                addPageRequest( const fnPageId_t pageIndex );

    PageCacheManager*   getTextureCache( const uint32_t textureIndex );

private:
    struct PageUpload
    {
        uint32_t    x;
        uint32_t    y;
        uint32_t    mipLevel;
        uint32_t    texIndex;
        void*       data;
    };

private:
    TaskManager*                        taskManager;

    std::vector<PageTable*>             allocatedPageTables;
    std::vector<VirtualTexture*>        virtualTextures;
    std::vector<PageCacheManager*>      pageCaches;
    std::vector<PageIndirectionTable*>  pageIndirectionTable;

    std::queue<PageUpload>              pageUploads;
    std::mutex                          pageUploadsMutex;
};
