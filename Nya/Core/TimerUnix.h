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

#if NYA_UNIX
#include <time.h>

namespace nya
{
    namespace core
    {
        // Return time as microseconds
        static int64_t GetCurrentTimeImpl()
        {
            timespec time;
            clock_gettime( CLOCK_MONOTONIC, &time );

            return ( static_cast<int64_t>( time.tv_sec ) * 1000000 + time.tv_nsec / 1000 );
        }
    }
}
#endif
