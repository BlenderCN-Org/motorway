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
#include "AudioSource.h"
#include "AudioContext.h"
#include "AudioBuffer.h"

NativeAudioSource* flan::audio::CreateAudioSourceImpl( NativeAudioContext* audioContext )
{
    IXAudio2SourceVoice* pSourceVoice;
    HRESULT hr = S_OK;
    WAVEFORMATEX waveFormat;
    return nullptr;
}

void flan::audio::DestroyAudioSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{

}

void flan::audio::SetSourcePitchImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float pitch )
{

}

void flan::audio::SetSourceGainImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float gain )
{

}

void flan::audio::SetSourcePositionImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& position )
{

}

void flan::audio::SetSourceVelocityImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& velocity )
{

}

void flan::audio::SetSourceLoopingImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const bool isLooping )
{

}

void flan::audio::BindBufferToSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, NativeAudioBuffer* audioBuffer )
{

}

void flan::audio::PlaySourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{

}
#endif
