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

#include <unordered_map>
#include <glm/glm.hpp>

class FileSystemObject;
class Component;

class GameObject
{
public:
                                    GameObject( const fnStringHash_t objectNameHashcode = FLAN_STRING_HASH( "GameObject" ) );
                                    GameObject& operator = ( GameObject& ) = delete; // A generic game object should not be copiable;
                                    GameObject( GameObject& ) = delete; // A generic game object should not be copiable
    virtual                         ~GameObject() { }

    virtual void                    Update( const float frameTime );
    
    const fnStringHash_t            getHashcode() const;
    bool                            isHashcodeEqual( const fnStringHash_t hashcodeToCompareTo );

    virtual void                    Serialize( FileSystemObject* stream );
    virtual void                    Deserialize( FileSystemObject* stream );

private:
    fnStringHash_t                  objectHashcode;
};
