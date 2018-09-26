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
#define FLAN_COUT FLAN_COUT_STREAM
#define FLAN_CIN FLAN_CIN_STREAM

#define FLAN_CWARN FLAN_COUT_STREAM << "[WARNING] " << FLAN_FILENAME << ":" << __LINE__ << " >> "
#define FLAN_CERR FLAN_COUT_STREAM << "[ERROR] " << FLAN_FILENAME << ":" << __LINE__ << " >> "
#define FLAN_CLOG FLAN_COUT_STREAM << "[LOG] " << FLAN_FILENAME << ":" << __LINE__ << " >> "

#define FLAN_CLOG_NO_TRACE FLAN_COUT_STREAM << "[LOG] "
