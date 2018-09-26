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

class AudioDevice;
class AudioBuffer;
struct NativeAudioSource;

class AudioSource
{
public:
            AudioSource();
            AudioSource( AudioSource& ) = default;
            AudioSource& operator = ( AudioSource& ) = default;
            ~AudioSource();

    void    create( AudioDevice* audioDevice );
    void    destroy( AudioDevice* audioDevice );

    void    setPitch( AudioDevice* audioDevice, const float pitch );
    void    setGain( AudioDevice* audioDevice, const float gain );
    void    setPosition( AudioDevice* audioDevice, const glm::vec3& position );
    void    setVelocity( AudioDevice* audioDevice, const glm::vec3& velocity );
    void    setLooping( AudioDevice* audioDevice, const bool isLooping );

    void    bindBuffer( AudioDevice* audioDevice, AudioBuffer* audioBuffer );
    void    play( AudioDevice* audioDevice );

private:
    std::unique_ptr<NativeAudioSource>  audioSource;
};
