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
#pragma once

class FileLogger
{
public:
                        FileLogger( const fnString_t& applicationName = FLAN_STRING( "Flan GameEngine" ) );
                        FileLogger( FileLogger& ) = default;
                        FileLogger& operator = ( FileLogger& ) = delete;
                        ~FileLogger();

    void                open( const fnString_t& logFolder = FLAN_STRING( "./logs/" ) );
    void                close();

private:
    fnString_t          applicationFileName;
    fnOfStream_t        fileStream;

    fnStreamBuffer_t*   coutBuffer;
    fnStreamBuffer_t*   cerrBuffer;
    fnStreamBuffer_t*   clogBuffer;

    bool                isAvailable;
};
