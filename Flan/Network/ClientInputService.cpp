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
#include "ClientInputService.h"

#include "PacketsCRC.h"
//#include <System/Input/InputKeys.h>
//
//void Net_SendClientInput( ENetHost* fromHost, ENetPeer* toPeer, const bool* buttonsState )
//{
//    constexpr auto buttonBitfieldSize = KEYBOARD_KEY_COUNT / sizeof( uint64_t );
//    ENetPacket* packet = enet_packet_create( nullptr, sizeof( packetCRC_t ) + buttonBitfieldSize, ENET_PACKET_FLAG_RELIABLE );
//
//    *(packetCRC_t*)packet->data = CRC_A_SUBMIT_PLAYER_INPUT;
//    
//    int i = 0;
//    uint64_t keyState = 0;
//
//    // Pack boolean map into a bitfield
//    for ( int j = 0; j < KEYBOARD_KEY_COUNT / 64; j++ ) {
//        for ( i = 0; i < 64 && i < KEYBOARD_KEY_COUNT; i++ ) {
//            if ( buttonsState[i] )
//                keyState ^= 1ull << i;
//        }
//        *(uint64_t*)( packet->data + ( sizeof( packetCRC_t ) + j * sizeof( uint64_t ) ) ) = keyState;
//    }
//
//
//    enet_peer_send( toPeer, 0, packet );
//    enet_host_flush( fromHost );
//    enet_packet_destroy( packet );
//}
//
//void Net_ReadClientInput( uint8_t* packetData, bool* buttonsStateToUpdate )
//{
//    int i = 0;
//    uint64_t packetDataOffset = sizeof( packetCRC_t );
//
//    // Unpack bitfield
//    for ( int j = 0; j < KEYBOARD_KEY_COUNT; j += i ) {
//        uint64_t keyState = static_cast<uint64_t>( packetData[packetDataOffset] );
//
//        for ( i = 0; i < 64 && i < KEYBOARD_KEY_COUNT; i++ ) {
//            buttonsStateToUpdate[i] = ( ( keyState >> i ) & 1U );
//        }
//
//        packetDataOffset += sizeof( uint64_t );
//    }
//}
