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
#include "CommandLineArgs.h"

#include <Core/Hashing/CRC32.h>

FLAN_DEV_VAR( RebuildGameCfgFile, "Rebuild Game Config File", false, bool )
FLAN_DEV_VAR( RebuildInputCfgFile, "Rebuild Input Config File", false, bool )
FLAN_DEV_VAR( EnableDebugRenderDevice, "Enable RenderDevice Debug Layer", false, bool )

void flan::core::ReadCommandLineArgs( char* cmdLineArgs )
{
    FLAN_CLOG << "Parsing command line arguments..." << std::endl;

	char* arg = strtok( cmdLineArgs, " " );

	while ( arg != nullptr ) {
        fnStringHash_t argHash = flan::core::CRC32( arg );

        switch ( argHash ) {
        case FLAN_STRING_HASH( "--rebuildGameCfg" ):
        {
            char* varState = strtok( nullptr, " " );
            if ( varState == nullptr ) {
                break;
            }

            RebuildGameCfgFile = ( strcmp( varState, "1" ) == 0 );
            break;
        }
        case FLAN_STRING_HASH( "--rebuildInputCfg" ):
        {
            char* varState = strtok( nullptr, " " );
            if ( varState == nullptr ) {
                break;
            }

            RebuildInputCfgFile = ( strcmp( varState, "1" ) == 0 );
            break;
        }
        case FLAN_STRING_HASH( "--enableDbgRenderDev" ):
        {
            char* varState = strtok( nullptr, " " );
            if ( varState == nullptr ) {
                break;
            }

            EnableDebugRenderDevice = ( strcmp( varState, "1" ) == 0 );
            break;
        }
        }

		arg = strtok( nullptr, " " );
	}
}
