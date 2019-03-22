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
#include "FramerateCounter.h"

#include <Maths/Helpers.h>

#include <limits>
#include <string.h>

FramerateCounter::FramerateCounter()
    : MinFramePerSecond( std::numeric_limits<float>::max() )
    , MaxFramePerSecond( 0.0f )
    , AvgFramePerSecond( 0.0f )
    , MinDeltaTime( std::numeric_limits<float>::max() )
    , MaxDeltaTime( 0.0f )
    , AvgDeltaTime( 0.0f )
    , fpsSamples{ 0.0f }
    , dtSamples{ 0.0f }
    , currentSampleIdx( 0 )
{

}

FramerateCounter::~FramerateCounter()
{
    MinFramePerSecond = 0.0f;
    MaxFramePerSecond = 0.0f;
    AvgFramePerSecond = 0.0f;

    MinDeltaTime = 0.0f;
    MaxDeltaTime = 0.0f;
    AvgDeltaTime = 0.0f;

    memset( fpsSamples, 0, sizeof( float ) * FramerateCounter::SAMPLE_COUNT );
    memset( dtSamples, 0, sizeof( float ) * FramerateCounter::SAMPLE_COUNT );

    currentSampleIdx = 0;
}

void FramerateCounter::onFrame( const float frameTime )
{
    // NOTE Avoid divison per zero (frameTime might be 0 when logic workload is low)
    static constexpr float EPSILON = 1e-9f;

    fpsSamples[currentSampleIdx % SAMPLE_COUNT] = 1.0f / nya::maths::max( EPSILON, frameTime );

    // Update FPS
    float fps = 0.0f;
    for ( int i = 0; i < SAMPLE_COUNT; i++ ) {
        fps += fpsSamples[i];
    }
    fps /= SAMPLE_COUNT;

    MinFramePerSecond = nya::maths::min( MinFramePerSecond, fps );
    MaxFramePerSecond = nya::maths::max( MaxFramePerSecond, fps );
    AvgFramePerSecond = fps;

    // Update dt
    dtSamples[currentSampleIdx % SAMPLE_COUNT] = frameTime;
    float dt = 0.0f;
    for ( int i = 0; i < SAMPLE_COUNT; i++ ) {
        dt += dtSamples[i];
    }
    dt /= SAMPLE_COUNT;

    // Convert to ms
    dt *= 1000.0f;

    MinDeltaTime = nya::maths::min( MinDeltaTime, dt );
    MaxDeltaTime = nya::maths::max( MaxDeltaTime, dt );
    AvgDeltaTime = dt;

    // Avoid overflow (happens during long playtest)
    if ( currentSampleIdx == std::numeric_limits<int>::max() ) {
        currentSampleIdx = 0;
    }

    ++currentSampleIdx;
}
