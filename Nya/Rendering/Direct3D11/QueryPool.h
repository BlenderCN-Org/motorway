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
#include <vector>

struct ID3D11Query;
enum D3D11_QUERY;

struct QueryPool
{
    ID3D11Query**   queryHandles;
    ID3D11Query**   disjointQueryHandles;
    D3D11_QUERY     targetType;
    unsigned int    capacity;
    unsigned int    currentAllocableIndex;

    unsigned int    currentDisjointAllocableIndex;
    std::vector<unsigned int> queryDisjointTable;
    std::vector<uint64_t> disjointQueriesResult;
    uint64_t        deviceClockFrequency;
    int             frequencySampleCount;
    bool            isRecordingDisjointQuery;
};
#endif
