cbuffer ActiveCameraBuffer : register( b0 )
{
    float4x4	ViewMatrix;
    float4x4	ProjectionMatrix;
    float4x4	InverseViewMatrix;
    float4x4	InverseProjectionMatrix;
    float4x4	ViewProjectionMatrix;
    float4x4	g_InverseViewProjectionMatrix;
    float3		WorldPosition;
    int         g_CameraFrameIndex; // Camera frame index (not renderer frame index!)
    float4x4	DepthProjectionMatrix;
    float4x4	DepthViewProjectionMatrix;
    float4      CascadeOffsets[4];
    float4      CascadeScales[4];
    float4      ShadowSplitDistances;
    float4x4    ShadowMatrices[4];
    float4x4    ShadowMatrixShared;
    float4x4    g_PreviousViewProjectionMatrix;
    float4x4    g_PreviousViewMatrix;
    float2      g_CameraJitteringOffset;
    float2      g_CameraPreviousJitteringOffset;
};
