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

#include "Environment.h"

#ifdef NYA_WIN
#include "EnvironmentWin32.h"
#elif NYA_UNIX
#include "EnvironmentUnix.h"
#endif

void nya::core::RetrieveWorkingDirectory( nyaString_t& workingDirectory )
{
    nya::core::RetrieveWorkingDirectoryImpl( workingDirectory );
}

void nya::core::RetrieveHomeDirectory( nyaString_t& homeDirectory )
{
    nya::core::RetrieveHomeDirectoryImpl( homeDirectory );
}

void nya::core::RetrieveSavedGamesDirectory( nyaString_t& savedGamesDirectory )
{
    nya::core::RetrieveSavedGamesDirectoryImpl( savedGamesDirectory );
}

void nya::core::RetrieveCPUName( nyaString_t& cpuName )
{
    nya::core::RetrieveCPUNameImpl( cpuName );
}

int32_t nya::core::GetCPUCoreCount()
{
    return nya::core::GetCPUCoreCountImpl();
}

void nya::core::RetrieveOSName( nyaString_t& osName )
{
    nya::core::RetrieveOSNameImpl( osName );
}

std::size_t nya::core::GetTotalRAMSizeAsMB()
{
    return nya::core::GetTotalRAMSizeImpl();
}
