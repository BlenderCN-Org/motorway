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

#if FLAN_WIN
#include <string>
#include <Shlwapi.h>
#include <Pathcch.h>
namespace flan
{
    namespace core
    {
        static void ExtractFilenameFromPath( const fnString_t& pathWithFile, fnString_t& fileNameOnly )
        {
            auto filenamePointer = PathFindFileName( pathWithFile.c_str() );

            fileNameOnly = fnString_t( filenamePointer );
        }

        static void GetFilenameWithoutExtension( const fnString_t& filenameWithExtension, fnString_t& fileWithoutExtension )
        {
            fileWithoutExtension = fnString_t( filenameWithExtension );
            PathRemoveExtension( &fileWithoutExtension[0] );
        }
    }
}
#endif