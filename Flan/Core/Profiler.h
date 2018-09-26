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

#if FLAN_DEVBUILD
class QueryPool;
class RenderDevice;
class WorldRenderer;

#include "Timer.h"
#include <queue>

class Profiler
{
public:
    static constexpr int MAX_PROFILE_SECTION_COUNT = 128;

public:
            Profiler();
            Profiler( Profiler& ) = delete;
            Profiler& operator = ( Profiler& ) = delete;
            ~Profiler();

    void    create();
    void    onFrame( WorldRenderer* worldRenderer );

    void    beginSection( const std::string& sectionName );
    void    endSection();

    void    drawOnScreen( const bool enabled, const float positionX, const float positionY );

    const double* getSectionResultArray() const;

private:
    std::list<uint32_t>         recordedSectionIndexes;

    std::string                 sectionSummaryString;

    double                      sectionsResult[MAX_PROFILE_SECTION_COUNT];
    Timer                       sectionsTimer[MAX_PROFILE_SECTION_COUNT];
    std::string                 sectionsName[MAX_PROFILE_SECTION_COUNT];

    unsigned int                sectionCount;

    bool                        enableDrawOnScreen;
    float                       screenPosX;
    float                       screenPosY;
};

extern Profiler g_Profiler;
#endif
