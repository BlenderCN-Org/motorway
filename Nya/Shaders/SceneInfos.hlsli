#ifndef __SCENE_INFOS_H__
#define __SCENE_INFOS_H__ 1
cbuffer SceneInfos : register( b1 )
{
	float3	g_ClustersScale;
	float3	g_ClustersInverseScale;
	float3	g_ClustersBias;
	float3	g_SceneAABBMin;
	float3	g_SceneAABBMax;
};
#endif
