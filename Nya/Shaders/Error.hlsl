struct VertexStageData
{
	float4	Position : SV_POSITION;
	float2	TexCoordinates : TEXCOORD;
};

VertexStageData EntryPointVS( uint VertexID : SV_VERTEXID )
{
    VertexStageData output;

    output.TexCoordinates = float2( ( VertexID << 1 ) & 2, VertexID & 2 );
    output.Position = float4( output.TexCoordinates * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f ), 0.0f, 1.0f );

    return output;
}

float4 EntryPointPS() : SV_TARGET0
{
    return float4( 1, 0.00, 1, 1.00 );
}

[numthreads( 1, 1, 1 )]
void EntryPointCS()
{

}
