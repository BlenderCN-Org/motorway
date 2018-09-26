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

#define FLAN_GEN_ENUM_ENTRY( optionName ) optionName,
#define FLAN_GEN_FLAN_STRING_ENTRY( optionName ) (fnChar_t* const)FLAN_STRING( #optionName ),
#define FLAN_GEN_STRING_ENTRY( optionName ) (char* const)#optionName,
#define FLAN_ENUM_TO_STRING_CASE( optionName ) case FLAN_STRING_HASH( #optionName ): return optionName;

// Generate a strongly typed enum lazily (create the enum, enum to string convertion, hashcode to enum convertion, stream operator overload, etc.)
//      optionList: an user defined macro holding each enum member name (e.g. #define optionList( operator ) operator( ENUM_MEMBER_1 ) operator( ENUM_MEMBER_2 ))
#define FLAN_LAZY_ENUM( optionList )\
enum e##optionList : int32_t\
{\
    optionList( FLAN_GEN_ENUM_ENTRY )\
    optionList##_COUNT\
};\
\
static constexpr fnChar_t* optionList##ToFlanString[e##optionList::optionList##_COUNT] = \
{\
    optionList( FLAN_GEN_FLAN_STRING_ENTRY )\
};\
static constexpr char* optionList##ToString[e##optionList::optionList##_COUNT] = \
{\
    optionList( FLAN_GEN_STRING_ENTRY )\
};\
\
static e##optionList StringTo##optionList( const fnStringHash_t hashcode )\
{\
    switch ( hashcode ) {\
        optionList( FLAN_ENUM_TO_STRING_CASE )\
    default:\
        return (e##optionList)0;\
    };\
}\
static fnOStream_t& operator << ( fnOStream_t& stream, const e##optionList& option )\
{\
    stream << optionList##ToFlanString[option];\
    return stream;\
}
