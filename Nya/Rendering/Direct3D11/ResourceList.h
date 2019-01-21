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

#if NYA_D3D11
struct ID3D11Buffer;
struct ID3D11SamplerState;

static constexpr int MAX_RES_COUNT = 64;

struct ResourceList
{
    template<typename Res>
    struct ResourceBinding
    {
        Res  vertexStage[MAX_RES_COUNT];
        UINT vertexBindCount;

        Res  hullStage[MAX_RES_COUNT];
        UINT hullBindCount;

        Res  domainStage[MAX_RES_COUNT];
        UINT domainBindCount;

        Res  pixelStage[MAX_RES_COUNT];
        UINT pixelBindCount;

        Res  computeStage[MAX_RES_COUNT];
        UINT computeBindCount;
    };

    ResourceBinding<ID3D11Buffer*>          constantBuffers;
    ResourceBinding<ID3D11SamplerState*>    samplers;
};
#endif
