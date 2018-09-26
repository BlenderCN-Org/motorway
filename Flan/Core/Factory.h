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

#include <functional>
#include <unordered_map>

template<typename T, typename... TArgs>
class Factory
{
public:
    using fnFactoryBuildFunc_t = std::function<T( TArgs... )>;

private:
    static std::map<fnStringHash_t, fnFactoryBuildFunc_t>& Variables()
    {
        static std::map<fnStringHash_t, fnFactoryBuildFunc_t> variables;
        return variables;
    }

public:
    static bool registerComponent( const fnStringHash_t hashcode, fnFactoryBuildFunc_t buildFunction )
    {
        Variables().emplace( hashcode, buildFunction );

        return true;
    }

    static T tryBuildWithHashcode( const fnStringHash_t hashcode, TArgs... args )
    {
        auto it = Variables().find( hashcode );

        return ( it != Variables().end() ) ? it->second( std::forward<TArgs>( args )... ) : ( T )0;
    }
};
