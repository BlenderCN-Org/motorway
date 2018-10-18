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
#include "PageStreaming.h"

#include <Io/VirtualTexture.h>
#include <Core/TaskManager.h>

PageStreaming::PageStreaming()
    : taskManager( nullptr )
{

}

PageStreaming::~PageStreaming()
{

}

void PageStreaming::create( TaskManager* taskManagerInstance )
{
    taskManager = taskManagerInstance;
}

void PageStreaming::update( CommandList* cmdList )
{
    // Process uploads (not sure if the lock is required at this point?)
    {
        std::unique_lock<std::mutex> pendingUpload = std::unique_lock<std::mutex>( pageUploadsMutex );

        while ( !pageUploads.empty() ) {
            auto& pageUpload = pageUploads.back();

            auto pageTable = allocatedPageTables.back();
            pageTable->uploadPage( cmdList, pageUpload.x, pageUpload.y, pageUpload.mipLevel, pageUpload.data );

            delete[] pageUpload.data;

            pageUploads.pop();
        }
    }
}

PageTable* PageStreaming::allocatePageTable( RenderDevice* renderDevice )
{   
    PageTable* pageTable = new PageTable();
    pageTable->create( renderDevice );

    allocatedPageTables.push_back( pageTable );

    return allocatedPageTables.back();
}

void PageStreaming::registerVirtualTexture( VirtualTexture* virtualTexture )
{
    virtualTextures.push_back( virtualTexture );
}

void PageStreaming::addPageRequest( const fnPageId_t pageIndex )
{
    taskManager->addTask( [&]() {
        const int level = ( ( pageIndex & 0x00FF0000 ) >> 16 );
        const int textureIndex = ( ( pageIndex & 0xFF000000 ) >> 24 );

        auto virtualTexture = virtualTextures[textureIndex];
        const std::size_t mipSize = ( 128 >> level );

        // TODO Correct allocators for pages
        uint8_t* pageData = new uint8_t[mipSize * mipSize * 4 * sizeof( float )];
        virtualTexture->loadPage( pageIndex, pageData );

        // Defer upload until the rendering start
        {
            std::unique_lock<std::mutex> pendingUpload = std::unique_lock<std::mutex>( pageUploadsMutex );
            pageUploads.push( { ( pageIndex & 0x000000FF ), ( ( pageIndex & 0x0000FF00 ) >> 8 ), level, 0, pageData } );
        }
    } );
}
