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
#include "AudioDevice.h"

#if FLAN_OPENAL
#include "OpenAL/AudioContext.h"
#elif FLAN_XAUDIO2
#include "XAudio2/AudioContext.h"
#endif

AudioDevice::AudioDevice()
    : nativeAudioContext( nullptr )
    , deviceAllocator( nullptr )
{

}

AudioDevice::~AudioDevice()
{
    nativeAudioContext = nullptr;
    deviceAllocator = nullptr;
}

NativeAudioContext* AudioDevice::getNativeAudioContext() const
{
    return nativeAudioContext;
}

void AudioDevice::destroy()
{
    flan::audio::DestroyAudioContextImpl( nativeAudioContext );

    flan::core::free( deviceAllocator, nativeAudioContext );
}

void AudioDevice::create( BaseAllocator* allocator )
{
    nativeAudioContext = flan::audio::CreateAudioContextImpl( allocator );
}

void AudioDevice::setListenerPosition( const glm::vec3& position )
{
    flan::audio::SetDefaultListenerPositionImpl( nativeAudioContext, position );
}

void AudioDevice::setListenerVelocity( const glm::vec3& velocity )
{
    flan::audio::SetDefaultListenerVelocityImpl( nativeAudioContext, velocity );
}
