/*
    Project Nya Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Nya source code.

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
#if NYA_UNIX
#include "Editor.h"
// Forces dedicated GPU usage if the system uses an hybrid GPU (e.g. laptops)
extern "C"
{
    __attribute__( ( visibility( "default" ) ) ) unsigned int NvOptimusEnablement = 0x00000001;
    __attribute__( ( visibility( "default" ) ) ) int AmdPowerXpressRequestHighPerformance = 1;
}

// Application EntryPoint (Unix)
int main( int argc, char** argv )
{
    nya::editor::Start();
    return 0;
}
#endif
