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

#define FLAN_SERIALIZE_CHAR_ARRAY( stream, variable, variableLength ) stream->write( (uint8_t*)&variable, variableLength )
#define FLAN_SERIALIZE_VARIABLE( stream, variable ) stream->write<decltype( variable )>( variable )
#define FLAN_DESERIALIZE_VARIABLE( stream, variable ) stream->read( (uint8_t*)&variable, sizeof( decltype( variable ) ) )

#define FLAN_DESERIALIZE_VECTOR2( stream, variable )\
{\
    float x = variable[0];\
    float y = variable[1];\
    FLAN_DESERIALIZE_VARIABLE( stream, x );\
    FLAN_DESERIALIZE_VARIABLE( stream, y );\
    variable = glm::vec2( x, y );\
}

#define FLAN_DESERIALIZE_VECTOR3( stream, variable )\
{\
    float x = variable[0];\
    float y = variable[1];\
    float z = variable[2];\
    FLAN_DESERIALIZE_VARIABLE( stream, x );\
    FLAN_DESERIALIZE_VARIABLE( stream, y );\
    FLAN_DESERIALIZE_VARIABLE( stream, z );\
    variable.x = x;\
    variable.y = y;\
    variable.z = z;\
}

#define FLAN_DESERIALIZE_VECTOR4( stream, variable )\
{\
    float x = variable[0];\
    float y = variable[1];\
    float z = variable[2];\
    float w = variable[3];\
    FLAN_DESERIALIZE_VARIABLE( stream, x );\
    FLAN_DESERIALIZE_VARIABLE( stream, y );\
    FLAN_DESERIALIZE_VARIABLE( stream, z );\
    FLAN_DESERIALIZE_VARIABLE( stream, w );\
    variable = glm::vec4( x, y, z, w );\
}

#define FLAN_DESERIALIZE_QUATERNION( stream, variable )\
{\
    float w = variable[3];\
    float x = variable[0];\
    float y = variable[1];\
    float z = variable[2];\
    FLAN_DESERIALIZE_VARIABLE( stream, w );\
    FLAN_DESERIALIZE_VARIABLE( stream, x );\
    FLAN_DESERIALIZE_VARIABLE( stream, y );\
    FLAN_DESERIALIZE_VARIABLE( stream, z );\
    variable = glm::quat( w, x, y, z );\
}
