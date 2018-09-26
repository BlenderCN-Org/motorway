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

#if FLAN_WIN
#include <time.h>

namespace
{
    LARGE_INTEGER getFrequency()
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency( &frequency );
        return frequency;
    }
}

namespace flan
{
    namespace core
    {
        // Return time as microseconds
        static auto GetCurrentTimeImpl() -> int64_t
        {
            static LARGE_INTEGER frequency = getFrequency();

            LARGE_INTEGER time;
            QueryPerformanceCounter( &time );

            return 1000000 * time.QuadPart / frequency.QuadPart;
        }
    }
}
#endif
