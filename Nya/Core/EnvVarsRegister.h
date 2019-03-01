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

#include <map>
#include <functional>

#include <FileSystem/FileSystemObject.h>

using nyaToStringEnvType_t = std::function<char*( void* )>;
using nyaFromStringEnvType_t = std::function<void*( const nyaString_t& )>;

struct Variable
{
    std::string      Name;
    void*            Value;
    nyaStringHash_t  Type;
    bool             ShouldBeSerialized;
};

struct CustomVariableType
{
    nyaToStringEnvType_t     ToString;
    nyaFromStringEnvType_t   FromString;
};

class EnvironmentVariables
{
public:
    template<typename T> static T&      registerVariable( const std::string& name, const nyaStringHash_t typeHashcode, T* value, const T defaultValue = ( T )0, const bool serialize = true );
    static int                          registerCustomType( const nyaStringHash_t typeHashcode, nyaToStringEnvType_t toStringFunc, nyaFromStringEnvType_t fromStringFunc );
    template<typename T> static T*      getVariable( nyaStringHash_t hashcode );
    static void                         serialize( FileSystemObject* file );
    static void                         deserialize( FileSystemObject* file );

private:
    static void                                  readValue( Variable& variable, const nyaString_t& inputValue );
    static std::map<nyaStringHash_t, Variable>&  getEnvironmentVariableMap();
};

#include "EnvVarsRegister.inl"

#define NYA_ENV_OPTION_ENUM( optionName ) optionName,
#define NYA_ENV_OPTION_STRING( optionName ) (char* const)#optionName,
#define NYA_ENV_OPTION_CASE( optionName ) case NYA_STRING_HASH( #optionName ): return (void*)optionName;

// Generates and registers a custom option type to the EnvironmentVariables instance
// The registered type will be serializable/deserializable
// Option list must be a macro defining the different options available
// Options enum is named: eACustomOptionList
// Options string is name: sACustomOptionList
//
// e.g. 
// #define OPTION_LIST( option ) option( OPTION_1 ) option( OPTION_2 )
// NYA_ENV_OPTION_LIST( ACustomOptionList, OPTION_LIST )
// (...)
// NYA_ENV_VAR( d_TestVar, "TestVariable", OPTION_1, eACustomOptionList )
#define NYA_ENV_OPTION_LIST( name, list )\
enum e##name : int32_t\
{\
    list( NYA_ENV_OPTION_ENUM )\
    e##name##_COUNT\
};\
static constexpr char* s##name[e##name##_COUNT] =\
{\
    list( NYA_ENV_OPTION_STRING )\
};\
int __DUMMY##name =EnvironmentVariables::registerCustomType(\
NYA_STRING_HASH( "e"#name ),\
[=]( void* typelessVar )\
{\
    return s##name[*(e##name*)typelessVar]; \
},\
[=]( const nyaString_t& str )\
{\
    const nyaStringHash_t hashcode = nya::core::CRC32( str );\
    switch ( hashcode ) {\
        list( NYA_ENV_OPTION_CASE )\
        default: return (void*)nullptr;\
    };\
} );

#define NYA_ENV_VAR( name, defaultValue, type ) static type name = EnvironmentVariables::registerVariable<type>( #name, NYA_STRING_HASH( #type ), &name, defaultValue );

#if NYA_DEVBUILD
#define NYA_DEV_VAR_PERSISTENT( name, desc, defaultValue, type ) static type name = EnvironmentVariables::registerVariable<type>( #name, NYA_STRING_HASH( #type ), &name, defaultValue );
#define NYA_DEV_VAR( name, desc, defaultValue, type ) static type name = EnvironmentVariables::registerVariable<type>( #name, NYA_STRING_HASH( #type ), &name, defaultValue, false );
#else
#define NYA_DEV_VAR_PERSISTENT( name, desc, defaultValue, type ) static type name = defaultValue;
#define NYA_DEV_VAR( name, desc, defaultValue, type ) static type name = defaultValue;
#endif
