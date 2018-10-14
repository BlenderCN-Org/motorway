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

#ifdef __cplusplus
#pragma once
#define uint uint32_t
#define float2 glm::vec2

struct MaterialEditionInput
{
    glm::vec3 Input3D;
    float     Input1D;

    enum : int
    {
        NONE = 0,
        COLOR_1D,
        COLOR_3D,
        TEXTURE,

        EDITABLE_MATERIAL_COMPONENT_TYPE_COUNT
    } InputType;

    enum : uint32_t
    {
        // BaseColor Flags
        SRGB_SOURCE = 0,
        LINEAR_SOURCE = 1,

        // Roughness Flags
        ROUGHNESS_SOURCE = 0,
        ALPHA_ROUGHNESS_SOURCE = 1,

        // Normal Flags
        TANGENT_SPACE_SOURCE = 0,
        WORLD_SPACE_SOURCE = 1,
    } SamplingFlags;

    Texture* InputTexture;
};
#else
struct MaterialEditionInput
{
    // Input can have 4 states:
    //      0: none
    //      1: constant 1d value
    //      2: constant 3d vector    
    //      3: texture input
    float3  Input3D;
    float   Input1D;
    
    int     Type;
    uint    SamplingMode;
    float2  EXPLICIT_PADDING; // Holds Texture pointer on CPU side
};
#endif

#define INPUT_TYPE_NONE 0
#define INPUT_TYPE_1D 1
#define INPUT_TYPE_3D 2
#define INPUT_TYPE_TEXTURE 3

#define SAMPLING_MODE_SRGB 0
#define SAMPLING_MODE_LINEAR 1

#define SAMPLING_MODE_ALPHA_ROUGHNESS 0
#define SAMPLING_MODE_ROUGHNESS 1

#define SAMPLING_MODE_TANGENT_SPACE 0
#define SAMPLING_MODE_WORLD_SPACE 1

static const int MAX_LAYER_COUNT = 3;
    
struct MaterialLayer 
{
    MaterialEditionInput   BaseColor;
    MaterialEditionInput   Reflectance;
    MaterialEditionInput   Roughness;   
    MaterialEditionInput   Metalness;
    MaterialEditionInput   AmbientOcclusion;
    MaterialEditionInput   Normal;  
    MaterialEditionInput   Emissivity;
    MaterialEditionInput   AlphaMask;
    MaterialEditionInput   Displacement;    
    MaterialEditionInput   SecondaryNormal;
    MaterialEditionInput   BlendMask;
    MaterialEditionInput   Heightmap;
    MaterialEditionInput   HeightmapNormal;
    MaterialEditionInput   HeightmapDisplacement;
    MaterialEditionInput   TerrainSplatMap;

    float                  Refraction;
    float                  RefractionIor;
    float                  ClearCoat;
    float                  ClearCoatGlossiness;

    float                  NormalMapStrength;
    float                  SecondaryNormalMapStrength;
    float                  DisplacementMapStrength;
    float                  HeightmapWorldHeight; // Heightmap height (in world space units)

    float                  DiffuseContribution;
    float                  SpecularContribution;
    float                  NormalContribution;
    float                  AlphaCutoff;

    float2                 LayerOffset;
    float2                 LayerScale;
};
