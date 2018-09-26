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
#include "GameClient.h"

#include <Network/PacketsCRC.h>
#include <Framework/Tickrate.h>

GameClient::GameClient()
    : client( nullptr )
{

}

GameClient::~GameClient()
{
    enet_host_destroy( client );
    enet_deinitialize();
}

void GameClient::ConnectTo( const char* hostIp, const unsigned short port )
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

    client = enet_host_create( nullptr /* create a client host */,
                               1 /* only allow 1 outgoing connection */,
                               2 /* allow up 2 channels to be used, 0 and 1 */,
                               57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
                               14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */ );

    if ( client == NULL ) {
        FLAN_CERR << "An error occurred while trying to create an ENet client." << std::endl;
    }

    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect( client, &address, 2, 0 );
    if ( peer == NULL ) {
        FLAN_CERR << "No available peers for initiating an ENet connection." << std::endl;
    }

    ENetEvent event;

    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if ( enet_host_service( client, &event, 5000 ) > 0 &&
         event.type == ENET_EVENT_TYPE_CONNECT ) {
        FLAN_CLOG << "Connection to " << hostIp << ":" << FLAN_TO_STRING( address.port ) << " succeeded." << std::endl;
        
        activeServer = "IP: " + std::string( hostIp ) + ":" + std::to_string( port );
    } else {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset( peer );
        FLAN_CERR << "Connection to " << hostIp << ":" << FLAN_TO_STRING( address.port ) << " failed (connection timed out)" << std::endl;
    }

}

static void InstallCar( uint8_t* packetData )
{
    //// Read informations from the stream
    //auto worldPosition = static_cast<glm::vec3>( packetData[6] );

    //// Add the game object to the local game instance
    //Vehicle* vehicleToInstall = new Vehicle( worldPosition );
}

void GameClient::OnFrame()
{
    ENetEvent event;

    static constexpr int32_t PACKET_MINIMAL_SIZE = sizeof( packetCRC_t );

    while ( enet_host_service( client, &event, flan::framework::LOGIC_DELTA ) > 0 ) {
        switch ( event.type ) {
        case ENET_EVENT_TYPE_RECEIVE:
            // Ignore malformed packets
            if ( event.packet->dataLength < PACKET_MINIMAL_SIZE ) {
                break;
            }

            switch ( *(packetCRC_t*)( event.packet->data ) ) {
            case CRC_A_GET_PLAYERS_INFOS: {
                ENetPacket* packet = enet_packet_create( nullptr, sizeof( packetCRC_t ) + 3 * sizeof( float ), ENET_PACKET_FLAG_RELIABLE );
                float xtest = 0, ytest = 1, ztest = 1;
                *(packetCRC_t*)packet->data = CRC_R_GET_PLAYERS_INFOS;

                memcpy( &packet->data[4], &xtest, sizeof( float ) );
                memcpy( &packet->data[8], &ytest, sizeof( float ) );
                memcpy( &packet->data[12], &ztest, sizeof( float ) );

                enet_peer_send( event.peer, 0, packet );
                enet_host_flush( client );
                enet_packet_destroy( packet );
            } break;

            case CRC_B_INCOMING_PLAYER: {
                uint16_t playerConnectedIndex = static_cast<uint16_t>( event.packet->data[4] );

                // If the mesage was originaly sent by the client, ignore
                if ( playerConnectedIndex == peer->outgoingPeerID ) {
                    break;
                }

                InstallCar( event.packet->data );
            } break;
            }

            /*PH_COUT << "A packet of length " << std::to_string( event.packet->dataLength )
                << " containing " << event.packet->data
                << " was received from " <<
                event.peer->data << " on channel " << event.channelID << "." << std::endl;*/
            enet_packet_destroy( event.packet );
            break;
        }
    }
}
