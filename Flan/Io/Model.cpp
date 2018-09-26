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
#include "Model.h"

#include <Framework/Mesh.h>
#include <Core/LocaleHelpers.h>

#include <Framework/Material.h>

#include "TextStreamHelpers.h"

void Io_ReadModelFile( FileSystemObject* file, ModelLoadData& data )
{
    fnString_t streamLine, dictionaryKey, dictionaryValue;

    ModelMeshInstance* activeMesh = nullptr;
    bool isReadingSubmeshes = false;
    while ( file->isGood() ) {
        flan::core::ReadString( file, streamLine );

        // Find seperator character offset in the line (if any)
        const auto commentSeparator = streamLine.find_first_of( FLAN_STRING( "#" ) );

        // Remove user comments before reading the keypair value
        if ( commentSeparator != fnString_t::npos ) {
            streamLine.erase( streamLine.begin() + commentSeparator, streamLine.end() );
        }

        // Skip commented out and empty lines
        if ( streamLine.empty() ) {
            continue;
        }

        if ( streamLine[0] == '{' ) {
            continue;
        } else if ( streamLine[0] == '}' ) {
            if ( !isReadingSubmeshes )
                activeMesh = nullptr;

            continue;
        }

        const auto keyValueSeparator = streamLine.find_first_of( ':' );

        // Check if this is a key value line
        if ( keyValueSeparator != fnString_t::npos ) {
            dictionaryKey = streamLine.substr( 0, keyValueSeparator );
            dictionaryValue = streamLine.substr( keyValueSeparator + 1 );

            // Trim both key and values (useful if a file has inconsistent spacing, ...)
            flan::core::TrimString( dictionaryKey );
            flan::core::TrimString( dictionaryValue );

            // Do the check after triming, since the value might be a space or a tab character
            if ( !dictionaryValue.empty() ) {
                auto keyHashcode = flan::core::CRC32( dictionaryKey.c_str() );

                // Check if we are currently reader a layer or not
                if ( activeMesh != nullptr ) {
                    if ( isReadingSubmeshes ) {
                        activeMesh->SubMeshMaterialMap.push_back( std::make_pair( dictionaryKey, dictionaryValue ) );
                    } else {
                        switch ( keyHashcode ) {
                        case FLAN_STRING_HASH( "Name" ):
                            activeMesh->Name = dictionaryValue;
                            break;
                        }
                    }
                }
            }
        } else {
            // Trim the line since assets on Windows will have both new line and carriage return characters
            flan::core::TrimString( streamLine );

            // Read and proceed well-known keywords (layers, ...)
            auto potentialKeywordHashcode = flan::core::CRC32( streamLine.c_str() );

            if ( potentialKeywordHashcode == FLAN_STRING_HASH( "Mesh" ) ) {
                data.Meshes.push_back( {} );

                activeMesh = &data.Meshes.back();
            } else if ( potentialKeywordHashcode == FLAN_STRING_HASH( "SubMeshes" ) ) {
                isReadingSubmeshes = true;
            }
        }
    }

    file->close();
}

void Io_WriteModelFile( FileSystemObject* file, const std::vector<Mesh*>& meshes )
{
    for ( auto mesh : meshes ) {
        auto narrowMeshName = flan::core::WideStringToString( mesh->getName() );

        file->writeString( "Mesh\n{\n" );
        file->writeString( "\tName : " + narrowMeshName + "\n" );
        file->writeString( "\tSubMeshes\n{\n" );
        for ( auto& subMesh : mesh->getSubMeshVector() ) {
            auto narrowSubMeshName = flan::core::WideStringToString( subMesh.name );
            auto narrowMaterialName = flan::core::WideStringToString( subMesh.material->getName() );

            file->writeString( narrowSubMeshName + " : " + narrowMaterialName + "\n" );
        }
        file->writeString( "\t}\n" );
        file->writeString( "}\n" );
    }
}
