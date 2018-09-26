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

class Scene;
class DrawCommandBuilder;
class Camera;

#include <Core/LazyEnum.h>
#include <Core/Timer.h>

#define GameMode( options )\
    options( SANDBOX )\
    options( DEV_MENU )\
    options( GAMEPLAY )

FLAN_LAZY_ENUM( GameMode )
#undef GameMode

class GameLogic
{
public:
    Scene* getCurrentScene() const { return currentScene.get(); }
    eGameMode getActiveMode() const { return currentMode;  }

public:
            GameLogic();
            GameLogic( GameLogic& ) = delete;
            GameLogic& operator = ( GameLogic& ) = delete;
            ~GameLogic();

    void    create();
    void    update( const float frameTime );

    void    collectRenderKeys( DrawCommandBuilder* drawCommandBuilder );

    void    changeScene( Scene* newScene );
    void    changeGameMode( const eGameMode nextGameMode );

private:
    eGameMode                   currentMode;
    std::unique_ptr<Scene>      currentScene;
    Timer                       autoSaveTimer;

private:
    void    loadDefaultScene();
    void    rebuildPlayerRenderPipeline( Camera* mainCamera );
};
