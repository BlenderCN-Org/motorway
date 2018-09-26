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

static constexpr int PLAYER_POOL_CAPACITY = 1024;

#include <glm/glm.hpp>

class PlayerPool
{
public:
    struct Player
    {
        bool        ButtonMap[256];
        uint32_t    Host;
        uint16_t    Port;
        glm::vec3   PositionWorldSpace;
    };

public:
                PlayerPool();
                PlayerPool( PlayerPool& ) = delete;
                ~PlayerPool();

    Player&     GetPlayerByIndex( const uint16_t index );

private:
    Player      players[PLAYER_POOL_CAPACITY];
};

void Net_ResetPlayerEntry( PlayerPool::Player& player );
