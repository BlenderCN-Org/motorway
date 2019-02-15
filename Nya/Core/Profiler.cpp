#include <Shared.h>

#if NYA_DEVBUILD
#include "Profiler.h"

#include <Graphics/WorldRenderer.h>

Profiler::Profiler()
    : sectionSummaryString( "" )
    , sectionsResult{ -1.0 }
    , sectionsName{ "" }
    , sectionCount( 0 )
{
    std::fill_n( sectionsResult, MAX_PROFILE_SECTION_COUNT, -1.0 );
}

Profiler::~Profiler()
{

}

void Profiler::onFrame()
{
    sectionSummaryString.clear();

    for ( int sectionIdx = 0; sectionIdx < sectionCount; sectionIdx++ ) {
        sectionSummaryString.append( sectionsName[sectionIdx] );
        sectionSummaryString.append( "  " );
        sectionSummaryString.append( std::to_string( sectionsResult[sectionIdx] ) + "ms\n" );
    }

    sectionCount = 0;
}

void Profiler::beginSection( const std::string& sectionName )
{
    if ( sectionCount >= MAX_PROFILE_SECTION_COUNT ) {
        return;
    }

    const char sectionIdx = sectionCount;
    sectionsResult[sectionIdx] = -1.0;
    
    nya::core::StartTimer( &sectionsTimer[sectionIdx] );
    sectionsName[sectionIdx].clear();

    for ( int depth = 0; depth < recordedSectionIndexes.size(); depth++ ) {
        sectionsName[sectionIdx] += "\t";
    }
    sectionsName[sectionIdx] += sectionName;

    sectionCount++;

    sectionLock.lock();
        recordedSectionIndexes.push_back( sectionIdx );
    sectionLock.unlock();
}

void Profiler::endSection()
{
    if ( recordedSectionIndexes.empty() ) {
        return;
    }

    auto latestSectionIdx = recordedSectionIndexes.back();
    sectionsResult[latestSectionIdx] = nya::core::GetTimerDeltaAsMiliseconds( &sectionsTimer[latestSectionIdx] );

    sectionLock.lock();
        recordedSectionIndexes.pop_back();
    sectionLock.unlock();
}

const double* Profiler::getSectionResultArray() const
{
    return sectionsResult;
}

const std::string& Profiler::getProfilingSummaryString() const
{
    return sectionSummaryString;
}

Profiler g_Profiler = {};
#endif
