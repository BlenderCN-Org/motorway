struct VertexBufferData
{
    float4 Position : POSITION;
    float4 Color    : COLOR0;
};

struct VertexStageData
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
};

cbuffer ScreenInfos : register( b0 )
{
    float4x4  g_OrthographicMatrix;
};

VertexStageData EntryPointVS( VertexBufferData VertexBuffer )
{
    float4 Position = mul( float4( VertexBuffer.Position.xyz, 1.0f ), g_OrthographicMatrix );

    VertexStageData output = {
        Position,
        VertexBuffer.Color
    };

    return output;
}

float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{
    return VertexStage.Color;
}
