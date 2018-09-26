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

#include <vector>

#include <Audio/AudioFormats.h>

struct Riff
{
    uint32_t riffMagic;
    int chunkSize; // (assuming int is 32 bits)
    uint32_t format; // "WAVE"
};

struct Format
{
    uint32_t formatMagic;
    int chunkSize;
    short format; // assuming short is 16 bits
    int byteRate;
    short align;
};

struct Wave // Actual structure of a PCM WAVE file
{
    Riff riffHeader;
    Format formatHeader;

    std::vector<uint8_t> data;
    uint32_t sampleRate;
    flan::audio::eAudioFormat audioFormat;
};

class FileSystemObject;
namespace flan
{
    namespace core
    {
        void LoadWaveFile( FileSystemObject* stream, Wave& wave );
    }
}
