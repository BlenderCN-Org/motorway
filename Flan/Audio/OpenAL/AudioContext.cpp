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

#if FLAN_OPENAL
#include "AudioContext.h"

NativeAudioContext* flan::audio::CreateAudioContextImpl( BaseAllocator* allocator )
{
    FLAN_CLOG << "Creating render context (OpenAL)" << std::endl;

    // Check if we can enumerate audio devices first
    const ALCchar* selectedDevice = nullptr;

    ALboolean enumeration = alcIsExtensionPresent( nullptr, "ALC_ENUMERATION_EXT" );
    if ( enumeration == AL_FALSE ) {
        FLAN_CWARN << "'ALC_ENUMERATION_EXT' is unsupported; using default audio device..." << std::endl;
    } else {
        const ALCchar* deviceList = alcGetString( nullptr, ALC_DEVICE_SPECIFIER );
        const ALCchar* device = deviceList, *next = deviceList + 1;
        size_t len = 0;

        FLAN_CLOG << "Devices List" << std::endl;

        int deviceIdx = 0;
        while ( device && *device != '\0' && next && *next != '\0' ) {
            FLAN_CLOG << "- deviceList[" << deviceIdx << "] = '" << device << "'" << std::endl;

            len = strlen( device );
            device += ( len + 1 );
            next += ( len + 2 );
        }

        selectedDevice = device;
    }

    auto device = alcOpenDevice( selectedDevice );
    if ( device == nullptr ) {
        FLAN_CERR << "Failed to open default OpenAL device! (nullptr returned)" << std::endl;
        return nullptr;
    }

    auto context = alcCreateContext( device, nullptr );
    if ( !alcMakeContextCurrent( context ) ) {
        FLAN_CERR << "Failed to create OpenAL context (error code: " << alcGetError( device ) << std::endl;
        alcCloseDevice( device );
        return nullptr;
    }

    NativeAudioContext* audioContext = flan::core::allocate<NativeAudioContext>( allocator );
    audioContext->device = device;
    audioContext->context = context;

    return audioContext;
}

void flan::audio::DestroyAudioContextImpl( NativeAudioContext* audioContext )
{
    if ( audioContext == nullptr ) {
        return;
    }

    if ( audioContext->context != nullptr ) {
        alcMakeContextCurrent( nullptr );
        alcDestroyContext( audioContext->context );
        audioContext->context = nullptr;
    }

    if ( audioContext->device != nullptr ) {
        alcCloseDevice( audioContext->device );
        audioContext->device = nullptr;
    }
}

void flan::audio::SetDefaultListenerPositionImpl( NativeAudioContext* audioContext, const glm::vec3& position )
{
    alListener3f( AL_POSITION, position.x, position.y, position.z );
}

void flan::audio::SetDefaultListenerVelocityImpl( NativeAudioContext* audioContext, const glm::vec3& velocity )
{
    alListener3f( AL_VELOCITY, velocity.x, velocity.y, velocity.z );
}
#endif
