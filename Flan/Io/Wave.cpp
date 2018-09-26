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
#include "Wave.h"

#include <FileSystem/FileSystemObject.h>

void flan::core::LoadWaveFile( FileSystemObject* stream, Wave& wave )
{
    // First read the RIFF header
    stream->read( wave.riffHeader.riffMagic );
    stream->read( wave.riffHeader.chunkSize );
    stream->read( wave.riffHeader.format );

    stream->read( wave.formatHeader.formatMagic );
    stream->read( wave.formatHeader.chunkSize );
    stream->read( wave.formatHeader.format );

    short numChannels = 0;
    stream->read( numChannels );

    stream->read( wave.sampleRate );

    stream->read( wave.formatHeader.byteRate );
    stream->read( wave.formatHeader.align );

    short bitsPerSample = 0;
    stream->read( bitsPerSample );

    if ( numChannels == 1 ) {
        switch ( bitsPerSample ) {
        case 8:
            wave.audioFormat = flan::audio::eAudioFormat::AUDIO_FORMAT_8_MONO;
            break;
        case 16:
            wave.audioFormat = flan::audio::eAudioFormat::AUDIO_FORMAT_16_MONO;
            break;
        case 32:
            wave.audioFormat = flan::audio::eAudioFormat::AUDIO_FORMAT_32_MONO;
            break;
        }
    } else if ( numChannels == 2 ) {
        switch ( bitsPerSample ) {
        case 8:
            wave.audioFormat = flan::audio::eAudioFormat::AUDIO_FORMAT_8_STEREO;
            break;
        case 16:
            wave.audioFormat = flan::audio::eAudioFormat::AUDIO_FORMAT_16_STEREO;
            break;
        case 32:
            wave.audioFormat = flan::audio::eAudioFormat::AUDIO_FORMAT_32_STEREO;
            break;
        }
    }

    uint32_t dataMagic;
    stream->read( dataMagic );

    uint32_t dataSize = 0;
    stream->read( dataSize );

    wave.data.resize( dataSize, 0x0 );
    stream->read( ( uint8_t* )&wave.data[0], dataSize * sizeof( uint8_t ) );
}
