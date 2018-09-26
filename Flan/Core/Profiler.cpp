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
#include <Shared.h>

#if FLAN_DEVBUILD
#include "Profiler.h"

#include <Graphics/WorldRenderer.h>

#include <Rendering/QueryPool.h>

Profiler::Profiler()
    : sectionSummaryString( "" )
    , sectionsResult{ -1.0 }
    , sectionsName{ "" }
    , sectionCount( 0 )
    , enableDrawOnScreen( false )
    , screenPosX( 0.0f )
    , screenPosY( 0.0f )
{

}

Profiler::~Profiler()
{

}

void Profiler::create()
{
    std::fill_n( sectionsResult, MAX_PROFILE_SECTION_COUNT, -1.0 );
}

void Profiler::onFrame( WorldRenderer* worldRenderer )
{
    sectionSummaryString.clear();

    for ( unsigned int sectionIdx = 0; sectionIdx < sectionCount; sectionIdx++ ) {
        sectionSummaryString.append( sectionsName[sectionIdx] );
        sectionSummaryString.append( "  " );
        sectionSummaryString.append( std::to_string( sectionsResult[sectionIdx] ) + "ms\n" );
    }

    sectionCount = 0;

    // Submit profiler text to the renderer
    if ( enableDrawOnScreen ) {
        worldRenderer->drawDebugText( sectionSummaryString, 0.30f, screenPosX, screenPosY, 0.0f, glm::vec4( 1, 1, 1, 1 ) );
    }
}

void Profiler::beginSection( const std::string& sectionName )
{
    if ( sectionCount >= MAX_PROFILE_SECTION_COUNT ) {
        return;
    }

    const auto sectionIdx = sectionCount;
    sectionsResult[sectionIdx] = -1.0;
    sectionsTimer[sectionIdx].start();

    sectionsName[sectionIdx].clear();

    for ( int depth = 0; depth < recordedSectionIndexes.size(); depth++ )
        sectionsName[sectionIdx] += "\t";

    sectionsName[sectionIdx] += sectionName;

    sectionCount++;
    recordedSectionIndexes.push_back( sectionIdx );
}

void Profiler::endSection()
{
    if ( recordedSectionIndexes.empty() ) {
        return;
    }

    auto latestSectionIdx = recordedSectionIndexes.back();
    sectionsResult[latestSectionIdx] = sectionsTimer[latestSectionIdx].getElapsedAsMiliseconds();

    recordedSectionIndexes.pop_back();
}

void Profiler::drawOnScreen( const bool enabled, const float positionX, const float positionY )
{
    enableDrawOnScreen = enabled;
    screenPosX = positionX;
    screenPosY = positionY;
}

const double* Profiler::getSectionResultArray() const
{
    return sectionsResult;
}

Profiler g_Profiler = {};
#endif
