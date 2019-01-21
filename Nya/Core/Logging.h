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

// Stream buffers
#define NYA_COUT NYA_COUT_STREAM
#define NYA_CIN NYA_CIN_STREAM

#define NYA_CWARN NYA_COUT_STREAM << "[WARNING] " << NYA_FILENAME << ":" << __LINE__ << " >> "
#define NYA_CERR NYA_COUT_STREAM << "[ERROR] " << NYA_FILENAME << ":" << __LINE__ << " >> "
#define NYA_CLOG NYA_COUT_STREAM << "[LOG] " << NYA_FILENAME << ":" << __LINE__ << " >> "

#define NYA_CLOG_NO_TRACE NYA_COUT_STREAM << "[LOG] "
#define NYA_PRINT_HEX( var ) std::hex << "0x" << var << std::dec