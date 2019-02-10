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

#include "Shared.h"
#include "Mesh.h"

#include <FileSystem/FileSystemObject.h>
#include "TextStreamHelpers.h"

using blockMagic_t = uint32_t;

//=====================================
// Mesh file header
//=====================================
struct FileHeader
{
    uint32_t    version;
    uint32_t    fileSize;

    union
    {
        struct
        {
            uint8_t hasUvMap0 : 1;
            uint8_t hasNormals : 1;
            uint8_t : 0;
        };

        uint32_t flagset;
    };

    uint32_t    __PADDING__;
};

//=====================================
// Mesh file data block header
//=====================================
struct BlockHeader
{
    blockMagic_t	magic;
	uint32_t        size;
    uint32_t        subBlock1Size;
    uint32_t        subBlock2Size;
};

//=====================================
// Data block magic
//=====================================

// MATerial Library
constexpr uint32_t MATL_MAGIC = 0x4C54414D;

// GEOMetry data
constexpr uint32_t GEOM_MAGIC = 0x4D4F4547;

void nya::core::LoadGeometryFile( FileSystemObject* file, GeomLoadData& data )
{
    FileHeader fileHeader = {};
    file->read( ( uint8_t* )&fileHeader, sizeof( FileHeader ) );

    // The script ALWAYS export position (3D)
    data.vertexStrides = { 3 };

    // NOTE Order is important, strides setup the input layout later in the asset loading pipeline!
    if ( fileHeader.hasUvMap0 == 1 ) {
        data.vertexStrides.push_back( 2 );
    }

    if ( fileHeader.hasNormals == 1 ) {
        data.vertexStrides.push_back( 3 );
    }

    while ( file->tell() < fileHeader.fileSize ) {
        BlockHeader blockHeader = {};
        file->read( ( uint8_t* )&blockHeader, sizeof( BlockHeader ) );

        // For modularity sake, we check the block magic
        switch ( blockHeader.magic ) {
        case MATL_MAGIC: {
            const std::streampos blockEndOffset = static_cast< std::size_t >( file->tell() ) + blockHeader.size;

            while ( file->tell() < static_cast<uint64_t>( blockEndOffset ) ) {
                uint32_t materialHashcode = 0;
                file->read( ( uint8_t* )&materialHashcode, sizeof( uint32_t ) );

                nyaString_t materialFileName = {};
                nya::core::ReadString( file, materialFileName );

                data.materialsReferences.push_back( std::make_pair( materialHashcode, materialFileName ) );
            }
            break;
        }

        case GEOM_MAGIC: {
            if ( fileHeader.version == 1 ) {      
                //// Read submesh entries
                //const uint32_t subMeshCount = ( blockHeader.size / sizeof( submeshEntry_t ) );

                //data.submeshesToLoad.resize( subMeshCount );
                //fileStream.read( (char*)data.submeshesToLoad.data(), blockHeader.size );
            } else if ( fileHeader.version >= 2 ) {
                const auto blockEndOffset = file->tell() + blockHeader.size;

                while ( file->tell() < blockEndOffset ) {
                    data.subMesh.push_back( {} );
                    auto& subMesh = data.subMesh.back();

                    file->read( subMesh.hashcode );
                    
                    nya::core::ReadString( file, subMesh.name );

                    file->read( subMesh.indiceBufferOffset );
                    file->read( subMesh.indiceCount );
                    file->read( subMesh.boundingSphere );
                    
                    if ( fileHeader.version >= 3 ) {
                        nyaVec3f location, dimensions;
                        file->read( ( uint8_t* )&location, sizeof( nyaVec3f ) );
                        file->read( ( uint8_t* )&dimensions, sizeof( nyaVec3f ) );

                        nya::core::CreateAABB( subMesh.aabb, location, dimensions );
                    }

                    file->read( subMesh.levelOfDetailIndex );

                    // TODO This should not be necessary...
                    // Skip padding and seek to the next block offset
                    int32_t streamPos = ( file->tell() % 16 );
                    if ( streamPos <= 0 ) streamPos = 16;
                    file->skip( 16 - streamPos );
                }
            }

            auto vertexBufferSize = blockHeader.subBlock1Size;
            auto indiceBufferSize = blockHeader.subBlock2Size;

            data.vertices.resize( vertexBufferSize / sizeof( float ) );
            data.indices.resize( indiceBufferSize / sizeof( uint32_t ) );

            // Read buffer data
            file->read( ( uint8_t* )data.vertices.data(), vertexBufferSize );
            file->read( ( uint8_t* )data.indices.data(), indiceBufferSize );
            break;
        }

        default: // Unknown block
            continue;
        }
            
        // TODO This should not be necessary...
        // Skip padding and seek to the next block offset
        int32_t streamPos = ( file->tell() % 16 );
        if ( streamPos <= 0 ) streamPos = 16;
        file->skip( 16 - streamPos );
    } 

    file->close();
}
