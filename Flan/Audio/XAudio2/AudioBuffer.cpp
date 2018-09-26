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

#if FLAN_XAUDIO2
#include "AudioBuffer.h"
#include "AudioContext.h"

#include <Audio/AudioFormats.h>

NativeAudioBuffer* flan::audio::CreateAudioBufferImpl( NativeAudioContext* audioContext )
{
    NativeAudioBuffer* audioBuffer = new NativeAudioBuffer();
    audioBuffer->nativeBuffer = { 0 };
    return audioBuffer;
}

void flan::audio::DestroyAudioBufferImpl( NativeAudioContext* audioContext, NativeAudioBuffer* audioBuffer )
{
    audioBuffer->nativeBuffer = { 0 };
}

void flan::audio::UpdateAudioBufferImpl( NativeAudioContext* audioContext, NativeAudioBuffer* audioBuffer, void* dataToUpload, const std::size_t dataToUploadSize, const flan::audio::eAudioFormat audioFormat, const uint32_t sampleRate )
{
    audioBuffer->nativeBuffer.AudioBytes = dataToUploadSize;  //buffer containing audio data
    audioBuffer->nativeBuffer.pAudioData = static_cast<const BYTE*>( dataToUpload );  //size of the audio buffer in bytes
    audioBuffer->nativeBuffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
}
#endif
