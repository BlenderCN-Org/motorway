/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

#if FLAN_OPENAL

#include <OpenAL/include/AL/al.h>
#include <OpenAL/include/AL/alc.h>

struct NativeAudioContext
{
    NativeAudioContext()
        : device( nullptr )
        , context( nullptr )
    {

    }

    ALCdevice*  device;
    ALCcontext* context;
};

namespace flan
{
    namespace audio
    {
        NativeAudioContext*     CreateAudioContextImpl( BaseAllocator* allocator );
        void                    DestroyAudioContextImpl( NativeAudioContext* audioContext );
        void                    SetDefaultListenerPositionImpl( NativeAudioContext* audioContext, const glm::vec3& position );
        void                    SetDefaultListenerVelocityImpl( NativeAudioContext* audioContext, const glm::vec3& velocity );
    }
}
#endif
