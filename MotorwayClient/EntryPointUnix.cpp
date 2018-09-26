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
#if FLAN_UNIX
#include <Shared.h>
#include "App.h"
#include <Core/CommandLineArgs.h>

// Forces dedicated GPU usage if the system uses an hybrid GPU (e.g. laptops)
extern "C"
{
    __attribute__( ( visibility( "default" ) ) ) uint32_t NvOptimusEnablement = 0x00000001;
    __attribute__( ( visibility( "default" ) ) ) int32_t AmdPowerXpressRequestHighPerformance = 1;
}

using namespace flan::core;

//=====================================
//  Application EntryPoint
//      Game Entry point (Unix)
//=====================================
int main( int argc, char** argv )
{
    ReadCommandLineArgs( *argv );
    std::unique_ptr<App> appInstance( new App() );
    return appInstance->launch();
}
#endif
