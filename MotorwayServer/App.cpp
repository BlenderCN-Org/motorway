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
#include "App.h"

#include "AppShared.h"

#include <Core/FileLogger.h>
#include <Core/TaskManager.h>

#include <Core/Environment.h>
#include <Core/Timer.h>
#include <Core/ScopedTimer.h>
#include <Physics/DynamicsWorld.h>

#include <Framework/Tickrate.h>
#include <Network/GameHost.h>

static constexpr fnChar_t* const PROJECT_NAME = ( fnChar_t* const )FLAN_STRING( "MotorwayServer" );

App::App()
{
    // Create global instances whenever the Application ctor is called
    g_FileLogger.reset( new FileLogger( PROJECT_NAME ) );
    g_TaskManager.reset( new TaskManager() );
    g_DynamicsWorld.reset( new DynamicsWorld() );
    g_GameHost.reset( new GameHost() );
}

App::~App()
{

}

int App::launch()
{
    if ( initialize() != 0 ) {
        return 1;
    }

    // Application main loop
    Timer updateTimer = {};

    float frameTime = static_cast<float>( updateTimer.getDeltaAsSeconds() );

    float previousTime = frameTime;
    double accumulator = 0.0;

    while ( 1 ) {
        frameTime = static_cast<float>( updateTimer.getDeltaAsSeconds() );

        // Avoid spiral of death
        if ( frameTime > 0.2500f ) {
            frameTime = 0.2500f;
        }

        previousTime += frameTime;

        // Do fixed step updates and interpolate between each App state
        accumulator += frameTime;

        while ( accumulator >= flan::framework::LOGIC_DELTA ) {
            g_DynamicsWorld->update( flan::framework::LOGIC_DELTA );
            g_GameHost->OnFrame();

            accumulator -= flan::framework::LOGIC_DELTA;
        }
    }

    return 0;
}

int App::initialize()
{
    const ScopedTimer AppInitializationTimer( FLAN_STRING( "AppInitializationTimer" ) );

    FLAN_CLOG << "Initializing '" << PROJECT_NAME << "'..." << std::endl;

    // Open FileLogger (setup console output stream redirection)
    g_FileLogger->open();

    // Log generic stuff that could be useful
    FLAN_COUT << PROJECT_NAME << " " << FLAN_BUILD << std::endl
        << FLAN_BUILD_DATE << "\n" << std::endl;

    g_TaskManager->create();

    FLAN_CLOG << "Initializing physics subsystems..." << std::endl;
    g_DynamicsWorld->create();

    g_GameHost->Create( "0.0.0.0", 4583 );

    FLAN_CLOG << "Initialization done!" << std::endl;

    return 0;
}
