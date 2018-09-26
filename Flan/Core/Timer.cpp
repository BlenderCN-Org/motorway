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

#if FLAN_UNIX
#include "TimerUnix.h"
#elif FLAN_WIN
#include "TimerWin32.h"
#endif

Timer::Timer()
    : startTime( 0 )
    , previousTime( 0 )
{
    start();
}

Timer::~Timer()
{
    startTime = 0;
    previousTime = 0;
}

void Timer::start()
{
    reset();
}

double Timer::getDelta()
{
    auto currentTime = flan::core::GetCurrentTimeImpl();

    int64_t deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    return static_cast<double>( deltaTime );
}

double Timer::getDeltaAsMiliseconds()
{
    return getDelta()  * 0.001;
}

double Timer::getDeltaAsSeconds()
{
    return getDelta() * 0.000001;
}

double Timer::getElapsed()
{
    return static_cast<double>( flan::core::GetCurrentTimeImpl() - startTime );
}

double Timer::getElapsedAsMiliseconds()
{
    return getElapsed() * 0.001;
}

double Timer::getElapsedAsSeconds()
{
    return getElapsed() * 0.000001;
}

void Timer::reset()
{
    startTime = flan::core::GetCurrentTimeImpl();
    previousTime = startTime;
}
