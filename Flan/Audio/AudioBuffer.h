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

#include "AudioFormats.h"

class AudioDevice;
class AudioBuffer;
struct NativeAudioBuffer;

class AudioBuffer
{
public:
            AudioBuffer();
            AudioBuffer( AudioBuffer& ) = default;
            AudioBuffer& operator = ( AudioBuffer& ) = default;
            ~AudioBuffer();

    void    create( AudioDevice* audioDevice );
    void    destroy( AudioDevice* audioDevice );

    void    update( AudioDevice* audioDevice, void* dataToUpload, const std::size_t dataToUploadSize, const flan::audio::eAudioFormat audioFormat, const uint32_t sampleRate );

    NativeAudioBuffer* getNativeAudioBuffer() const;

private:
    std::unique_ptr<NativeAudioBuffer>  audioBuffer;
};

#if 0

#include <AL/al.h>
#include <AL/alc.h>

struct NativeAudioContext;

struct NativeAudioBuffer
{
    NativeAudioBuffer()
        : sourceHandle( 0 )
    {

    }

    ALCuint     sourceHandle;
};

namespace flan
{
    namespace audio
    {
        NativeAudioBuffer*      CreateAudioBufferImpl( NativeAudioContext* audioContext );
        void                    DestroyAudioBufferImpl( NativeAudioContext* audioContext, NativeAudioBuffer* AudioBuffer );
        void                    SetSourcePitchImpl( NativeAudioContext* audioContext, NativeAudioBuffer* AudioBuffer, const float pitch );
        void                    SetSourceGainImpl( NativeAudioContext* audioContext, NativeAudioBuffer* AudioBuffer, const float gain );
        void                    SetSourcePositionImpl( NativeAudioContext* audioContext, NativeAudioBuffer* AudioBuffer, const glm::vec3& position );
        void                    SetSourceVelocityImpl( NativeAudioContext* audioContext, NativeAudioBuffer* AudioBuffer, const glm::vec3& velocity );
        void                    SetSourceLoopingImpl( NativeAudioContext* audioContext, NativeAudioBuffer* AudioBuffer, const bool isLooping );

        void                    PlaySourceImpl( NativeAudioContext* audioContext, NativeAudioBuffer* AudioBuffer );
    }
}
#endif
