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
#include <Audio/AudioFormats.h>

#include <xaudio2.h>

struct NativeAudioContext;

struct NativeAudioBuffer
{
    NativeAudioBuffer()
        : nativeBuffer{}
    {

    }

    XAUDIO2_BUFFER  nativeBuffer;
};

namespace flan
{
    namespace audio
    {
        NativeAudioBuffer*      CreateAudioBufferImpl( NativeAudioContext* audioContext );
        void                    DestroyAudioBufferImpl( NativeAudioContext* audioContext, NativeAudioBuffer* audioBuffer );
        void                    UpdateAudioBufferImpl( NativeAudioContext* audioContext, NativeAudioBuffer* audioBuffer, void* dataToUpload, const std::size_t dataToUploadSize, const flan::audio::eAudioFormat audioFormat, const uint32_t sampleRate );
    }
}
#endif
