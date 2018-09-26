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

#ifdef FLAN_WIN
#include "EnvironmentWin32.h"
#elif FLAN_UNIX
#include "EnvironmentUnix.h"
#endif

void flan::core::RetrieveWorkingDirectory( fnString_t& workingDirectory )
{
    flan::core::RetrieveWorkingDirectoryImpl( workingDirectory );
}

void flan::core::RetrieveHomeDirectory( fnString_t& homeDirectory )
{
    flan::core::RetrieveHomeDirectoryImpl( homeDirectory );
}

void flan::core::RetrieveSavedGamesDirectory( fnString_t& savedGamesDirectory )
{
    flan::core::RetrieveSavedGamesDirectoryImpl( savedGamesDirectory );
}

void flan::core::RetrieveCPUName( fnString_t& cpuName )
{
    flan::core::RetrieveCPUNameImpl( cpuName );
}

int32_t flan::core::GetCPUCoreCount()
{
    return flan::core::GetCPUCoreCountImpl();
}

void flan::core::RetrieveOSName( fnString_t& osName )
{
    flan::core::RetrieveOSNameImpl( osName );
}

std::size_t flan::core::GetTotalRAMSizeAsMB()
{
    return flan::core::GetTotalRAMSizeImpl();
}
