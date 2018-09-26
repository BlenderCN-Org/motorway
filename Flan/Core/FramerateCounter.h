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
#pragma once

struct FramerateCounter
{
public:
    float   MinFramePerSecond;
    float   MaxFramePerSecond;
    float   AvgFramePerSecond;

    float   MinDeltaTime;
    float   MaxDeltaTime;
    float   AvgDeltaTime;

public:
            FramerateCounter();
            FramerateCounter( FramerateCounter& ) = default;
            FramerateCounter& operator = ( FramerateCounter& ) = default;
            ~FramerateCounter();
    
    void    onFrame( const float frameTime );

private:
    static constexpr int SAMPLE_COUNT = 128;

private:
    float   fpsSamples[FramerateCounter::SAMPLE_COUNT];
    float   dtSamples[FramerateCounter::SAMPLE_COUNT];
    int     currentSampleIdx;
};
