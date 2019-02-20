#pragma once

#if NYA_DEVBUILD
#include "Timer.h"

#include <list>

class Profiler
{
public:
                                Profiler();
                                Profiler( Profiler& ) = delete;
                                Profiler& operator = ( Profiler& ) = delete;
                                ~Profiler();

    void                        onFrame();

    void                        beginSection( const std::string& sectionName );
    void                        endSection();

    const double*               getSectionResultArray() const;
    const std::string&          getProfilingSummaryString() const;

private:
    static constexpr int        MAX_PROFILE_SECTION_COUNT = 128;

private:
    std::list<uint32_t>         recordedSectionIndexes;

    std::string                 sectionSummaryString;

    double                      sectionsResult[MAX_PROFILE_SECTION_COUNT];
    Timer                       sectionsTimer[MAX_PROFILE_SECTION_COUNT];
    std::string                 sectionsName[MAX_PROFILE_SECTION_COUNT];

    unsigned int                sectionCount;
};

extern Profiler                 g_Profiler;

struct ProfileSection
{
    ProfileSection( const std::string& name ) {
        g_Profiler.beginSection( name );
    }

    ~ProfileSection() {
        g_Profiler.endSection();
    }
};

#define NYA_PROFILE( section ) ProfileSection( section );
#define NYA_PROFILE_FUNCTION ProfileSection( __FUNCTION__ );

// Profile code block with an explicit scope
// Use NYA_PROFILE for automatic scope-based profiling
#define NYA_BEGIN_PROFILE_SCOPE( section ) g_Profiler.beginSection( section );
#define NYA_END_PROFILE_SCOPE() g_Profiler.endSection();
#else
#define NYA_PROFILE( section )
#define NYA_BEGIN_PROFILE_SCOPE( section )
#define NYA_END_PROFILE_SCOPE( section )
#endif
