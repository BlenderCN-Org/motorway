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
template<typename T> 
T& EnvironmentVariables::registerVariable( const std::string& name, const nyaStringHash_t typeHashcode, T* value, const T defaultValue, const bool serialize )
{
    Variable var = { name, value, typeHashcode, serialize };

    getEnvironmentVariableMap().emplace( NYA_STRING_HASH( name.c_str() ), var );
    *value = defaultValue;

    return *value;
}

template<typename T>
T* EnvironmentVariables::getVariable( nyaStringHash_t hashcode )
{
    auto iterator = getEnvironmentVariableMap().find( hashcode );

    return iterator != getEnvironmentVariableMap().end() ? ( T* )getEnvironmentVariableMap()[hashcode].Value : nullptr;
}
