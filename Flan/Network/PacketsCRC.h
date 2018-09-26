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

using packetCRC_t = fnStringHash_t;

// CRC_A_ : 'ask' packets
// CRC_R_ : 'reply' packets
// CRC_B_ : 'broadcast' packets

static constexpr packetCRC_t CRC_A_CONNECTION_REQUEST   = FLAN_STRING_HASH( "A_CONNECTION_REQUEST" );
static constexpr packetCRC_t CRC_A_CHALLENGE_SUBMIT     = FLAN_STRING_HASH( "A_CHALLENGE_SUBMIT" );
static constexpr packetCRC_t CRC_R_CHALLENGE_ANSWER     = FLAN_STRING_HASH( "R_CHALLENGE_ANSWER" );

static constexpr packetCRC_t CRC_B_INCOMING_PLAYER      = FLAN_STRING_HASH( "B_INCOMING_PLAYER" );
static constexpr packetCRC_t CRC_A_GET_PLAYERS_INFOS    = FLAN_STRING_HASH( "A_GET_PLAYERS_INFOS" );
static constexpr packetCRC_t CRC_R_GET_PLAYERS_INFOS    = FLAN_STRING_HASH( "R_GET_PLAYERS_INFOS" );

static constexpr packetCRC_t CRC_A_SUBMIT_PLAYER_INPUT  = FLAN_STRING_HASH( "A_SUBMIT_PLAYER_INPUT" );
static constexpr packetCRC_t CRC_A_SUBMIT_MESSAGE = FLAN_STRING_HASH( "A_SUBMIT_MESSAGE" );
