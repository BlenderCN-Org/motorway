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

#ifdef FLAN_UNIX
#include "EnvironmentUnix.h"

#include <cpuid.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <sys/sysinfo.h>

void flan::core::RetrieveWorkingDirectoryImpl( fnString_t& workingDirectory )
{
    workingDirectory.reserve( FLAN_MAX_PATH );
    getcwd( &workingDirectory[0], FLAN_MAX_PATH );

    workingDirectory = fnString_t( workingDirectory.c_str() );
}

void flan::core::RetrieveHomeDirectoryImpl( fnString_t& homeDirectory )
{
    const fnChar_t* envVarHome = getenv( "XDG_DATA_HOME" );

    if ( envVarHome == nullptr ) {
        FLAN_CWARN << "$XDG_DATA_HOME is undefined; using $HOME as fallback..." << std::endl;

        envVarHome = getenv( "HOME" );
    }

    homeDirectory = fnString_t( envVarHome ) + "/";
}

void flan::core::RetrieveSavedGamesDirectoryImpl( fnString_t& savedGamesDirectory )
{

}

void flan::core::RetrieveCPUNameImpl( fnString_t& cpuName )
{
    fnString_t cpuInfosName;
    unsigned int cpuInfos[4] = { 0, 0, 0, 0 };

    __get_cpuid( 0x80000000, &cpuInfos[0], &cpuInfos[1], &cpuInfos[2], &cpuInfos[3] );

    if ( cpuInfos[0] >= 0x80000004 ) {
        for ( unsigned int i = 0x80000002; i < 0x80000005; ++i ) {
            __get_cpuid( i, &cpuInfos[0], &cpuInfos[1], &cpuInfos[2], &cpuInfos[3] );

            for ( unsigned int info : cpuInfos ) {
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 0 ) ) & 0xFF );
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 1 ) ) & 0xFF );
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 2 ) ) & 0xFF );
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 3 ) ) & 0xFF );
            }
        }
    } else {
        FLAN_CERR << "Failed to retrieve CPU Name (EAX register returned empty)" << std::endl;
    }

    cpuName = cpuInfosName;
}

int32_t flan::core::GetCPUCoreCountImpl()
{
    return sysconf( _SC_NPROCESSORS_ONLN );
}

void flan::core::RetrieveOSNameImpl( fnString_t& osName )
{
    std::string unixVersion = "Unix";

    std::ifstream unixVersionFile( "/proc/version" );

    if ( unixVersionFile.is_open() ) {
        unixVersionFile >> unixVersion;
        unixVersionFile.close();
    }

    osName = unixVersion;
}

std::size_t flan::core::GetTotalRAMSizeImpl()
{
    struct sysinfo infos = {};

    if ( sysinfo( &infos ) == 0 ) {
        return ( infos.totalram >> 20 );
    }

    return 0;
}
#endif
