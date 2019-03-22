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

#include <fstream>
#include <iostream>

static nyaString_t         g_ApplicationFileName;
static nyaOfStream_t       g_FileStream;

static nyaStreamBuffer_t*  g_CoutBuffer;
static nyaStreamBuffer_t*  g_CerrBuffer;
static nyaStreamBuffer_t*  g_ClogBuffer;

static bool                g_IsAvailable;

void nya::core::OpenLogFile( const nyaString_t& logFolder, const nyaString_t& applicationName )
{
    constexpr const nyaChar_t* LOG_FILE_EXTENSION = static_cast<const nyaChar_t* const>( NYA_STRING( ".log" ) );

    nyaString_t fileName = ( logFolder + NYA_STRING( "/" ) + applicationName + LOG_FILE_EXTENSION );

    g_FileStream = nyaOfStream_t( fileName );
    g_IsAvailable = ( g_FileStream.good() && g_FileStream.is_open() );

    if ( g_IsAvailable ) {
        g_CoutBuffer = NYA_COUT_STREAM.rdbuf();
        g_CerrBuffer = NYA_CERR_STREAM.rdbuf();
        g_ClogBuffer = NYA_CLOG_STREAM.rdbuf();

        // Redirect the console stream to the file stream
        NYA_COUT_STREAM.rdbuf( g_FileStream.rdbuf() );
        NYA_CERR_STREAM.rdbuf( g_FileStream.rdbuf() );
        NYA_CLOG_STREAM.rdbuf( g_FileStream.rdbuf() );
    } else {
        NYA_CERR << "Could not redirect console stream to file (is output folder read-only?)" << std::endl;
    }
}

void nya::core::CloseLogFile()
{
    NYA_CLOG << "Closing log file..." << std::endl;

    if ( g_IsAvailable ) {
        g_FileStream.close();

        // Restore the console output
        NYA_COUT_STREAM.rdbuf( g_CoutBuffer );
        NYA_CERR_STREAM.rdbuf( g_CerrBuffer );
        NYA_CLOG_STREAM.rdbuf( g_ClogBuffer );

        g_IsAvailable = false;
    }
}
