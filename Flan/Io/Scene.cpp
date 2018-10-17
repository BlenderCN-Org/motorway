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
#include "Scene.h"

#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderableEntityManager.h>
#include "TextStreamHelpers.h"
#include <Framework/Scene.h>
#include <Framework/SceneNodes/EmptySceneNode.h>
#include <Core/SerializationHelpers.h>
#include <Core/LocaleHelpers.h>

//#include <Physics/RigidBody.h>

static constexpr uint32_t SCENE_MAGIC_V1 = 0x314E4F4C; // LON1
static constexpr uint32_t TREE_MAGIC = 0x45455254; // TREE

void Io_ReadSceneFile( FileSystemObject* file, GraphicsAssetManager* graphicsAssetManager, RenderableEntityManager* renderableEntityManager, Scene& scene )
{
    uint32_t magic = 0;

    file->read( ( uint8_t* )&magic, sizeof( uint32_t ) );

    if ( magic != SCENE_MAGIC_V1 ) {
        FLAN_CERR << " >> " << file->getFilename() << " : invalid header magic!" << std::endl;
        return;
    }

    uint32_t size = 0;
    file->read( ( uint8_t* )&size, sizeof( uint32_t ) );

    uint32_t flags = 0;
    file->read( ( uint8_t* )&flags, sizeof( uint32_t ) );

    fnString_t sceneName;
    flan::core::ReadString( file, sceneName );
    scene.setSceneName( flan::core::WideStringToString( sceneName ) );

    // TODO This should not be necessary...
    // Skip padding and seek to the next block offset
    int32_t streamPos = ( file->tell() % 16 );
    if ( streamPos <= 0 ) streamPos = 16;
    file->skip( 16 - streamPos );

    while ( file->tell() < size ) {
        uint32_t blockMagic = 0x0;
        file->read( ( uint8_t* )&blockMagic, sizeof( uint32_t ) );

        // For modularity sake, we check the block magic
        switch ( blockMagic ) {
        case TREE_MAGIC: {
            uint32_t nodeCount = 0;
            uint32_t treeFlags = 0;
            uint32_t reserved = 0;

            file->read( ( uint8_t* )&nodeCount, sizeof( uint32_t ) );
            file->read( ( uint8_t* )&treeFlags, sizeof( uint32_t ) );
            file->read( ( uint8_t* )&reserved, sizeof( uint32_t ) );

            for ( uint32_t i = 0; i < nodeCount; i++ ) {
                uint32_t contentHashcode = 0;
                fnStringHash_t parentHashcode = 0;

                FLAN_DESERIALIZE_VARIABLE( file, contentHashcode );
                FLAN_DESERIALIZE_VARIABLE( file, parentHashcode );

                fnString_t nodeName;
                flan::core::ReadString( file, nodeName );
                auto Name = flan::core::WideStringToString( nodeName );

                auto node = Factory<SceneNode*, Scene*, fnStringHash_t, std::string&>::tryBuildWithHashcode( contentHashcode, &scene, parentHashcode, Name );

                // EmptySceneNode have no hashcode
                if ( node == nullptr ) {
                    node = scene.createEmptyNode( Name, scene.findNodeByHashcode( parentHashcode ) );
                }

                node->deserialize( file, graphicsAssetManager, renderableEntityManager );

                // Skip padding and seek to the next block offset
                int32_t streamPos = ( file->tell() % 16 );
                if ( streamPos <= 0 ) streamPos = 16;
                file->skip( 16 - streamPos );
            }
        } break;
        }
        
        // Skip padding and seek to the next block offset
        int32_t streamPos = ( file->tell() % 16 );
        if ( streamPos <= 0 ) streamPos = 16;
        file->skip( 16 - streamPos );
    }

    file->close();
}


void Io_WriteSceneFile( Scene* scene, FileSystemObject* file )
{
    file->write( SCENE_MAGIC_V1 );

    auto fileSizeOffset = file->tell();
    file->write<int32_t>( 0x0 );

    // RESERVED : scene flags, etc
    file->write<int32_t>( 0x0 );

    file->writeString( scene->getSceneName() );
    file->write<uint8_t>( 0x0 );
    file->writePadding();

    file->write( TREE_MAGIC );

    auto& sceneNodes = scene->getSceneNodes();
    file->write( static_cast<int32_t>( sceneNodes.size() ) );

    // RESERVED : scene flags, etc
    file->write<int32_t>( 0x0 );
    file->write<int32_t>( 0x0 );

    for ( auto node : sceneNodes ) {
        node->serialize( file );
        file->writePadding();
    }

    auto fileSize = file->tell();
    file->seek( fileSizeOffset, flan::core::eFileReadDirection::FILE_READ_DIRECTION_BEGIN );
    file->write( fileSize );
}
