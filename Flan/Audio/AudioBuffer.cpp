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
#include "AudioBuffer.h"

#include "AudioDevice.h"

#if FLAN_OPENAL
#include "OpenAL/AudioContext.h"
#include "OpenAL/AudioBuffer.h"
#elif FLAN_XAUDIO2
#include "XAudio2/AudioContext.h"
#include "XAudio2/AudioBuffer.h"
#endif

AudioBuffer::AudioBuffer()
    : audioBuffer( nullptr )
{

}

AudioBuffer::~AudioBuffer()
{

}

void AudioBuffer::create( AudioDevice* audioDevice )
{
    audioBuffer.reset( flan::audio::CreateAudioBufferImpl( audioDevice->getNativeAudioContext() ) );
}

void AudioBuffer::destroy( AudioDevice* audioDevice )
{
    flan::audio::DestroyAudioBufferImpl( audioDevice->getNativeAudioContext(), audioBuffer.get() );
}

void AudioBuffer::update( AudioDevice* audioDevice, void* dataToUpload, const std::size_t dataToUploadSize, const flan::audio::eAudioFormat audioFormat, const uint32_t sampleRate )
{
    flan::audio::UpdateAudioBufferImpl( audioDevice->getNativeAudioContext(), audioBuffer.get(), dataToUpload, dataToUploadSize, audioFormat, sampleRate );
}

NativeAudioBuffer* AudioBuffer::getNativeAudioBuffer() const
{
    return audioBuffer.get();
}
