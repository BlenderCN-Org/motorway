struct VertexStageData
{
	float4	Position : SV_POSITION;
	float2	TexCoordinates : TEXCOORD;
};

Texture2D 	g_InputTexture : register( t0 );
sampler		g_LinearSampler : register( s0 );

float4 EntryPointPS( VertexStageData VertexStage ) : SV_TARGET0
{
    return g_InputTexture.Sample( g_LinearSampler, VertexStage.TexCoordinates );
}
