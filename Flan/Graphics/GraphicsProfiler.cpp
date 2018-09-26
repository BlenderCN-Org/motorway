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
#include "GraphicsProfiler.h"

#include "WorldRenderer.h"

#include <Rendering/QueryPool.h>

#include <Core/Profiler.h>

GraphicsProfiler::GraphicsProfiler()
    : timestampQueryPool( nullptr )
    , sectionSummaryString( "" )
    , sectionsResult{ -1.0 }
    , sectionsBeginQueryHandle{ 0 }
    , sectionsEndQueryHandle{ 0 }
    , sectionsName{ "" }
    , sectionCount( 0 )
    , sectionReadIndex( 0 )
    , sectionWriteIndex( 0 )
    , enableDrawOnScreen( false )
    , screenPosX( 0.0f )
    , screenPosY( 0.0f )
{

}

GraphicsProfiler::~GraphicsProfiler()
{

}

void GraphicsProfiler::create( RenderDevice* renderDevice )
{
    timestampQueryPool.reset( new QueryPool() );
    timestampQueryPool->create( renderDevice, eQueryType::QUERY_TYPE_TIMESTAMP, TOTAL_QUERY_COUNT );

    std::fill_n( sectionsResult, TOTAL_QUERY_COUNT, -1.0 );
    std::fill_n( sectionsBeginQueryHandle, TOTAL_QUERY_COUNT, ~0 );
    std::fill_n( sectionsEndQueryHandle, TOTAL_QUERY_COUNT, ~0 );
}

void GraphicsProfiler::destroy( RenderDevice* renderDevice )
{
    timestampQueryPool->destroy( renderDevice );
}

void GraphicsProfiler::onFrame( RenderDevice* renderDevice, WorldRenderer* worldRenderer )
{
    if ( (int)( (int)worldRenderer->getFrameNumber() - RESULT_RETRIVAL_FRAME_LAG ) >= 0 ) {
        auto retrievedQueryCount = getSectionsResult( renderDevice );

        // Move on the next frame if at least one result is available
        if ( retrievedQueryCount > 0 && enableDrawOnScreen ) {
            // Build profiler summary string
            sectionSummaryString.clear();

            for ( unsigned int sectionIdx = sectionReadIndex; sectionIdx < ( sectionReadIndex + retrievedQueryCount ); sectionIdx++ ) {
                sectionSummaryString.append( sectionsName[sectionIdx] );
                sectionSummaryString.append( "  " );
                sectionSummaryString.append( std::to_string( sectionsResult[sectionIdx] ) + "ms\n" );
            }
        }

        sectionReadIndex += MAX_PROFILE_SECTION_COUNT;
        if ( sectionReadIndex == TOTAL_QUERY_COUNT ) {
            sectionReadIndex = 0;
        }
    }

    sectionWriteIndex += MAX_PROFILE_SECTION_COUNT;
    if ( sectionWriteIndex == TOTAL_QUERY_COUNT ) {
        sectionWriteIndex = 0;
    }

    sectionCount = 0;

    // Submit profiler text to the renderer
    if ( enableDrawOnScreen ) {
        worldRenderer->drawDebugText( sectionSummaryString, 0.3f, screenPosX, screenPosY, 0.0f, glm::vec4( 1, 1, 1, 1 ) );
    }
}

void GraphicsProfiler::beginSection( CommandList* cmdList, const std::string& sectionName )
{
    const auto sectionIdx = ( sectionWriteIndex + sectionCount );
    sectionsResult[sectionIdx] = -1.0;
    sectionsBeginQueryHandle[sectionIdx] = timestampQueryPool->allocateQueryHandle( cmdList );
    sectionsEndQueryHandle[sectionIdx] = timestampQueryPool->allocateQueryHandle( cmdList );
    sectionsName[sectionIdx] = sectionName;

    sectionCount++;

    // Write start timestamp
    timestampQueryPool->writeTimestamp( cmdList, sectionsBeginQueryHandle[sectionIdx] );

    recordedSectionIndexes.push( sectionIdx );
}

void GraphicsProfiler::endSection( CommandList* cmdList )
{
    if ( recordedSectionIndexes.empty() ) {
        return;
    }

    auto latestSectionIdx = recordedSectionIndexes.back();
    timestampQueryPool->writeTimestamp( cmdList, sectionsEndQueryHandle[latestSectionIdx] );

    recordedSectionIndexes.pop();
}

std::size_t GraphicsProfiler::getSectionsResult( RenderDevice* renderDevice )
{
    g_Profiler.beginSection( "GraphicsProfiler::getSectionsResult" );
    for ( std::size_t sectionIdx = 0; sectionIdx < sectionCount; sectionIdx++ ) {
        const auto sectionArrayIdx = ( sectionReadIndex + sectionIdx );

        uint64_t beginQueryResult = 0, endQueryResult = 0;

        auto beginQueryHandle = sectionsBeginQueryHandle[sectionArrayIdx];
        auto endQueryHandle = sectionsEndQueryHandle[sectionArrayIdx];

        // Check if the current handle is valid and available
        // If not, stop the retrival here
        if ( beginQueryHandle > TOTAL_QUERY_COUNT
          || endQueryHandle > TOTAL_QUERY_COUNT
          || !timestampQueryPool->retrieveQueryResult( renderDevice, beginQueryHandle, beginQueryResult )
          || !timestampQueryPool->retrieveQueryResult( renderDevice, endQueryHandle, endQueryResult ) ) {
            g_Profiler.endSection();
            return sectionIdx;
        }

        uint64_t elapsedTicks = ( endQueryResult - beginQueryResult );

        double elapsedMs = flan::rendering::ConvertTimestampToMs( static_cast<double>( elapsedTicks ) );

        sectionsResult[sectionArrayIdx] = elapsedMs;
    }
    g_Profiler.endSection();

    return sectionCount;
}

void GraphicsProfiler::drawOnScreen( const bool enabled, const float positionX, const float positionY )
{
    enableDrawOnScreen = enabled;
    screenPosX = positionX;
    screenPosY = positionY;
}

const double* GraphicsProfiler::getSectionResultArray() const
{
    return sectionsResult;
}
#endif
