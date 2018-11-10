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
#include "AudioSource.h"

#include "AudioDevice.h"
#include "AudioBuffer.h"

#if FLAN_OPENAL
#include "OpenAL/AudioContext.h"
#include "OpenAL/AudioSource.h"
#elif FLAN_XAUDIO2
#include "XAudio2/AudioContext.h"
#include "XAudio2/AudioSource.h"
#endif

AudioSource::AudioSource()
    : audioSource( nullptr )
    , sourceAllocator( nullptr )
{

}

AudioSource::~AudioSource()
{
    audioSource = nullptr;
    sourceAllocator = nullptr;
}

void AudioSource::create( AudioDevice* audioDevice, BaseAllocator* allocator )
{
    audioSource = flan::audio::CreateAudioSourceImpl( audioDevice->getNativeAudioContext(), allocator );
    sourceAllocator = allocator;
}

void AudioSource::destroy( AudioDevice* audioDevice )
{
    flan::audio::DestroyAudioSourceImpl( audioDevice->getNativeAudioContext(), audioSource );

    flan::core::free( sourceAllocator, audioSource );
}

void AudioSource::setPitch( AudioDevice* audioDevice, const float pitch )
{
    flan::audio::SetSourcePitchImpl( audioDevice->getNativeAudioContext(), audioSource, pitch );
}

void AudioSource::setGain( AudioDevice* audioDevice, const float gain )
{
    flan::audio::SetSourceGainImpl( audioDevice->getNativeAudioContext(), audioSource, gain );
}

void AudioSource::setPosition( AudioDevice* audioDevice, const glm::vec3& position )
{
    flan::audio::SetSourcePositionImpl( audioDevice->getNativeAudioContext(), audioSource, position );
}

void AudioSource::setVelocity( AudioDevice* audioDevice, const glm::vec3& velocity )
{
    flan::audio::SetSourceVelocityImpl( audioDevice->getNativeAudioContext(), audioSource, velocity );
}

void AudioSource::setLooping( AudioDevice* audioDevice, const bool isLooping )
{
    flan::audio::SetSourceLoopingImpl( audioDevice->getNativeAudioContext(), audioSource, isLooping );
}

void AudioSource::bindBuffer( AudioDevice* audioDevice, AudioBuffer* audioBuffer )
{
    flan::audio::BindBufferToSourceImpl( audioDevice->getNativeAudioContext(), audioSource, audioBuffer->getNativeAudioBuffer() );
}

void AudioSource::play( AudioDevice* audioDevice )
{
    flan::audio::PlaySourceImpl( audioDevice->getNativeAudioContext(), audioSource );
}
