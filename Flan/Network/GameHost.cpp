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
#include "GameHost.h"

#include "NetworkHelpers.h"
#include "PlayerPool.h"
#include "PacketsCRC.h"

#include "ClientInputService.h"

#include <Framework/Tickrate.h>

GameHost::GameHost()
    : server( nullptr )
{
    playerPool.reset( new PlayerPool() );
}

GameHost::~GameHost()
{
    enet_host_destroy( server );
    enet_deinitialize();
}

void GameHost::Create( const char* hostIp, const unsigned short port )
{
    // Initialize ENet
    auto initResCode = enet_initialize();
    if ( initResCode != 0 ) {
        FLAN_CERR << "An error occurred while initializing ENet (error code: " << initResCode << ")" << std::endl;
        return;
    }

    ENetAddress address;
    enet_address_set_host( &address, hostIp );
    address.port = port;

    server = enet_host_create( &address /* the address to bind the server host to */,
                               32      /* allow up to 32 clients and/or outgoing connections */,
                               2      /* allow up to 2 channels to be used, 0 and 1 */,
                               0      /* assume any amount of incoming bandwidth */,
                               0      /* assume any amount of outgoing bandwidth */ );
    if ( server == nullptr ) {
        FLAN_CERR << "An error occurred while trying to create an ENet server host." << std::endl;
    }

    FLAN_CLOG << "Server Bind To " << Net_Uint32PackedIPToString( server->address.host ) << ":" << FLAN_TO_STRING( server->address.port ) << std::endl;
}

void GameHost::OnFrame()
{
    ENetEvent event;

    static constexpr int32_t PACKET_MINIMAL_SIZE = sizeof( packetCRC_t );

    while ( enet_host_service( server, &event, flan::framework::LOGIC_DELTA ) > 0 ) {
        switch ( event.type ) {
        // TODO Reimplement all the connection stuff if enet is dropped later on
        case ENET_EVENT_TYPE_CONNECT: {
            auto& player = playerPool->GetPlayerByIndex( event.peer->incomingPeerID );
            player.Host = event.peer->address.host;
            player.Port = event.peer->address.port;

            FLAN_CLOG << "Incoming connection (ID: " << FLAN_TO_STRING( event.peer->incomingPeerID )
                << ") | IP: " << Net_Uint32PackedIPToString( player.Host )
                << ":" << FLAN_TO_STRING( player.Port ) << std::endl;

            // Request infos from the user (world position, car id, ...)
            ENetPacket* packetUserInfosRequest = enet_packet_create( nullptr, sizeof( packetCRC_t ), ENET_PACKET_FLAG_RELIABLE );

            *( packetCRC_t* )packetUserInfosRequest->data = CRC_A_GET_PLAYERS_INFOS;

            enet_peer_send( event.peer, 0, packetUserInfosRequest );
            enet_host_flush( server );
            enet_packet_destroy( packetUserInfosRequest );
        } break;

        case ENET_EVENT_TYPE_RECEIVE: {
            // Ignore malformed packets
            if ( event.packet->dataLength < PACKET_MINIMAL_SIZE ) {
                break;
            }

            const packetCRC_t packetCRC = *(packetCRC_t*)( event.packet->data );
            switch ( packetCRC ) {
            case CRC_R_GET_PLAYERS_INFOS: {
                // Read informations from the player
                auto& player = playerPool->GetPlayerByIndex( event.peer->incomingPeerID );
                player.PositionWorldSpace = static_cast<glm::vec3>( event.packet->data[4] );

                // Broadcast an incoming player packet to every client connected
                // We also send the informations belonging to the client
                constexpr std::size_t packetSize = sizeof( packetCRC_t ) + sizeof( uint16_t ) + 3 * sizeof( float );
                ENetPacket* packet = enet_packet_create( nullptr, packetSize, ENET_PACKET_FLAG_RELIABLE );
                
                *( packetCRC_t* )packet->data = CRC_B_INCOMING_PLAYER;
                *( uint16_t* )( packet->data + 4 ) = event.peer->incomingPeerID;
                *( glm::vec3* )( packet->data + 6 ) = player.PositionWorldSpace;

                enet_host_broadcast( server, 0, packet );
                enet_host_flush( server );
                enet_packet_destroy( packet );
            } break;
             
            case CRC_A_SUBMIT_PLAYER_INPUT: {
                auto& player = playerPool->GetPlayerByIndex( event.peer->incomingPeerID );
                //Net_ReadClientInput( event.packet->data, player.ButtonMap );
            } break;

            case CRC_A_SUBMIT_MESSAGE:
            {
                uint16_t messageLength = static_cast<uint16_t>( event.packet->data[4] );
                uint8_t messageChannel = static_cast<uint8_t>( event.packet->data[6] );

                fnString_t message;
                message.resize( messageLength );
                memcpy( &message, &event.packet->data[8], sizeof( fnChar_t ) * messageLength );

                // TODO Filter words and stuff
            } break;

            default:
                FLAN_CWARN << "Unknown packet identifier: 0x" << std::hex << packetCRC << std::dec << std::endl;
                break;
            }

            enet_packet_destroy( event.packet );
        } break;

        case ENET_EVENT_TYPE_DISCONNECT: {
            auto& player = playerPool->GetPlayerByIndex( event.peer->incomingPeerID );

            // Force the connection down
            enet_peer_reset( event.peer );

            FLAN_CLOG << "Client Disconnected ID: " << FLAN_TO_STRING( event.peer->incomingPeerID )
                << " | IP: " << Net_Uint32PackedIPToString( player.Host )
                << ":" << FLAN_TO_STRING( player.Port ) << std::endl;

            Net_ResetPlayerEntry( player );
            enet_packet_destroy( event.packet );
        } break;
        }
    }
}

void GameHost::ReceivePlayerInput( uint8_t* rawPacketData )
{

}
