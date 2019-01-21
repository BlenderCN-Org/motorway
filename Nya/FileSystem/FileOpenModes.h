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
#pragma once

namespace nya
{
    namespace core
    {
        enum eFileOpenMode
        {
            FILE_OPEN_MODE_NONE = 0,
            FILE_OPEN_MODE_READ = 1,
            FILE_OPEN_MODE_WRITE = 2,
            FILE_OPEN_MODE_BINARY = 4,
            FILE_OPEN_MODE_APPEND = 8,
            FILE_OPEN_MODE_TRUNCATE = 16,
            FILE_OPEN_MODE_START_FROM_END = 32,
        };
    }
}
