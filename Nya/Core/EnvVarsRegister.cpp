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
#include "EnvVarsRegister.h"

#include <Io/TextStreamHelpers.h>
#include <Core/StringHelpers.h>

// Not really nice, but this way we can control the initialization of global vars
static inline std::map<nyaStringHash_t, CustomVariableType>& ENV_VAR_TYPES()
{
    static std::map<nyaStringHash_t, CustomVariableType> internalMap = {};
    return internalMap;
}

inline std::map<nyaStringHash_t, Variable>& EnvironmentVariables::getEnvironmentVariableMap()
{
    static std::map<nyaStringHash_t, Variable> internalMap = {};
    return internalMap;
}

void EnvironmentVariables::readValue( Variable& variable, const nyaString_t& inputValue )
{
    if ( !variable.ShouldBeSerialized ) {
        return;
    }

    switch ( variable.Type ) {
    case NYA_STRING_HASH( "double" ):
        *( double* )variable.Value = std::stod( inputValue );
        break;
    case NYA_STRING_HASH( "float" ):
        *( float* )variable.Value = std::stof( inputValue );
        break;
    case NYA_STRING_HASH( "long long" ):
    case NYA_STRING_HASH( "int64_t" ):
        *( int64_t* )variable.Value = std::stoll( inputValue );
        break;
    case NYA_STRING_HASH( "unsigned long long" ):
    case NYA_STRING_HASH( "uint64_t" ):
        *( uint64_t* )variable.Value = std::stoull( inputValue );
        break;
    case NYA_STRING_HASH( "int" ):
    case NYA_STRING_HASH( "int32_t" ):
        *( int32_t* )variable.Value = std::stoi( inputValue );
        break;
    case NYA_STRING_HASH( "unsigned int" ):
    case NYA_STRING_HASH( "uint32_t" ):
        *( uint32_t* )variable.Value = std::stoi( inputValue );
        break;
        // TODO Not safe
    case NYA_STRING_HASH( "short" ):
    case NYA_STRING_HASH( "int16_t" ):
        *( int16_t* )variable.Value = std::stoi( inputValue );
        break;
    case NYA_STRING_HASH( "unsigned short" ):
    case NYA_STRING_HASH( "uint16_t" ):
        *( uint16_t* )variable.Value = std::stoi( inputValue );
        break;
    case NYA_STRING_HASH( "bool" ):
        *( bool* )variable.Value = ( inputValue == NYA_STRING( "True" ) );
        break;
    default: {
        auto it = ENV_VAR_TYPES().find( variable.Type );

        // This shouldn't cause any trouble as long as the generated enum type matches the type used here
        if ( it != ENV_VAR_TYPES().end() ) {
            *( int64_t* )variable.Value = ( int64_t )it->second.FromString( inputValue );
        }
    } break;
    }
}

int EnvironmentVariables::registerCustomType( const nyaStringHash_t typeHashcode, nyaToStringEnvType_t toStringFunc, nyaFromStringEnvType_t fromStringFunc )
{
    CustomVariableType varType = { toStringFunc, fromStringFunc };
    ENV_VAR_TYPES().emplace( typeHashcode, varType );

    // NOTE Dummy value so that the compiler doesn't think we're trying to redeclare the function outside the class within a macro call
    return 0;
}

void EnvironmentVariables::serialize( FileSystemObject* file )
{
    for ( auto& var : getEnvironmentVariableMap() ) {
        const auto varInfos = var.second;

        if ( !varInfos.ShouldBeSerialized ) {
            continue;
        }

        std::string varLine = varInfos.Name;
        varLine.append( ": " );

        switch ( varInfos.Type ) {
        case NYA_STRING_HASH( "double" ):
            varLine.append( std::to_string( *( double* )varInfos.Value ) );
            break;
        case NYA_STRING_HASH( "float" ):
            varLine.append( std::to_string( *( float* )varInfos.Value ) );
            break;
        case NYA_STRING_HASH( "long long" ):
        case NYA_STRING_HASH( "int64_t" ):
            varLine.append( std::to_string( *( int64_t* )varInfos.Value ) );
            break;
        case NYA_STRING_HASH( "unsigned long long" ):
        case NYA_STRING_HASH( "uint64_t" ):
            varLine.append( std::to_string( *( uint64_t* )varInfos.Value ) );
            break;
        case NYA_STRING_HASH( "int" ):
        case NYA_STRING_HASH( "int32_t" ):
            varLine.append( std::to_string( *( int32_t* )varInfos.Value ) );
            break;
        case NYA_STRING_HASH( "unsigned int" ):
        case NYA_STRING_HASH( "uint32_t" ):
            varLine.append( std::to_string( *( uint32_t* )varInfos.Value ) );
            break;
            // TODO Not safe
        case NYA_STRING_HASH( "short" ):
        case NYA_STRING_HASH( "int16_t" ):
            varLine.append( std::to_string( *( int16_t* )varInfos.Value ) );
            break;
        case NYA_STRING_HASH( "unsigned short" ):
        case NYA_STRING_HASH( "uint16_t" ):
            varLine.append( std::to_string( *( uint16_t* )varInfos.Value ) );
            break;
        case NYA_STRING_HASH( "bool" ):
            varLine.append( *( bool* )varInfos.Value ? "True" : "False" );
            break;
        default:
            auto it = ENV_VAR_TYPES().find( varInfos.Type );

            if ( it != ENV_VAR_TYPES().end() ) {
                varLine.append( it->second.ToString( varInfos.Value ) );
            } else {
                NYA_COUT << "Unknown type (with hashcode: 0x" << std::hex << varInfos.Type << ")" << std::endl;
                continue;
            }
            break;
        }

        varLine.append( "\n" );
        file->writeString( varLine );
    }
}

void EnvironmentVariables::deserialize( FileSystemObject* file )
{
    nyaString_t streamLine, dictionaryKey, dictionaryValue;

    NYA_CLOG << "Env vars deserialization started..." << std::endl;

    while ( file->isGood() ) {
        nya::core::ReadString( file, streamLine );

        // Find seperator character offset in the line (if any)
        const auto commentSeparator = streamLine.find_first_of( NYA_STRING( "#" ) );

        // Remove user comments before reading the keypair value
        if ( commentSeparator != nyaString_t::npos ) {
            streamLine.erase( streamLine.begin() + commentSeparator, streamLine.end() );
        }

        // Skip commented out and empty lines
        if ( streamLine.empty() ) {
            continue;
        }

        const auto keyValueSeparator = streamLine.find_first_of( ':' );

        // Check if this is a key value line
        if ( keyValueSeparator != nyaString_t::npos ) {
            dictionaryKey = streamLine.substr( 0, keyValueSeparator );
            dictionaryValue = streamLine.substr( keyValueSeparator + 1 );

            // Trim both key and values (useful if a file has inconsistent spacing, ...)
            nya::core::TrimString( dictionaryKey );
            nya::core::TrimString( dictionaryValue );

            // Do the check after triming, since the value might be a space or a tab character
            if ( !dictionaryValue.empty() ) {
                auto keyHashcode = nya::core::CRC32( dictionaryKey.c_str() );
                auto variablesIterator = getEnvironmentVariableMap().find( keyHashcode );

                if ( variablesIterator != getEnvironmentVariableMap().end() ) {
                    auto variableInstance = getEnvironmentVariableMap().at( keyHashcode );

                    readValue( variableInstance, dictionaryValue );
                } else {
                    NYA_CERR << "Unknown environment variable '" << dictionaryKey << "' (either spelling mistake or deprecated)" << std::endl;
                }
            }
        }
    }
}
