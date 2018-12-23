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
#include "DebugHelpersMsvc.h"

void DumpStackBacktrace()
{
    void* callers[62];

    WORD count = RtlCaptureStackBackTrace( 0, 62, callers, nullptr );

    FLAN_COUT << "Stack backtrace" << std::endl;
    for ( int i = 0; i < count; i++ ) {
        FLAN_COUT << "\tat " << i << " called " << FLAN_PRINT_HEX( callers[i] ) << std::endl;
    }
}
