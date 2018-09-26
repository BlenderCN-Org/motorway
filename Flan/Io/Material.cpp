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
#include "Material.h"

#include "TextStreamHelpers.h"
#include <Core/StringHelpers.h>

#include <Graphics/GraphicsAssetManager.h>

void flan::core::LoadCompiledMaterialFile( FileSystemObject* file, CompiledMaterialLoadData& data )
{
    fnString_t streamLine, dictionaryKey, dictionaryValue;

    bool isReadingTextureSet = false;
    bool isReadingRendererSetup = false;
    bool isReadingRenderPasses = false;
    bool isReadingShaderSet = false;

    std::unordered_map<uint32_t, fnString_t>* currentTextureSet = nullptr;
    fnString_t* currentShaderFile = nullptr;

    while ( file->isGood() ) {
        flan::core::ReadString( file, streamLine );

        // Find seperator character offset in the line (if any)
        const auto commentSeparator = streamLine.find_first_of( FLAN_STRING( "#" ) );

        // Remove user comments before reading the keypair value
        if ( commentSeparator != fnString_t::npos ) {
            streamLine.erase( streamLine.begin() + commentSeparator, streamLine.end() );
        }

        flan::core::TrimString( streamLine );

        // Skip commented out and empty lines
        if ( streamLine.empty() ) {
            continue;
        }

        if ( streamLine[0] == '{' ) {
            continue;
        } else if ( streamLine[0] == '}' ) {
            // True if the function has finished reading a render pass
            auto hasReadPass = ( !isReadingTextureSet && !isReadingShaderSet );

            if ( hasReadPass && ( currentTextureSet == nullptr && currentShaderFile == nullptr ) ) {
                isReadingRenderPasses = false;
            } else if ( hasReadPass && isReadingRenderPasses ) {
                currentTextureSet = nullptr;
                currentShaderFile = nullptr;
            }

            isReadingTextureSet = false;
            isReadingShaderSet = false;
            isReadingRendererSetup = false;
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
                if ( isReadingTextureSet ) {
                    uint32_t textureSlotIndex = std::stoul( dictionaryKey );
                    auto textureAbsolutePath = flan::core::WrappedStringToString( dictionaryValue );

                    currentTextureSet->insert( std::make_pair( textureSlotIndex, textureAbsolutePath ) );
                } else if ( isReadingRendererSetup ) {
                    switch ( flan::core::CRC32( dictionaryKey.c_str() ) ) {
                    case FLAN_STRING_HASH( "UseAlphaTest" ):
                        data.RendererSetup.UseAlphaTest = flan::core::StringToBoolean( dictionaryValue );
                        break;
                    case FLAN_STRING_HASH( "UseAlphaBlend" ):
                        data.RendererSetup.UseAlphaBlend = flan::core::StringToBoolean( dictionaryValue );
                        break;
                    case FLAN_STRING_HASH( "IsDoubleFace" ):
                        data.RendererSetup.IsDoubleFace = flan::core::StringToBoolean( dictionaryValue );
                        break;
                    }
                } else if ( isReadingShaderSet ) {
                    auto shaderFilename = flan::core::WrappedStringToString( dictionaryValue );
                    *currentShaderFile = shaderFilename;
                } else {
                    switch ( flan::core::CRC32( dictionaryKey.c_str() ) ) {
                    case FLAN_STRING_HASH( "Name" ):
                        data.MaterialName = flan::core::WrappedStringToString( dictionaryValue );
                        break;
                    /*case FLAN_STRING_HASH( "Shader" ):
                        data.ShaderFileName = Io_WrappedStringToString( dictionaryValue );
                        break;*/
                    }
                }
            }
        } else {
            // Trim the line since assets on Windows will have both new line and carriage return characters
            flan::core::TrimString( streamLine );

            auto potentialKeywordHashcode = flan::core::CRC32( streamLine.c_str() );

            if ( isReadingRenderPasses ) {
                if ( currentTextureSet == nullptr && currentShaderFile == nullptr ) {
                    if ( potentialKeywordHashcode == FLAN_STRING_HASH( "Light" ) ) {
                        currentTextureSet = &data.Textures;
                        currentShaderFile = &data.ShaderFileName;
                    } else if ( potentialKeywordHashcode == FLAN_STRING_HASH( "Depth" ) ) {
                        currentTextureSet = &data.DepthTextures;
                        currentShaderFile = &data.DepthShaderFileName;
                    }
                } else {
                    if ( potentialKeywordHashcode == FLAN_STRING_HASH( "ShaderSet" ) ) {
                        isReadingShaderSet = true;
                    } else if ( potentialKeywordHashcode == FLAN_STRING_HASH( "TextureSet" ) ) {
                        isReadingTextureSet = true;
                    }
                }
            } else {
                /*if ( potentialKeywordHashcode == FLAN_STRING_HASH( "TextureSet" ) ) {
                    isReadingTextureSet = true;
                } else*/ if ( potentialKeywordHashcode == FLAN_STRING_HASH( "Setup" ) ) {
                    isReadingRendererSetup = true;
                } else if ( potentialKeywordHashcode == FLAN_STRING_HASH( "Pass" ) ) {
                    isReadingRenderPasses = true;
                }
            }
        }
    }

    file->close();
}
