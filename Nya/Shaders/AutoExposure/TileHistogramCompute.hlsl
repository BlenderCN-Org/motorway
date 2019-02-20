#include <PhotometricHelpers.hlsli>
#include "SharedAutoExposure.hlsli"

#define NUMTHREADX	1
#define NUMTHREADY	1
#define NUMTHREADZ	1

cbuffer CB_AutoExposure : register( b0 )
{
    float	_delta_time;
    float	_white_level;
    float	_clip_shadows;
    float	_clip_highlights;
    float	_EV;
    float	_fstop_bias;
    float	_reference_camera_fps;
    float	_adapt_min_luminance;
    float	_adapt_max_luminance;
    float	_adapt_speed_up;
    float	_adapt_speed_down;
};

StructuredBuffer<AutoExposureInfos>	    _bufferAutoExposure2 : register( t8 );
RWStructuredBuffer<AutoExposureInfos>	_targetBufferAutoExposure : register( u0 );
Buffer<uint>						    _texHistogram : register( t9 );

[numthreads( NUMTHREADX, NUMTHREADY, NUMTHREADZ )]
void EntryPointCS( uint3 _GroupID : SV_GROUPID, uint3 _GroupThreadID : SV_GROUPTHREADID, uint3 _DispatchThreadID : SV_DISPATCHTHREADID )
{
    AutoExposureInfos	Result;

    AutoExposureInfos	LastFrameResult = _bufferAutoExposure2[0];

    float	MonitorLuminanceRange_dB = Luminance2dB( 255.0 * _white_level );
    float	MonitorBucketsCount = MonitorLuminanceRange_dB / HISTOGRAM_BUCKET_RANGE_DB;

    uint2	ClippedBucketIndex = uint2( _clip_shadows * MonitorBucketsCount, _clip_highlights * MonitorBucketsCount );

    Result.PeakHistogramValue = 0;
    uint	TailBucketPosition = ClippedBucketIndex.x;
    uint	HeadBucketPosition = TailBucketPosition;
    uint	Integral = 0U;
    for ( ; HeadBucketPosition.x < ClippedBucketIndex.y; HeadBucketPosition.x++ ) {
        uint	HistoValue = _texHistogram[HeadBucketPosition].x;
        Integral += HistoValue;
        Result.PeakHistogramValue = max( Result.PeakHistogramValue, HistoValue );
    }

    uint	LastBucketIndex = HISTOGRAM_BUCKETS_COUNT - uint( MonitorBucketsCount - ClippedBucketIndex.y );
    uint	MaxIntegralTailBucketPosition = TailBucketPosition.x;
    uint	MaxIntegral = Integral;

    while ( HeadBucketPosition.x < LastBucketIndex ) {
        uint	HistoValueTail = _texHistogram[TailBucketPosition].x; TailBucketPosition.x++;
        Integral -= HistoValueTail;

        uint	HistoValueHead = _texHistogram[HeadBucketPosition].x; HeadBucketPosition.x++;
        Integral += HistoValueHead;

        if ( Integral >= MaxIntegral ) {
            MaxIntegral = Integral;
            MaxIntegralTailBucketPosition = TailBucketPosition.x;
        }

        Result.PeakHistogramValue = max( Result.PeakHistogramValue, HistoValueHead );
    }
    
    if ( MaxIntegral == 0U ) {
        MaxIntegralTailBucketPosition = ( HISTOGRAM_BUCKETS_COUNT - MonitorBucketsCount ) / 2;
    }

    float	MinAdaptableLuminance_Bucket = Luminance2HistogramBucketIndex( _adapt_min_luminance );
    float	MaxAdaptableLuminance_Bucket = Luminance2HistogramBucketIndex( _adapt_max_luminance );

    float	TargetTailBucketPosition = clamp( MaxIntegralTailBucketPosition, MinAdaptableLuminance_Bucket, MaxAdaptableLuminance_Bucket - MonitorBucketsCount );

    float	TargetLuminance_dB = MIN_ADAPTABLE_SCENE_LUMINANCE_DB + TargetTailBucketPosition * HISTOGRAM_BUCKET_RANGE_DB;
    float	TargetLuminance = dB2Luminance( TargetLuminance_dB );
    TargetLuminance *= exp2( -_EV );
    
    if ( LastFrameResult.TargetLuminance < TargetLuminance ) {
        Result.TargetLuminance = lerp( TargetLuminance, LastFrameResult.TargetLuminance, pow( abs( 1.0 - _adapt_speed_up ), _delta_time ) );
    } else {
        float	InvTargetLuminanceFactor = 1.0 / max( 1e-4, TargetLuminance );
        float	InvLastFrameLuminanceFactor = 1.0 / max( 1e-4, LastFrameResult.TargetLuminance );
        Result.TargetLuminance = 1.0 / lerp( InvTargetLuminanceFactor, InvLastFrameLuminanceFactor, pow( abs( 1.0 - _adapt_speed_down ), _delta_time ) );
    }

    Result.MinLuminanceLDR = Result.TargetLuminance;
    Result.MaxLuminanceLDR = Result.MinLuminanceLDR * _white_level * TARGET_MONITOR_LUMINANCE_RANGE;
    Result.MiddleGreyLuminanceLDR = Result.MinLuminanceLDR * 55.497598410127913869937213614334;
    
    float PowEV = Result.MiddleGreyLuminanceLDR / ABSOLUTE_EV_REFERENCE_LUMINANCE;
    Result.EV = log2( PowEV );
    Result.Fstop = sqrt( PowEV / _reference_camera_fps );
    Result.Fstop += _fstop_bias;
    
    float	WhiteLevelLuminance = Result.MinLuminanceLDR * 1.0 * TARGET_MONITOR_LUMINANCE_RANGE;
    float	WhiteLevelEngineLuminance = WhiteLevelLuminance * WORLD_TO_BISOU_LUMINANCE;
    Result.EngineLuminanceFactor = 1.0 / WhiteLevelEngineLuminance;
    
    _targetBufferAutoExposure[0] = Result;
}
