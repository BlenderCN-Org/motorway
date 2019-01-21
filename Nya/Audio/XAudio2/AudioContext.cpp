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

#if NYA_XAUDIO2
#include <Audio/AudioDevice.h>

#include <xaudio2.h>
#include <x3daudio.h>

struct AudioContext
{
    IXAudio2* engineInstance;
    IXAudio2MasteringVoice* masteringVoice;

    X3DAUDIO_HANDLE x3dAudioHandle;
    X3DAUDIO_LISTENER defaultListener;
};

AudioDevice::~AudioDevice()
{
    if ( audioContext == nullptr ) {
        return;
    }

    if ( audioContext->masteringVoice != nullptr ) {
        audioContext->masteringVoice->DestroyVoice();
        audioContext->masteringVoice = nullptr;
    }

    if ( audioContext->engineInstance != nullptr ) {
        audioContext->engineInstance->Release();
        audioContext->engineInstance = nullptr;
    }

    nya::core::free( deviceAllocator, audioContext );
}

void AudioDevice::create()
{
    NYA_CLOG << "Creating render context (XAudio2)" << std::endl;

    IXAudio2* pXAudio2 = nullptr;

    HRESULT hr;
    if ( FAILED( hr = XAudio2Create( &pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ) ) {
        NYA_CERR << "Failed to initialize XAudio2 engine (error code: " << hr << ")" << std::endl;
        return;
    }

    IXAudio2MasteringVoice* pMasterVoice = nullptr;
    if ( FAILED( hr = pXAudio2->CreateMasteringVoice( &pMasterVoice ) ) ) {
        NYA_CERR << "Failed to initialize XAudio2 mastering voice (error code: " << hr << ")" << std::endl;
        return;
    }

    XAUDIO2_VOICE_DETAILS details;
    pMasterVoice->GetVoiceDetails( &details );

    NYA_CLOG << "-Input Channels Count: " << details.InputChannels << std::endl;
    NYA_CLOG << "-Input Sample Rate: " << details.InputSampleRate << std::endl;

    DWORD dwChannelMask;
    pMasterVoice->GetChannelMask( &dwChannelMask );

    audioContext = nya::core::allocate<AudioContext>( deviceAllocator );
    audioContext->engineInstance = pXAudio2;
    audioContext->masteringVoice = pMasterVoice;

    X3DAudioInitialize( dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, audioContext->x3dAudioHandle );
    audioContext->defaultListener = {};
}
#endif
