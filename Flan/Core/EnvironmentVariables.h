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

using fnToStringEnvType_t = std::function<char*( void* )>;
using fnFromStringEnvType_t = std::function<void*( const fnString_t& )>;

struct Variable
{
    std::string     Name;
    void*           Value;
    fnStringHash_t  Type;
    bool            ShouldBeSerialized;
};

struct CustomVariableType
{
    fnToStringEnvType_t     ToString;
    fnFromStringEnvType_t   FromString;
};

class EnvironmentVariables
{
public:
    template<typename T> static T&      registerVariable( const std::string& name, const fnStringHash_t typeHashcode, T* value, const T defaultValue = ( T )0, const bool serialize = true );
    static int                          registerCustomType( const fnStringHash_t typeHashcode, fnToStringEnvType_t toStringFunc, fnFromStringEnvType_t fromStringFunc );
    template<typename T> static T*      getVariable( fnStringHash_t hashcode );
    static void                         serialize( FileSystemObject* file );
    static void                         deserialize( FileSystemObject* file );

private:
    static void                                 readValue( Variable& variable, const fnString_t& inputValue );
    static std::map<fnStringHash_t, Variable>&  getEnvironmentVariableMap();
};

#include "EnvironmentVariables.inl"

#define FLAN_ENV_OPTION_ENUM( optionName ) optionName,
#define FLAN_ENV_OPTION_STRING( optionName ) (char* const)#optionName,
#define FLAN_ENV_OPTION_CASE( optionName ) case FLAN_STRING_HASH( #optionName ): return (void*)optionName;

// Generates and registers a custom option type to the EnvironmentVariables instance
// The registered type will be serializable/deserializable
// Option list must be a macro defining the different options available
// Options enum is named: eACustomOptionList
// Options string is name: sACustomOptionList
//
// e.g. 
// #define OPTION_LIST( option ) option( OPTION_1 ) option( OPTION_2 )
// FLAN_ENV_OPTION_LIST( ACustomOptionList, OPTION_LIST )
// (...)
// FLAN_ENV_VAR( d_TestVar, "TestVariable", OPTION_1, eACustomOptionList )
#define FLAN_ENV_OPTION_LIST( name, list )\
enum e##name : int32_t\
{\
    list( FLAN_ENV_OPTION_ENUM )\
    e##name##_COUNT\
};\
static constexpr char* s##name[e##name##_COUNT] =\
{\
    list( FLAN_ENV_OPTION_STRING )\
};\
int DUMMY##name = EnvironmentVariables::registerCustomType(\
FLAN_STRING_HASH( "e"#name ),\
[=]( void* typelessVar )\
{\
    return s##name[*(e##name*)typelessVar]; \
},\
[=]( const fnString_t& str )\
{\
    const fnStringHash_t hashcode = flan::core::CRC32( str );\
    switch ( hashcode ) {\
        list( FLAN_ENV_OPTION_CASE )\
        default: return (void*)nullptr;\
    };\
} );

#define FLAN_ENV_VAR( name, desc, defaultValue, type ) static type name = EnvironmentVariables::registerVariable<type>( #name, FLAN_STRING_HASH( #type ), &name, defaultValue );

#if FLAN_DEVBUILD
#define FLAN_DEV_VAR_PERSISTENT( name, desc, defaultValue, type ) static type name = EnvironmentVariables::registerVariable<type>( #name, FLAN_STRING_HASH( #type ), &name, defaultValue );
#define FLAN_DEV_VAR( name, desc, defaultValue, type ) static type name = EnvironmentVariables::registerVariable<type>( #name, FLAN_STRING_HASH( #type ), &name, defaultValue, false );
#else
#define FLAN_DEV_VAR_PERSISTENT( name, desc, defaultValue, type ) static type name = defaultValue;
#define FLAN_DEV_VAR( name, desc, defaultValue, type ) static type name = defaultValue;
#endif

#define FLAN_IMPORT_VAR_PTR( name, type ) type* name = EnvironmentVariables::getVariable<type>( FLAN_STRING_HASH( #name ) );
