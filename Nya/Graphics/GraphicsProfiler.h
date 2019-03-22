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

#if NYA_DEVBUILD
struct QueryPool;

class RenderDevice;
class CommandList;

#include <queue>

class GraphicsProfiler
{
public:
                                    GraphicsProfiler();
                                    GraphicsProfiler( GraphicsProfiler& ) = delete;
                                    GraphicsProfiler& operator = ( GraphicsProfiler& ) = delete;
                                    ~GraphicsProfiler();

    void                            create( RenderDevice* renderDevice );
    void                            destroy( RenderDevice* renderDevice );
    void                            onFrame( RenderDevice* renderDevice );

    void                            beginSection( CommandList* cmdList, const std::string& sectionName );
    void                            endSection( CommandList* cmdList );

    const double*                   getSectionResultArray() const;
    const std::string&              getProfilingSummaryString() const;

private:
    static constexpr int            RESULT_RETRIVAL_FRAME_LAG = 5;
    static constexpr int            MAX_PROFILE_SECTION_COUNT = 128;
    static constexpr int            TOTAL_QUERY_COUNT = MAX_PROFILE_SECTION_COUNT * RESULT_RETRIVAL_FRAME_LAG;

private:
    int64_t                         frameIndex;

    QueryPool*                      timestampQueryPool;
    std::queue<uint32_t>            recordedSectionIndexes;

    std::string                     sectionSummaryString;

    double                          sectionsResult[TOTAL_QUERY_COUNT];
    uint32_t                        sectionsBeginQueryHandle[TOTAL_QUERY_COUNT];
    uint32_t                        sectionsEndQueryHandle[TOTAL_QUERY_COUNT];
    std::string                     sectionsName[TOTAL_QUERY_COUNT];

    unsigned int                    sectionCount;
    unsigned int                    sectionReadIndex;
    unsigned int                    sectionWriteIndex;

private:
    std::size_t                     getSectionsResult( RenderDevice* renderDevice );
};
#endif
