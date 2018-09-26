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
#include "AudioSource.h"
#include "AudioContext.h"
#include "AudioBuffer.h"

NativeAudioSource* flan::audio::CreateAudioSourceImpl( NativeAudioContext* audioContext )
{
    ALuint sourceHandle = 0;
    alGenSources( (ALuint)1, &sourceHandle );

    auto error = alcGetError( audioContext->device );
    if ( error != AL_NO_ERROR ) {
        FLAN_CERR << "Failed to create audio source (error code: " << error << ")" << std::endl;
        return nullptr;
    }

    NativeAudioSource* audioSource = new NativeAudioSource();
    audioSource->sourceHandle = sourceHandle;

    return audioSource;
}

void flan::audio::DestroyAudioSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{
    if ( audioSource->sourceHandle != 0 ) {
        alDeleteSources( 1, &audioSource->sourceHandle );
        audioSource->sourceHandle = 0;
    }
}

void flan::audio::SetSourcePitchImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float pitch )
{
    alSourcef( audioSource->sourceHandle, AL_PITCH, pitch );
}

void flan::audio::SetSourceGainImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float gain )
{
    alSourcef( audioSource->sourceHandle, AL_GAIN, gain );
}

void flan::audio::SetSourcePositionImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& position )
{
    alSource3f( audioSource->sourceHandle, AL_POSITION, position.x, position.y, position.z );
}

void flan::audio::SetSourceVelocityImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& velocity )
{
    alSource3f( audioSource->sourceHandle, AL_VELOCITY, velocity.x, velocity.y, velocity.z );
}

void flan::audio::SetSourceLoopingImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const bool isLooping )
{
    alSourcei( audioSource->sourceHandle, AL_LOOPING, ( isLooping ) ? AL_TRUE : AL_FALSE );
}

void flan::audio::BindBufferToSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, NativeAudioBuffer* audioBuffer )
{
    alSourcei( audioSource->sourceHandle, AL_BUFFER, audioBuffer->bufferHandle );
}

void flan::audio::PlaySourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{
    alSourcePlay( audioSource->sourceHandle );
}
#endif
