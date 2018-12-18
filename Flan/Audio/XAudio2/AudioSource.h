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
#pragma once

#if FLAN_XAUDIO2
#include <xaudio2.h>
#include <x3daudio.h>

struct NativeAudioContext;
struct NativeAudioBuffer;

struct NativeAudioSource
{
    IXAudio2SourceVoice*    sourceVoice; 
    X3DAUDIO_EMITTER        emitter;
};

namespace flan
{
    namespace audio
    {
        NativeAudioSource*      CreateAudioSourceImpl( NativeAudioContext* audioContext, BaseAllocator* allocator );
        void                    DestroyAudioSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource );
        void                    SetSourcePitchImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float pitch );
        void                    SetSourceGainImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float gain );
        void                    SetSourcePositionImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& position );
        void                    SetSourceVelocityImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& velocity );
        void                    SetSourceLoopingImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const bool isLooping );

        void                    BindBufferToSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, NativeAudioBuffer* audioBuffer );
        void                    PlaySourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource );
        void                    StopSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource );
    }
}
#endif