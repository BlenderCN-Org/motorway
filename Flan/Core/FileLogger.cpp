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
#include "FileLogger.h"

FileLogger::FileLogger( const fnString_t& applicationName )
    : applicationFileName( applicationName )
    , coutBuffer( nullptr )
    , cerrBuffer( nullptr )
    , clogBuffer( nullptr )
    , isAvailable( false )
{

}

FileLogger::~FileLogger()
{
    close();

    applicationFileName.clear();
    coutBuffer = nullptr;
    cerrBuffer = nullptr;
    clogBuffer = nullptr;
}

void FileLogger::open( const fnString_t& logFolder )
{
    constexpr fnChar_t* LOG_FILE_EXTENSION = ( fnChar_t* const )FLAN_STRING( ".log" );

    fnString_t fileName = ( logFolder + FLAN_STRING( "/" ) + applicationFileName + LOG_FILE_EXTENSION );

    fileStream = fnOfStream_t( fileName );
    isAvailable = ( fileStream.good() && fileStream.is_open() );

    if ( isAvailable ) {
        coutBuffer = FLAN_COUT_STREAM.rdbuf();
        cerrBuffer = FLAN_CERR_STREAM.rdbuf();
        clogBuffer = FLAN_CLOG_STREAM.rdbuf();

        // Redirect the console stream to the file stream
        FLAN_COUT_STREAM.rdbuf( fileStream.rdbuf() );
        FLAN_CERR_STREAM.rdbuf( fileStream.rdbuf() );
        FLAN_CLOG_STREAM.rdbuf( fileStream.rdbuf() );
    } else {
        FLAN_CERR << "Could not redirect console stream to file (is output folder read-only?)" << std::endl;
    }
}

void FileLogger::close()
{
    FLAN_CLOG << "Closing log file..." << std::endl;

    if ( isAvailable ) {
        fileStream.close();
        
        // Restore the console output
        FLAN_COUT_STREAM.rdbuf( coutBuffer );
        FLAN_CERR_STREAM.rdbuf( cerrBuffer );
        FLAN_CLOG_STREAM.rdbuf( clogBuffer );

        isAvailable = false;
    }
}
