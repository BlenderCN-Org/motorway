#ifndef __GRASS_INSTANCE_H__
#define __GRASS_INSTANCE_H__ 1
struct Instance
{
	float3	position;
	float 	specular;
	float3	albedo;
	uint	vertexOffsetAndSkew;
	float2	rotation;
	float2	scale;
};
#endif
