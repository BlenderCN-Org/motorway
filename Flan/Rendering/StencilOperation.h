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
#define StencilOperation( option )\
        option( STENCIL_OPERATION_KEEP )\
        option( STENCIL_OPERATION_ZERO )\
        option( STENCIL_OPERATION_REPLACE )\
        option( STENCIL_OPERATION_INC )\
        option( STENCIL_OPERATION_INC_WRAP )\
        option( STENCIL_OPERATION_DEC )\
        option( STENCIL_OPERATION_DEC_WRAP )\
        option( STENCIL_OPERATION_INVERT )

        FLAN_LAZY_ENUM( StencilOperation )
#undef StencilOperation
    }
}
