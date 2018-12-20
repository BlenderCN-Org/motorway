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

void UpdateSoundSpatialization( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{
    X3DAudioCalculate( audioContext->x3dAudioHandle, &audioContext->defaultListener, &audioSource->emitter,
        X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
        &audioContext->dspSettings );

    audioSource->sourceVoice->SetOutputMatrix( audioContext->masteringVoice, 1, audioContext->details.InputChannels, audioContext->dspSettings.pMatrixCoefficients );

    flan::audio::SetSourcePitchImpl( audioContext, audioSource, audioContext->dspSettings.DopplerFactor );

    XAUDIO2_FILTER_PARAMETERS FilterParameters = { LowPassFilter, 2.0f * sinf( X3DAUDIO_PI / 6.0f * audioContext->dspSettings.LPFDirectCoefficient ), 1.0f };
    audioSource->sourceVoice->SetFilterParameters( &FilterParameters );
}

NativeAudioSource* flan::audio::CreateAudioSourceImpl( NativeAudioContext* audioContext, BaseAllocator* allocator )
{
    // NOTE Do not allocate until buffer submit (source relies on buffer infos)
    NativeAudioSource* audioSource = flan::core::allocate<NativeAudioSource>( allocator );
    audioSource->sourceVoice = nullptr;
    audioSource->emitter = { 0 };
    audioSource->emitter.ChannelCount = 1;
    audioSource->emitter.CurveDistanceScaler = std::numeric_limits<float>::max();

    return audioSource;
}

void flan::audio::DestroyAudioSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{
    if ( audioSource->sourceVoice != nullptr )
        audioSource->sourceVoice->DestroyVoice();
}

void flan::audio::SetSourcePitchImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float pitch )
{
    audioSource->sourceVoice->SetFrequencyRatio( pitch );
}

void flan::audio::SetSourceGainImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const float gain )
{
    audioSource->sourceVoice->SetVolume( gain );
}

void flan::audio::SetSourcePositionImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& position )
{
    audioSource->emitter.Position = X3DAUDIO_VECTOR( position.x, position.y, position.z );

    UpdateSoundSpatialization( audioContext, audioSource );
}

void flan::audio::SetSourceVelocityImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const glm::vec3& velocity )
{
    audioSource->emitter.Velocity = X3DAUDIO_VECTOR( velocity.x, velocity.y, velocity.z );

    UpdateSoundSpatialization( audioContext, audioSource );
}

void flan::audio::SetSourceLoopingImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, const bool isLooping )
{
    // TODO Looping is defined on the buffer side...
}

void flan::audio::BindBufferToSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource, NativeAudioBuffer* audioBuffer )
{    
    auto hr = S_OK;

    WAVEFORMATEXTENSIBLE wfx = { 0 };
    wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wfx.Format.cbSize = 22;
    wfx.Format.nChannels = 2;
    wfx.Format.nSamplesPerSec = 32000;
    wfx.Format.nBlockAlign = 4;
    wfx.Format.nAvgBytesPerSec = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
    wfx.Format.wBitsPerSample = 16;
    wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    wfx.Samples.wValidBitsPerSample = 16;
    wfx.dwChannelMask = ( SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT );

    if ( FAILED( hr = audioContext->engineInstance->CreateSourceVoice( &audioSource->sourceVoice, reinterpret_cast< WAVEFORMATEX* >( &wfx ) ) ) ) {
        FLAN_CERR << "Failed to create audio source (error code: " << hr << ")" << std::endl;
    }

    if ( FAILED( hr = audioSource->sourceVoice->SubmitSourceBuffer( &audioBuffer->nativeBuffer ) ) ) {
        FLAN_CERR << "Failed to submit audio buffer (error code: " << hr << ")" << std::endl;
    }
}

void flan::audio::PlaySourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{
    audioSource->sourceVoice->Start();
}

void flan::audio::StopSourceImpl( NativeAudioContext* audioContext, NativeAudioSource* audioSource )
{
    audioSource->sourceVoice->Stop();
}
#endif