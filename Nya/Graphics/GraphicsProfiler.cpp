/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

#if NYA_DEVBUILD
#include "GraphicsProfiler.h"

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

GraphicsProfiler::GraphicsProfiler()
    : frameIndex( 0 )
    , timestampQueryPool( nullptr )
    , sectionSummaryString( "" )
    , sectionsResult{ -1.0 }
    , sectionsBeginQueryHandle{ 0 }
    , sectionsEndQueryHandle{ 0 }
    , sectionsName{ "" }
    , sectionCount( 0 )
    , sectionReadIndex( 0 )
    , sectionWriteIndex( 0 )
{

}

GraphicsProfiler::~GraphicsProfiler()
{

}

void GraphicsProfiler::create( RenderDevice* renderDevice )
{
    timestampQueryPool = renderDevice->createQueryPool( eQueryType::QUERY_TYPE_TIMESTAMP, TOTAL_QUERY_COUNT );

    std::fill_n( sectionsResult, TOTAL_QUERY_COUNT, -1.0 );
    std::fill_n( sectionsBeginQueryHandle, TOTAL_QUERY_COUNT, ~0 );
    std::fill_n( sectionsEndQueryHandle, TOTAL_QUERY_COUNT, ~0 );
}

void GraphicsProfiler::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyQueryPool( timestampQueryPool );
}

void GraphicsProfiler::onFrame( RenderDevice* renderDevice )
{
    NYA_PROFILE_FUNCTION

    frameIndex++;

    if ( ( frameIndex - RESULT_RETRIVAL_FRAME_LAG ) >= 0 ) {
        auto retrievedQueryCount = getSectionsResult( renderDevice );

        // Move on the next frame if at least one result is available
        if ( retrievedQueryCount > 0 ) {
            // Build profiler summary string
            sectionSummaryString.clear();

            for ( unsigned int sectionIdx = sectionReadIndex; sectionIdx < ( sectionReadIndex + retrievedQueryCount ); sectionIdx++ ) {
                sectionSummaryString.append( sectionsName[sectionIdx] );
                sectionSummaryString.append( "  " );
                sectionSummaryString.append( std::to_string( sectionsResult[sectionIdx] ) + "ms\n" );
            }
        }

        sectionReadIndex = ( sectionReadIndex + MAX_PROFILE_SECTION_COUNT ) % TOTAL_QUERY_COUNT;
    }

    sectionWriteIndex = ( sectionWriteIndex + MAX_PROFILE_SECTION_COUNT ) % TOTAL_QUERY_COUNT;
    sectionCount = 0;
}

void GraphicsProfiler::beginSection( CommandList* cmdList, const std::string& sectionName )
{
    const auto sectionIdx = ( sectionWriteIndex + sectionCount );
    sectionsResult[sectionIdx] = -1.0;
    sectionsBeginQueryHandle[sectionIdx] = cmdList->allocateQuery( timestampQueryPool );
    sectionsEndQueryHandle[sectionIdx] = cmdList->allocateQuery( timestampQueryPool );
    sectionsName[sectionIdx] = sectionName;

    sectionCount++;

    // Write start timestamp
    cmdList->writeTimestamp( timestampQueryPool, sectionsBeginQueryHandle[sectionIdx] );

    recordedSectionIndexes.push( sectionIdx );
}

void GraphicsProfiler::endSection( CommandList* cmdList )
{
    if ( recordedSectionIndexes.empty() ) {
        return;
    }

    auto latestSectionIdx = recordedSectionIndexes.back();
    cmdList->writeTimestamp( timestampQueryPool, sectionsEndQueryHandle[latestSectionIdx] );

    recordedSectionIndexes.pop();
}

std::size_t GraphicsProfiler::getSectionsResult( RenderDevice* renderDevice )
{
    for ( std::size_t sectionIdx = 0; sectionIdx < sectionCount; sectionIdx++ ) {
        const std::size_t sectionArrayIdx = ( sectionReadIndex + sectionIdx );

        uint64_t beginQueryResult = 0, endQueryResult = 0;

        auto beginQueryHandle = sectionsBeginQueryHandle[sectionArrayIdx];
        auto endQueryHandle = sectionsEndQueryHandle[sectionArrayIdx];

        // Check if the current handle is valid and available
        // If not, stop the retrival here
        if ( beginQueryHandle > TOTAL_QUERY_COUNT
          || endQueryHandle > TOTAL_QUERY_COUNT
          || !renderDevice->getQueryResult( timestampQueryPool, beginQueryHandle, beginQueryResult )
          || !renderDevice->getQueryResult( timestampQueryPool, endQueryHandle, endQueryResult ) ) {
            return sectionIdx;
        }

        uint64_t elapsedTicks = ( endQueryResult - beginQueryResult );

        double elapsedMs = renderDevice->convertTimestampToMs( timestampQueryPool, static_cast<double>( elapsedTicks ) );

        sectionsResult[sectionArrayIdx] = elapsedMs;
    }

    return sectionCount;
}

const double* GraphicsProfiler::getSectionResultArray() const
{
    return sectionsResult;
}

const std::string& GraphicsProfiler::getProfilingSummaryString() const
{
    return sectionSummaryString;
}
#endif
