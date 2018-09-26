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

#if FLAN_D3D11
#include <Rendering/ComparisonFunctions.h>
#include <d3d11.h>

namespace flan
{
    namespace rendering
    {
        static constexpr D3D11_COMPARISON_FUNC D3D11_COMPARISON_FUNCTION[ComparisonFunction_COUNT] = {
            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER,
            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS,

            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS,
            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER,

            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL,
            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER_EQUAL,

            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL,
            D3D11_COMPARISON_FUNC::D3D11_COMPARISON_EQUAL,
        };
    }
}
#endif
