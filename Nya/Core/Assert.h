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
#pragma once

#if NYA_DEVBUILD
void DumpStackBacktrace();
#endif

#if NYA_DEVBUILD
#define NYA_DEV_ASSERT( condition, format, ... ) NYA_ASSERT( condition, format, __VA_ARGS__ )
#define NYA_ASSERT( condition, format, ... ) ( condition ) ? (void)0 : Assert( format, __VA_ARGS__ )
#else
#define NYA_DEV_ASSERT( condition, format, ... ) 
#define NYA_ASSERT( condition, format, ... ) ( condition ) ? (void)0 : Assert( format, __VA_ARGS__ )
#endif

#if NYA_WIN
[[noreturn]] static void Assert( const char* description, ... )
{
    char buffer[4096] = { 0 };

    va_list argList = {};
    va_start( argList, description );
    const int textLength = vsnprintf_s( buffer, sizeof( buffer ), sizeof( buffer ) - 1, description, argList );
    va_end( argList );

    MessageBoxA( NULL, buffer, "Nya GameEngine: Assertion Failed!", MB_OK | MB_SYSTEMMODAL | MB_ICONERROR );

#if NYA_DEVBUILD
    DumpStackBacktrace();
    MessageBoxA( NULL, "Stack Backtrace has been dumped to the log file", "Nya GameEngine: Assertion Failed!", MB_OK | MB_SYSTEMMODAL | MB_ICONASTERISK );
#endif

    // TODO
    //  - Better modal window?
    //  - Add 'Report to github' button
    //  - Allow debugging?

    std::exit( 1 );
}
#endif
