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
#include <Shared.h>

#ifdef NYA_WIN
#include "Environment.h"

#include <Shlobj.h>
#include <shlwapi.h>
#include <intrin.h>
#include <VersionHelpers.h>

void nya::core::RetrieveWorkingDirectory( nyaString_t& workingDirectory )
{
    workingDirectory.reserve( NYA_MAX_PATH );

    GetModuleFileName( NULL, &workingDirectory[0], NYA_MAX_PATH );

    // Remove executable name
    PathRemoveFileSpec( &workingDirectory[0] );
    workingDirectory = nyaString_t( workingDirectory.c_str() );
    workingDirectory.append( NYA_STRING( "\\" ) );
}

void nya::core::RetrieveHomeDirectory( nyaString_t& homeDirectory )
{
    PWSTR myDocuments;

    if ( SUCCEEDED( SHGetKnownFolderPath( FOLDERID_Documents, KF_FLAG_CREATE, NULL, &myDocuments ) ) ) {
        homeDirectory = nyaString_t( myDocuments ) + NYA_STRING( "/" );
    } else {
        NYA_CWARN << "Failed to retrieve MyDocuments folder" << std::endl;
    }
}

void nya::core::RetrieveSavedGamesDirectory( nyaString_t& savedGamesDirectory )
{
    PWSTR mySavedGames;

    if ( SUCCEEDED( SHGetKnownFolderPath( FOLDERID_SavedGames, KF_FLAG_CREATE, NULL, &mySavedGames ) ) ) {
        savedGamesDirectory = nyaString_t( mySavedGames ) + NYA_STRING( "/" );
    } else {
        NYA_CWARN << "Failed to retrieve SavedGames folder" << std::endl;
    }
}

void nya::core::RetrieveCPUName( nyaString_t& cpuName )
{
    nyaString_t cpuInfosName;
    int cpuInfos[4] = { 0, 0, 0, 0 };

    __cpuid( cpuInfos, 0x80000000 );

    if ( cpuInfos[0] >= 0x80000004 ) {
        for ( int i = 0x80000002; i < 0x80000005; ++i ) {
            __cpuid( cpuInfos, i );

            for ( unsigned int info : cpuInfos ) {
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 0 ) ) & 0xFF );
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 1 ) ) & 0xFF );
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 2 ) ) & 0xFF );
                cpuInfosName += ( static_cast<char>( info >> ( 8 * 3 ) ) & 0xFF );
            }
        }
    } else {
        NYA_CERR << "Failed to retrieve CPU Name (EAX register returned empty)" << std::endl;
    }

    cpuName = cpuInfosName;
}

int32_t nya::core::GetCPUCoreCount()
{
    SYSTEM_INFO systemInfo = {};
    GetSystemInfo( &systemInfo );

    return systemInfo.dwNumberOfProcessors;
}

void nya::core::RetrieveOSName( nyaString_t& osName )
{
    if ( IsWindows10OrGreater() ) {
        osName = NYA_STRING( "Windows 10" );
    } else if ( IsWindows8Point1OrGreater() ) {
        osName = NYA_STRING( "Windows 8.1" );
    } else if ( IsWindows8OrGreater() ) {
        osName = NYA_STRING( "Windows 8" );
    } else if ( IsWindows7SP1OrGreater() ) {
        osName = NYA_STRING( "Windows 7 SP1" );
    } else if ( IsWindows7OrGreater() ) {
        osName = NYA_STRING( "Windows 7" );
    } else if ( IsWindowsVistaSP2OrGreater() ) {
        osName = NYA_STRING( "Windows Vista SP2" );
    } else if ( IsWindowsVistaSP1OrGreater() ) {
        osName = NYA_STRING( "Windows Vista SP1" );
    } else if ( IsWindowsVistaOrGreater() ) {
        osName = NYA_STRING( "Windows Vista" );
    } else if ( IsWindowsXPSP3OrGreater() ) {
        osName = NYA_STRING( "Windows XP SP3" );
    } else if ( IsWindowsXPSP2OrGreater() ) {
        osName = NYA_STRING( "Windows XP SP2" );
    } else if ( IsWindowsXPSP1OrGreater() ) {
        osName = NYA_STRING( "Windows XP SP1" );
    } else if ( IsWindowsXPOrGreater() ) {
        osName = NYA_STRING( "Windows XP" );
    } else {
        osName = NYA_STRING( "Windows NT" );
    }
}

std::size_t nya::core::GetTotalRAMSizeAsMB()
{
    MEMORYSTATUSEX memoryStatusEx = {};
    memoryStatusEx.dwLength = sizeof( memoryStatusEx );

    BOOL operationResult = GlobalMemoryStatusEx( &memoryStatusEx );
    return ( operationResult == TRUE ) ? static_cast<uint64_t>( memoryStatusEx.ullTotalPhys >> 20 ) : 0;
}
#endif
