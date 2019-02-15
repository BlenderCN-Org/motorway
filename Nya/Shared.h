/*
    Project Nya Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Nya source code.

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
#ifndef __NYA_SHARED_HEADER__
#define __NYA_SHARED_HEADER__

#include "Core/BuildFlags.h"
#include "Core/SystemIncludes.h"
#include "Core/Types.h"

#include "Core/Assert.h"
#include "Core/DebugHelpers.h"
#include "Core/Logging.h"

#include "Core/Hashing/CompileTimeCRC32.h"
#include "Core/Hashing/CRC32.h"

#include "Core/Allocators/AllocationHelpers.h"
#include "Core/Allocators/BaseAllocator.h"

#include "Core/Profiler.h"

// Macro redefinition annoyance (NYA_FILENAME is defined per file so it shouldn't matter anyway)
#pragma warning( push )
#pragma warning( disable : 4651 )
#pragma warning( disable : 4005 )

#endif
