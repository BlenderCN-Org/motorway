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
#include "Shared.h"

#if NYA_MSVC
#include "DebugHelpersMsvc.h"

#include <dbghelp.h>

typedef DWORD( _stdcall *SymSetOptionsProc )( DWORD ) ;
typedef DWORD( _stdcall *SymInitializeProc )( HANDLE, PCSTR, BOOL );
typedef DWORD( _stdcall *SymGetSymFromAddr64Proc )( HANDLE, DWORD64, PDWORD64, PIMAGEHLP_SYMBOL64 );

void DumpStackBacktrace()
{
    HMODULE dbgHelp = LoadLibrary( NYA_STRING( "dbghelp.dll" ) );

    SymSetOptionsProc symSetOptions = ( SymSetOptionsProc )GetProcAddress( dbgHelp, "SymSetOptions" );
    SymInitializeProc symInitialize = ( SymInitializeProc )GetProcAddress( dbgHelp, "SymInitialize" );
    SymGetSymFromAddr64Proc symGetSymFromAddr64 = ( SymGetSymFromAddr64Proc )GetProcAddress( dbgHelp, "SymGetSymFromAddr64" );

    HANDLE hProcess = GetCurrentProcess();
    symSetOptions( SYMOPT_DEFERRED_LOADS );
    symInitialize( hProcess, NULL, TRUE );

    // NOTE Explicitly ignore the first frame ( = the function itself)
    void* callers[62];
    WORD count = RtlCaptureStackBackTrace( 1, 62, callers, nullptr );

    unsigned char symbolBuffer[sizeof( IMAGEHLP_SYMBOL64 ) + 512];
    PIMAGEHLP_SYMBOL64 pSymbol = reinterpret_cast< PIMAGEHLP_SYMBOL64 >( symbolBuffer );
    memset( pSymbol, 0, sizeof( IMAGEHLP_SYMBOL64 ) + 512 );
    pSymbol->SizeOfStruct = sizeof( IMAGEHLP_SYMBOL64 );
    pSymbol->MaxNameLength = 512;

    NYA_CLOG << "Dumping stack backtrace...\n==============================" << std::endl;
    for ( int i = 0; i < count; i++ ) {
        // Retrieve frame infos
        if ( !symGetSymFromAddr64( hProcess, ( DWORD64 )callers[i], nullptr, pSymbol ) ) {
            continue;
        }

        NYA_COUT << i << " : " << NYA_PRINT_HEX( pSymbol->Address ) << " | " << pSymbol->Name << std::endl;
    }
    NYA_COUT << "==============================\nEnd" << std::endl;
}
#endif

