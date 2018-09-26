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
#include "PlayerPool.h"

PlayerPool::PlayerPool()
{

}

PlayerPool::~PlayerPool()
{

}

PlayerPool::Player& PlayerPool::GetPlayerByIndex( const uint16_t index )
{
    return players[index];
}

void Net_ResetPlayerEntry( PlayerPool::Player& player )
{
    player.Host = 0;
    player.Port = 0;

    player.PositionWorldSpace.x = 0.0f;
    player.PositionWorldSpace.y = 0.0f;
    player.PositionWorldSpace.z = 0.0f;
}
