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
#include "AudioContext.h"

NativeAudioContext* flan::audio::CreateAudioContextImpl( BaseAllocator* allocator )
{
    FLAN_CLOG << "Creating render context (XAudio2)" << std::endl;

    IXAudio2* pXAudio2 = nullptr;

    HRESULT hr;
    if ( FAILED( hr = XAudio2Create( &pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ) ) {
        FLAN_CERR << "Failed to initialize XAudio2 engine (error code: " << hr << ")" << std::endl;
        return nullptr;
    }

    IXAudio2MasteringVoice* pMasterVoice = nullptr;
    if ( FAILED( hr = pXAudio2->CreateMasteringVoice( &pMasterVoice ) ) ) {
        FLAN_CERR << "Failed to initialize XAudio2 mastering voice (error code: " << hr << ")" << std::endl;
        return nullptr;
    }

#if FLAN_DEVBUILD
    XAUDIO2_DEBUG_CONFIGURATION debugConfiguration = {};
    debugConfiguration.TraceMask = ~0;
    debugConfiguration.BreakMask = ~0;

    pXAudio2->SetDebugConfiguration( &debugConfiguration );
#endif

    DWORD dwChannelMask;
    pMasterVoice->GetChannelMask( &dwChannelMask );

    NativeAudioContext* audioContext = new NativeAudioContext();
    audioContext->engineInstance = pXAudio2;
    audioContext->masteringVoice = pMasterVoice;

    pMasterVoice->GetVoiceDetails( &audioContext->details );

    FLAN_CLOG << "-Input Channels Count: " << audioContext->details.InputChannels << std::endl;
    FLAN_CLOG << "-Input Sample Rate: " << audioContext->details.InputSampleRate << std::endl;

    X3DAudioInitialize( dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, audioContext->x3dAudioHandle );
    audioContext->defaultListener = {};

    audioContext->dspSettings = { 0 };
    audioContext->dspSettings.SrcChannelCount = 1;
    audioContext->dspSettings.DstChannelCount = audioContext->details.InputChannels;
    audioContext->dspSettings.pMatrixCoefficients = flan::core::allocateArray<FLOAT32>( allocator, audioContext->details.InputChannels );

    return audioContext;
}

void flan::audio::DestroyAudioContextImpl( NativeAudioContext* audioContext )
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
}

void flan::audio::SetDefaultListenerPositionImpl( NativeAudioContext* audioContext, const glm::vec3& position )
{
    audioContext->defaultListener.Position.x = position.x;
    audioContext->defaultListener.Position.y = position.y;
    audioContext->defaultListener.Position.z = position.z;
}

void flan::audio::SetDefaultListenerVelocityImpl( NativeAudioContext* audioContext, const glm::vec3& velocity )
{
    audioContext->defaultListener.Velocity.x = velocity.x;
    audioContext->defaultListener.Velocity.y = velocity.y;
    audioContext->defaultListener.Velocity.z = velocity.z;
}
#endif
