#ifndef __SAMPLING_H__
#define __SAMPLING_H__
float TriplanarSample1D(Texture2D tex, float3 texcoord, float3 N) 
{
    float3 blending = abs( N );
    
	// Force weights to sum to 1.0
    blending = normalize( max( blending, 0.00001 ) );
	
	float b = blending.x + blending.y + blending.z;
    blending /= float3( b, b, b );
 
    float x = tex[texcoord.yz].r;
    float y = tex[texcoord.xz].r;
    float z = tex[texcoord.xy].r;
 
    return x * blending.x + y * blending.y + z * blending.z;
}

float3 TriplanarSample3D(Texture2D tex, float3 texcoord, float3 N) 
{
    float3 blending = abs( N );
    
	// Force weights to sum to 1.0
    blending = normalize( max( blending, 0.00001 ) );
	
	float b = blending.x + blending.y + blending.z;
    blending /= float3( b, b, b );
 
    float3 x = tex[texcoord.yz].xyz;
    float3 y = tex[texcoord.xz].xyz;
    float3 z = tex[texcoord.xy].xyz;
 
    return x * blending.x + y * blending.y + z * blending.z;
}

float3 TriplanarSample3D(Texture2DArray tex, sampler sample, uint textureIdx, float3 texcoord, float3 N) 
{
    float3 blending = abs( N );
    
	// Force weights to sum to 1.0
    blending = normalize( max( blending, 0.00001 ) );
	
	float b = blending.x + blending.y + blending.z;
    blending /= float3( b, b, b );
 
    float3 x = tex.Sample( sample, float3( texcoord.yz, textureIdx ) ).xyz;
    float3 y = tex.Sample( sample, float3( texcoord.xz, textureIdx ) ).xyz;
    float3 z = tex.Sample( sample, float3( texcoord.xy, textureIdx ) ).xyz;
 
    return x * blending.x + y * blending.y + z * blending.z;
}
#endif
