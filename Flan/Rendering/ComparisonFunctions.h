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

#include <Core/LazyEnum.h>

namespace flan
{
    namespace rendering
    {
#define ComparisonFunction( option )\
        option( COMPARISON_FUNCTION_NEVER )\
        option( COMPARISON_FUNCTION_ALWAYS )\
        option( COMPARISON_FUNCTION_LESS )\
        option( COMPARISON_FUNCTION_GREATER )\
        option( COMPARISON_FUNCTION_LEQUAL )\
        option( COMPARISON_FUNCTION_GEQUAL )\
        option( COMPARISON_FUNCTION_NOT_EQUAL )\
        option( COMPARISON_FUNCTION_EQUAL )

        FLAN_LAZY_ENUM( ComparisonFunction )
#undef ComparisonFunction
    }
}
