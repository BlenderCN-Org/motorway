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
#include "Timer.h"

#if NYA_UNIX
#include "TimerUnix.h"
#elif NYA_WIN
#include "TimerWin32.h"
#endif

void nya::core::StartTimer( Timer* timer )
{
    ResetTimer( timer );
}

void nya::core::ResetTimer( Timer* timer )
{
    timer->startTime = nya::core::GetCurrentTimeImpl();
    timer->previousTime = timer->startTime;
}

double nya::core::GetTimerDelta( Timer* timer )
{
    auto currentTime = nya::core::GetCurrentTimeImpl();

    int64_t deltaTime = currentTime - timer->previousTime;
    timer->previousTime = currentTime;

    return static_cast<double>( deltaTime );
}

double nya::core::GetTimerDeltaAsMiliseconds( Timer* timer )
{
    return GetTimerDelta( timer ) * 0.001f;
}

double nya::core::GetTimerDeltaAsSeconds( Timer* timer )
{
    return GetTimerDelta( timer ) * 0.000001f;
}

double nya::core::GetTimerElapsedTime( Timer* timer )
{
    return static_cast<double>( nya::core::GetCurrentTimeImpl() - timer->startTime );
}

double nya::core::GetTimerElapsedTimeAsMiliseconds( Timer* timer )
{
    return GetTimerElapsedTime( timer ) * 0.001f;
}

double nya::core::GetTimerElapsedTimeAsSeconds( Timer* timer )
{
    return GetTimerElapsedTime( timer ) * 0.000001f;
}

