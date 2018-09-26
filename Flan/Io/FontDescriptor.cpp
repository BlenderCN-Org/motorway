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
#include "FontDescriptor.h"

void flan::core::LoadFontFile( FileSystemObject* file, FontDescriptor& data )
{
    fnString_t streamLine, lineToken;

    while ( file->isGood() ) {
        flan::core::ReadString( file, streamLine );

        const auto keyValueSeparator = streamLine.find_first_of( ' ' );

        // Check if this is a key value line
        if ( keyValueSeparator != fnString_t::npos ) {
            lineToken = streamLine.substr( 0, keyValueSeparator );
            
            // Trim both key and values (useful if a file has inconsistent spacing, ...)
            flan::core::TrimString( lineToken );

            auto keyHashcode = flan::core::CRC32( lineToken.c_str() ); 
            
            switch ( keyHashcode ) {
            case FLAN_STRING_HASH( "page" ): {
                auto commonVariablesLine = streamLine.substr( keyValueSeparator + 1 );

                auto commonVariableOffset = commonVariablesLine.find( ' ' );
                auto previousCommonVariableOffset = 0;
                while ( commonVariableOffset != fnString_t::npos ) {
                    auto variable = commonVariablesLine.substr( previousCommonVariableOffset, commonVariableOffset - previousCommonVariableOffset );

                    const auto keyValueSeparator = variable.find_first_of( '=' );
                    if ( keyValueSeparator == fnString_t::npos ) {
                        continue;
                    }

                    auto key = variable.substr( 0, keyValueSeparator );
                    auto value = variable.substr( keyValueSeparator + 1 );

                    auto tokenHashcode = flan::core::CRC32( key.c_str() );
                    switch ( tokenHashcode ) {
                    case FLAN_STRING_HASH( "file" ):
                        data.Name = flan::core::WrappedStringToString( value );
                        break;
                    default:
                        break;
                    }

                    // TODO Shouldn't be needed
                    if ( previousCommonVariableOffset > commonVariableOffset ) {
                        break;
                    }

                    // Look for the next var
                    previousCommonVariableOffset = static_cast<int>( commonVariableOffset ) + 1;
                    commonVariableOffset = commonVariablesLine.find( ' ', commonVariableOffset );
                }
            } break;


            case FLAN_STRING_HASH( "common" ): {
                auto commonVariablesLine = streamLine.substr( keyValueSeparator + 1 );

                auto commonVariableOffset = commonVariablesLine.find( ' ', keyValueSeparator + 1 );
                auto previousCommonVariableOffset = 0;
                while ( commonVariableOffset != fnString_t::npos ) {
                    auto variable = commonVariablesLine.substr( previousCommonVariableOffset, commonVariableOffset - previousCommonVariableOffset );

                    const auto keyValueSeparator = variable.find_first_of( '=' );
                    if ( keyValueSeparator == fnString_t::npos ) {
                        continue;
                    }

                    auto key = variable.substr( 0, keyValueSeparator );
                    auto value = variable.substr( keyValueSeparator + 1 );

                    auto tokenHashcode = flan::core::CRC32( key.c_str() );
                    switch ( tokenHashcode ) {
                    case FLAN_STRING_HASH( "scaleW" ):
                        data.AtlasWidth = std::stoi( value );
                        break;
                    case FLAN_STRING_HASH( "scaleH" ):
                        data.AtlasHeight = std::stoi( value );
                        break;
                    default:
                        break;
                    }

                    // Look for the next var
                    previousCommonVariableOffset = static_cast<int>( commonVariableOffset ) + 1;
                    commonVariableOffset = commonVariablesLine.find( ' ', previousCommonVariableOffset );
                }
            } break;

            case FLAN_STRING_HASH( "char" ):
            {
                auto commonVariablesLine = streamLine.substr( keyValueSeparator + 1 );

                FontDescriptor::Glyph* glyph = nullptr;

                auto commonVariableOffset = commonVariablesLine.find( ' ', keyValueSeparator + 1 );
                auto previousCommonVariableOffset = 0;
                while ( commonVariableOffset != fnString_t::npos ) {
                    auto variable = commonVariablesLine.substr( previousCommonVariableOffset, commonVariableOffset - previousCommonVariableOffset );

                    if ( variable.empty() ) {
                        previousCommonVariableOffset++;
                        commonVariableOffset = commonVariablesLine.find( ' ', previousCommonVariableOffset );
                        continue;
                    }

                    const auto keyValueSeparator = variable.find_first_of( '=' );
                    if ( keyValueSeparator == fnString_t::npos ) {
                        continue;
                    }

                    auto key = variable.substr( 0, keyValueSeparator );
                    auto value = variable.substr( keyValueSeparator + 1 );

                    auto tokenHashcode = flan::core::CRC32( key.c_str() );
                    switch ( tokenHashcode ) {
                    case FLAN_STRING_HASH( "id" ):
                    {
                        auto glyphIndex = std::stoi( value );
                        auto requiredAtlasCapacity = ( glyphIndex + 1 );

                        if ( requiredAtlasCapacity > data.Glyphes.size() ) {
                            data.Glyphes.resize( requiredAtlasCapacity );
                        }

                        glyph = &data.Glyphes[glyphIndex];
                    } break;
                    case FLAN_STRING_HASH( "x" ):
                        glyph->PositionX = std::stoi( value );
                        break;
                    case FLAN_STRING_HASH( "y" ):
                        glyph->PositionY = std::stoi( value );
                        break;
                    case FLAN_STRING_HASH( "width" ):
                        glyph->Width = std::stoi( value );
                        break;
                    case FLAN_STRING_HASH( "height" ):
                        glyph->Height = std::stoi( value );
                        break;
                    case FLAN_STRING_HASH( "xoffset" ):
                        glyph->OffsetX = std::stoi( value );
                        break;
                    case FLAN_STRING_HASH( "yoffset" ):
                        glyph->OffsetY = std::stoi( value );
                        break;
                    case FLAN_STRING_HASH( "xadvance" ):
                        glyph->AdvanceX = std::stoi( value );
                        break;
                    default:
                        break;
                    }

                    // Look for the next var
                    previousCommonVariableOffset = static_cast<int>( commonVariableOffset ) + 1;
                    commonVariableOffset = commonVariablesLine.find( ' ', previousCommonVariableOffset );
                }
            } break;


            default:
                continue;
            }
        }
    }

    file->close();
}
