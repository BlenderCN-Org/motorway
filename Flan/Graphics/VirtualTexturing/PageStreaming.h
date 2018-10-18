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
class VirtualTexture;
class TaskManager;

#include <vector>
#include <queue>

#include "PageTable.h"
#include "PageIndex.h"

class PageStreaming
{
public:
                PageStreaming();
                PageStreaming( PageStreaming& ) = default;
                PageStreaming& operator = ( PageStreaming& ) = default;
                ~PageStreaming();

    void        create( TaskManager* taskManagerInstance );
    
    void        update( CommandList* cmdList );
    PageTable*  allocatePageTable( RenderDevice* renderDevice );

    void        registerVirtualTexture( VirtualTexture* virtualTexture );
    void        unregisterVirtualTexture( VirtualTexture* virtualTexture );

    void        addPageRequest( const fnPageId_t pageIndex );

private:
    struct PageUpload
    {
        uint32_t    x;
        uint32_t    y;
        void*       data;
    };

private:
    TaskManager*                    taskManager;

    std::vector<PageTable*>         allocatedPageTables;
    std::vector<VirtualTexture*>    virtualTextures;

    std::queue<PageUpload>          pageUploads;
};
