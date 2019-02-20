#ifndef __CAMERA_DATA_H__
#define __CAMERA_DATA_H__ 1
cbuffer ActiveCameraBuffer : register( b0 )
{
    float4x4	g_ViewMatrix;
    float4x4	g_ProjectionMatrix;
    float4x4	g_InverseViewMatrix;
    float4x4	g_InverseProjectionMatrix;
    float4x4	g_ViewProjectionMatrix;
    float4x4	g_InverseViewProjectionMatrix;
    float3		g_WorldPosition;
    int         g_CameraFrameIndex; // Camera frame index (not renderer frame index!)
    float2      g_ScreenSize;
    float2      g_InverseScreenSize;
    float4x4	g_DepthProjectionMatrix;
    float4x4	g_DepthViewProjectionMatrix;
    float4      g_CascadeOffsets[4];
    float4      g_CascadeScales[4];
    float4      g_ShadowSplitDistances;
    float4x4    g_ShadowMatrices[4];
    float4x4    g_ShadowMatrixShared;
    float4x4    g_PreviousViewProjectionMatrix;
    float4x4    g_PreviousViewMatrix;
    float2      g_CameraJitteringOffset;
    float2      g_CameraPreviousJitteringOffset;
    float4      g_CameraFrustumPlanes[6];
};
#endif
