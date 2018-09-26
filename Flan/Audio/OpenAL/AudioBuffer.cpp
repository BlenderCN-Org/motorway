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
#include "AudioBuffer.h"
#include "AudioContext.h"

#include <Audio/AudioFormats.h>

static constexpr ALenum OPENAL_AUDIO_FORMAT[flan::audio::eAudioFormat::AUDIO_FORMAT_COUNT] =
{
    0,
    AL_FORMAT_MONO8,
    AL_FORMAT_STEREO8,

    AL_FORMAT_MONO16,
    AL_FORMAT_STEREO16,
};

NativeAudioBuffer* flan::audio::CreateAudioBufferImpl( NativeAudioContext* audioContext )
{
    ALuint bufferHandle = 0;
    alGenBuffers( (ALuint)1, &bufferHandle );

    auto error = alcGetError( audioContext->device );
    if ( error != AL_NO_ERROR ) {
        FLAN_CERR << "Failed to create audio buffer (error code: " << error << ")" << std::endl;
        return nullptr;
    }

    NativeAudioBuffer* audioBuffer = new NativeAudioBuffer();
    audioBuffer->bufferHandle = bufferHandle;

    return audioBuffer;
}

void flan::audio::DestroyAudioBufferImpl( NativeAudioContext* audioContext, NativeAudioBuffer* audioBuffer )
{
    if ( audioBuffer->bufferHandle != 0 ) {
        alDeleteBuffers( 1, &audioBuffer->bufferHandle );
        audioBuffer->bufferHandle = 0;
    }
}

void flan::audio::UpdateAudioBufferImpl( NativeAudioContext* audioContext, NativeAudioBuffer* audioBuffer, void* dataToUpload, const std::size_t dataToUploadSize, const flan::audio::eAudioFormat audioFormat, const uint32_t sampleRate )
{
    alBufferData( audioBuffer->bufferHandle, OPENAL_AUDIO_FORMAT[audioFormat], dataToUpload, static_cast<ALCsizei>( dataToUploadSize ), static_cast<ALCsizei>( sampleRate ) );

    auto error = alcGetError( audioContext->device );
    if ( error != AL_NO_ERROR ) {
        FLAN_CWARN << "Failed to update audio buffer (error code: " << error << ")" << std::endl;
    }
}
#endif
