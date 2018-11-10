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
#ifndef __FLAN_SHARED_HEADER__
#define __FLAN_SHARED_HEADER__

#include "Core/BuildFlags.h"
#include "Core/SystemIncludes.h"
#include "Core/Types.h"

#if FLAN_DEVBUILD
#include "Core/HeapTracker.h"
#endif

#include "Core/Logging.h"

// NOTE Hashing function should not be changed between version
// (otherwise a lot of modules/assets might break)
#include "Core/Hashing/CompileTimeCRC32.h"
#include "Core/Hashing/CRC32.h"

#include "Core/EnvironmentVariables.h"
#include "Core/MemoryAlignementHelpers.h"
#include "Core/Allocators/BaseAllocator.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>

// Macro redefinition annoyance (FLAN_FILENAME is defined per file so it shouldn't matter anyway)
#pragma warning( push )
#pragma warning( disable : 4651 )
#pragma warning( disable : 4005 )

#endif
