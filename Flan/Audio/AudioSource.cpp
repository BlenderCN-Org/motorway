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
{

}

AudioSource::~AudioSource()
{

}

void AudioSource::create( AudioDevice* audioDevice )
{
    audioSource.reset( flan::audio::CreateAudioSourceImpl( audioDevice->getNativeAudioContext() ) );
}

void AudioSource::destroy( AudioDevice* audioDevice )
{
    flan::audio::DestroyAudioSourceImpl( audioDevice->getNativeAudioContext(), audioSource.get() );
}

void AudioSource::setPitch( AudioDevice* audioDevice, const float pitch )
{
    flan::audio::SetSourcePitchImpl( audioDevice->getNativeAudioContext(), audioSource.get(), pitch );
}

void AudioSource::setGain( AudioDevice* audioDevice, const float gain )
{
    flan::audio::SetSourceGainImpl( audioDevice->getNativeAudioContext(), audioSource.get(), gain );
}

void AudioSource::setPosition( AudioDevice* audioDevice, const glm::vec3& position )
{
    flan::audio::SetSourcePositionImpl( audioDevice->getNativeAudioContext(), audioSource.get(), position );
}

void AudioSource::setVelocity( AudioDevice* audioDevice, const glm::vec3& velocity )
{
    flan::audio::SetSourceVelocityImpl( audioDevice->getNativeAudioContext(), audioSource.get(), velocity );
}

void AudioSource::setLooping( AudioDevice* audioDevice, const bool isLooping )
{
    flan::audio::SetSourceLoopingImpl( audioDevice->getNativeAudioContext(), audioSource.get(), isLooping );
}

void AudioSource::bindBuffer( AudioDevice* audioDevice, AudioBuffer* audioBuffer )
{
    flan::audio::BindBufferToSourceImpl( audioDevice->getNativeAudioContext(), audioSource.get(), audioBuffer->getNativeAudioBuffer() );
}

void AudioSource::play( AudioDevice* audioDevice )
{
    flan::audio::PlaySourceImpl( audioDevice->getNativeAudioContext(), audioSource.get() );
}
