/*
    Project Motorway Source Code
    Copyright (C) 2018 Prévost Baptiste

    This stream is part of Project Motorway source code.

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

#include "Shared.h"
#include "Material.h"

#include <FileSystem/FileSystemObject.h>

#include <Core/StringHelpers.h>
#include <Io/TextStreamHelpers.h>

#include <Graphics/ShaderCache.h>
#include <Graphics/GraphicsAssetCache.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>
#include <Rendering/ImageFormat.h>

#include <Shaders/MaterialShared.h>

#include <Maths/Helpers.h>

using namespace nya::graphics;
using namespace nya::rendering;

void ReadEditableMaterialInput( const nyaString_t& materialInputLine, const int32_t layerIndex, const uint32_t inputTextureBindIndex, GraphicsAssetCache* graphicsAssetCache, MaterialEditionInput& materialComponent, Texture** textureSet, int& textureCount )
{
    auto valueHashcode = nya::core::CRC32( materialInputLine.c_str() );
    const bool isNone = ( valueHashcode == NYA_STRING_HASH( "None" ) ) || materialInputLine.empty();

    if ( isNone ) {
        materialComponent.InputType = MaterialEditionInput::NONE;
    } else if ( materialInputLine.front() == '{' && materialInputLine.back() == '}' ) {
        materialComponent.InputType = MaterialEditionInput::COLOR_3D;
        materialComponent.Input3D = nya::core::StringTo3DVector( materialInputLine );
    } else if ( materialInputLine.front() == '#' && materialInputLine.length() == 7 ) {
        // Assuming the following hex format: #RRGGBB (no explicit alpha)
        float redChannel = std::stoul( materialInputLine.substr( 1, 2 ).c_str(), nullptr, 16 ) / 255.0f;
        float greenChannel = std::stoul( materialInputLine.substr( 3, 2 ).c_str(), nullptr, 16 ) / 255.0f;
        float blueChannel = std::stoul( materialInputLine.substr( 5, 2 ).c_str(), nullptr, 16 ) / 255.0f;

        materialComponent.InputType = MaterialEditionInput::COLOR_3D;
        materialComponent.Input3D = nyaVec3f( redChannel, greenChannel, blueChannel );
    } else if ( materialInputLine.front() == '"' && materialInputLine.back() == '"' ) {
        auto assetPath = nya::core::WrappedStringToString( materialInputLine );
        auto extension = nya::core::GetFileExtensionFromPath( assetPath );

        materialComponent.InputType = MaterialEditionInput::TEXTURE;
        materialComponent.InputTexture = graphicsAssetCache->getTexture( assetPath.c_str() );

        textureSet[inputTextureBindIndex] = materialComponent.InputTexture;

        textureCount = nya::maths::max( static_cast<int32_t>( inputTextureBindIndex ), textureCount ) + 1;
    } else {
        materialComponent.InputType = MaterialEditionInput::COLOR_1D;
        materialComponent.Input1D = std::stof( materialInputLine );
    }
}

Material::Material( const nyaString_t& materialName )
    : name( materialName )
    , builderVersion( 0 )
    , stageCustomFlagset( "" )
    , defaultPipelineState( nullptr )
    , probeCapturePipelineState( nullptr )
    , defaultTextureSet{ nullptr }
    , defaultTextureSetCount( 0 )
    , depthOnlyPipelineState( nullptr )
    , depthOnlyTextureSet{ nullptr }
    , depthOnlyTextureSetCount( 0 )
    , sortKey( 0u )
{
    sortKeyInfos.shadingModel = eShadingModel::SHADING_MODEL_NONE;
}

Material::~Material()
{
    name.clear();
    stageCustomFlagset.clear();
    defaultPipelineState = nullptr;
    probeCapturePipelineState = nullptr;
    depthOnlyPipelineState = nullptr;
    sortKey = 0u;
}

void GetShaderStage( const eShadingModel shadingModel, std::string& standardVertexStage, std::string& standardPixelStage, std::string& probePixelStage, std::string& depthVertexStage, std::string& depthPixelStage )
{
    switch ( shadingModel ) {
    case eShadingModel::SHADING_MODEL_STANDARD:
        standardVertexStage = "Lighting/Ubersurface";
        standardPixelStage = "Lighting/Ubersurface+NYA_EDITOR+NYA_BRDF_STANDARD";
        probePixelStage = "Lighting/Ubersurface+NYA_EDITOR+NYA_BRDF_STANDARD+NYA_PROBE_CAPTURE+NYA_ENCODE_RGBD";
        depthVertexStage = "Lighting/UberDepthOnly";
        depthPixelStage = "Lighting/UberDepthOnly";
        break;

    case eShadingModel::SHADING_MODEL_CLEAR_COAT:
        standardVertexStage = "Lighting/Ubersurface";
        standardPixelStage = "Lighting/Ubersurface+NYA_EDITOR+NYA_BRDF_CLEAR_COAT";
        probePixelStage = "Lighting/Ubersurface+NYA_EDITOR+NYA_BRDF_CLEAR_COAT+NYA_PROBE_CAPTURE+NYA_ENCODE_RGBD";
        depthVertexStage = "Lighting/UberDepthOnly";
        depthPixelStage = "Lighting/UberDepthOnly";
        break;

    case eShadingModel::SHADING_MODEL_HUD_STANDARD:
        standardVertexStage = "HUD/Primitive";
        standardPixelStage = "HUD/Primitive";
        probePixelStage = "Error";
        depthVertexStage = "Lighting/UberDepthOnly";
        depthPixelStage = "Lighting/UberDepthOnly";
        break;

    case eShadingModel::SHADING_MODEL_NONE:
    default:
        standardVertexStage = "Lighting/Ubersurface";
        standardPixelStage = "Lighting/Ubersurface";
        probePixelStage = "Lighting/Ubersurface+NYA_PROBE_CAPTURE+NYA_ENCODE_RGBD";
        depthVertexStage = "Lighting/UberDepthOnly";
        depthPixelStage = "Lighting/UberDepthOnly";
        break;
    }
}

void Material::create( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    std::string compiledVertexStage, compiledPixelStage, compiledProbePixelStage, compiledDepthVertexStage, compiledDepthPixelStage;
    GetShaderStage( sortKeyInfos.shadingModel, compiledVertexStage, compiledPixelStage, compiledProbePixelStage, compiledDepthVertexStage, compiledDepthPixelStage );

    if ( !stageCustomFlagset.empty() ) {
        compiledPixelStage.append( stageCustomFlagset );
    } else {
        if ( sortKeyInfos.scaleUVByModelScale ) {
            compiledVertexStage.append( "+NYA_SCALE_UV_BY_MODEL_SCALE" );
            compiledDepthVertexStage.append( "+NYA_SCALE_UV_BY_MODEL_SCALE" );
        }

       /* if ( sortKeyInfos.useLodAlphaBlending ) {
            compiledPixelStage.append( "+NYA_USE_LOD_ALPHA_BLENDING" );
        }*/

        if ( editableMaterialData.ReceiveShadow ) {
            compiledPixelStage.append( "+NYA_RECEIVE_SHADOW" );
        }

        if ( editableMaterialData.CastShadow ) {
            compiledPixelStage.append( "+NYA_CAST_SHADOW" );
        }
    }

    // Default PSO
    PipelineStateDesc defaultPipelineStateDesc = {};

    defaultPipelineStateDesc.vertexShader = shaderCache->getOrUploadStage( compiledVertexStage, eShaderStage::SHADER_STAGE_VERTEX );
    defaultPipelineStateDesc.pixelShader = shaderCache->getOrUploadStage( compiledPixelStage, eShaderStage::SHADER_STAGE_PIXEL );
    defaultPipelineStateDesc.primitiveTopology = ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    defaultPipelineStateDesc.rasterizerState.fillMode = eFillMode::FILL_MODE_SOLID;
    defaultPipelineStateDesc.rasterizerState.cullMode = ( editableMaterialData.IsDoubleFace ) ? eCullMode::CULL_MODE_NONE : eCullMode::CULL_MODE_FRONT;
    defaultPipelineStateDesc.rasterizerState.useTriangleCCW = true;
    defaultPipelineStateDesc.depthStencilState.enableDepthTest = true;
    defaultPipelineStateDesc.depthStencilState.enableDepthWrite = true;
    defaultPipelineStateDesc.depthStencilState.depthComparisonFunc = eComparisonFunction::COMPARISON_FUNCTION_GREATER;
    defaultPipelineStateDesc.inputLayout[0] = { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, false, "POSITION" };
    defaultPipelineStateDesc.inputLayout[1] = { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, true, "NORMAL" };
    defaultPipelineStateDesc.inputLayout[2] = { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0, 0, true, "TEXCOORD" };
     
    if ( editableMaterialData.EnableAlphaBlend || editableMaterialData.AlphaToCoverage ) {
        defaultPipelineStateDesc.blendState.blendConfColor.operation = eBlendOperation::BLEND_OPERATION_ADD;
        defaultPipelineStateDesc.blendState.blendConfColor.source = eBlendSource::BLEND_SOURCE_SRC_ALPHA;
        defaultPipelineStateDesc.blendState.blendConfColor.dest = eBlendSource::BLEND_SOURCE_INV_SRC_ALPHA;

        defaultPipelineStateDesc.blendState.blendConfAlpha.operation = eBlendOperation::BLEND_OPERATION_ADD;
        defaultPipelineStateDesc.blendState.blendConfAlpha.source = eBlendSource::BLEND_SOURCE_INV_DEST_ALPHA;
        defaultPipelineStateDesc.blendState.blendConfAlpha.dest = eBlendSource::BLEND_SOURCE_ONE;

        defaultPipelineStateDesc.blendState.enableBlend = editableMaterialData.EnableAlphaBlend;
        defaultPipelineStateDesc.blendState.sampleMask = ~0u;
        defaultPipelineStateDesc.blendState.enableAlphaToCoverage = editableMaterialData.AlphaToCoverage;
    }

    defaultPipelineStateDesc.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    defaultPipelineStateDesc.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::WRITE;
    defaultPipelineStateDesc.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::DONT_CARE;
    defaultPipelineStateDesc.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

    if ( sortKeyInfos.shadingModel != eShadingModel::SHADING_MODEL_HUD_STANDARD ) {
        defaultPipelineStateDesc.renderPassLayout.attachements[1].stageBind = SHADER_STAGE_PIXEL;
        defaultPipelineStateDesc.renderPassLayout.attachements[1].bindMode = RenderPassLayoutDesc::WRITE;
        defaultPipelineStateDesc.renderPassLayout.attachements[1].targetState = RenderPassLayoutDesc::DONT_CARE;
        defaultPipelineStateDesc.renderPassLayout.attachements[1].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16_FLOAT;

        defaultPipelineStateDesc.renderPassLayout.attachements[2].stageBind = SHADER_STAGE_PIXEL;
        defaultPipelineStateDesc.renderPassLayout.attachements[2].bindMode = RenderPassLayoutDesc::WRITE;
        defaultPipelineStateDesc.renderPassLayout.attachements[2].targetState = RenderPassLayoutDesc::DONT_CARE;
        defaultPipelineStateDesc.renderPassLayout.attachements[2].viewFormat = eImageFormat::IMAGE_FORMAT_R11G11B10_FLOAT;

        defaultPipelineStateDesc.renderPassLayout.attachements[3].stageBind = SHADER_STAGE_PIXEL;
        defaultPipelineStateDesc.renderPassLayout.attachements[3].bindMode = RenderPassLayoutDesc::WRITE_DEPTH;
        defaultPipelineStateDesc.renderPassLayout.attachements[3].targetState = RenderPassLayoutDesc::DONT_CARE;
        defaultPipelineStateDesc.renderPassLayout.attachements[3].viewFormat = eImageFormat::IMAGE_FORMAT_R32_TYPELESS;

        defaultPipelineStateDesc.renderPassLayout.attachements[4].stageBind = SHADER_STAGE_PIXEL;
        defaultPipelineStateDesc.renderPassLayout.attachements[4].bindMode = RenderPassLayoutDesc::READ;
        defaultPipelineStateDesc.renderPassLayout.attachements[4].targetState = RenderPassLayoutDesc::DONT_CARE;
        defaultPipelineStateDesc.renderPassLayout.attachements[4].viewFormat = eImageFormat::IMAGE_FORMAT_R32_TYPELESS;

        defaultPipelineStateDesc.renderPassLayout.attachements[5].stageBind = SHADER_STAGE_PIXEL;
        defaultPipelineStateDesc.renderPassLayout.attachements[5].bindMode = RenderPassLayoutDesc::READ;
        defaultPipelineStateDesc.renderPassLayout.attachements[5].targetState = RenderPassLayoutDesc::DONT_CARE;
        defaultPipelineStateDesc.renderPassLayout.attachements[5].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

        defaultPipelineStateDesc.renderPassLayout.attachements[6].stageBind = SHADER_STAGE_PIXEL;
        defaultPipelineStateDesc.renderPassLayout.attachements[6].bindMode = RenderPassLayoutDesc::READ;
        defaultPipelineStateDesc.renderPassLayout.attachements[6].targetState = RenderPassLayoutDesc::DONT_CARE;
        defaultPipelineStateDesc.renderPassLayout.attachements[6].viewFormat = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

        defaultPipelineStateDesc.resourceListLayout.resources[0] = { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
        defaultPipelineStateDesc.resourceListLayout.resources[1] = { 1, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
        defaultPipelineStateDesc.resourceListLayout.resources[2] = { 2, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
        defaultPipelineStateDesc.resourceListLayout.resources[3] = { 0, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
        defaultPipelineStateDesc.resourceListLayout.resources[4] = { 1, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
        defaultPipelineStateDesc.resourceListLayout.resources[5] = { 1, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
        defaultPipelineStateDesc.resourceListLayout.resources[6] = { 2, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
        defaultPipelineStateDesc.resourceListLayout.resources[7] = { 8, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_GENERIC_BUFFER };
        defaultPipelineStateDesc.resourceListLayout.resources[8] = { 9, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_GENERIC_BUFFER };

#if NYA_DEVBUILD
        defaultPipelineStateDesc.resourceListLayout.resources[9] = { 3, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
#endif

        defaultPipelineStateDesc.resourceListLayout.resources[10] = { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_UAV_TEXTURE };
    } else {
        defaultPipelineStateDesc.resourceListLayout.resources[0] = { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
        defaultPipelineStateDesc.resourceListLayout.resources[1] = { 0, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
        defaultPipelineStateDesc.resourceListLayout.resources[2] = { 1, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
        defaultPipelineStateDesc.resourceListLayout.resources[3] = { 8, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_GENERIC_BUFFER };
    }

    uint32_t textureBindIndex = 11u;
    for ( int32_t textureIdx = 0; textureIdx < defaultTextureSetCount; textureIdx++ ) {
        defaultPipelineStateDesc.resourceListLayout.resources[textureBindIndex] = { textureIdx + 4, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE };
        textureBindIndex++;
    }

    defaultPipelineState = renderDevice->createPipelineState( defaultPipelineStateDesc );

    // Probe Capture PSO
    defaultPipelineStateDesc.pixelShader = shaderCache->getOrUploadStage( compiledProbePixelStage, eShaderStage::SHADER_STAGE_PIXEL );
    probeCapturePipelineState = renderDevice->createPipelineState( defaultPipelineStateDesc );

    // Depth only PSO
    PipelineStateDesc depthPipelineStateDesc = {};

    depthPipelineStateDesc.vertexShader = shaderCache->getOrUploadStage( compiledDepthVertexStage, eShaderStage::SHADER_STAGE_VERTEX );
    depthPipelineStateDesc.pixelShader = shaderCache->getOrUploadStage( compiledDepthPixelStage, eShaderStage::SHADER_STAGE_PIXEL );
    depthPipelineStateDesc.primitiveTopology = ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    depthPipelineStateDesc.rasterizerState.fillMode = eFillMode::FILL_MODE_SOLID;
    depthPipelineStateDesc.rasterizerState.cullMode = ( sortKeyInfos.isDoubleFace ) ? eCullMode::CULL_MODE_NONE : eCullMode::CULL_MODE_FRONT;
    depthPipelineStateDesc.rasterizerState.useTriangleCCW = true;
    depthPipelineStateDesc.depthStencilState.enableDepthTest = true;
    depthPipelineStateDesc.depthStencilState.enableDepthWrite = true;
    depthPipelineStateDesc.depthStencilState.depthComparisonFunc = eComparisonFunction::COMPARISON_FUNCTION_LESS;
    depthPipelineStateDesc.inputLayout[0] = { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, false, "POSITION" };
    depthPipelineStateDesc.inputLayout[1] = { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, true, "NORMAL" };
    depthPipelineStateDesc.inputLayout[2] = { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0, 0, true, "TEXCOORD" };

    depthPipelineStateDesc.renderPassLayout.attachements[0].stageBind = SHADER_STAGE_PIXEL;
    depthPipelineStateDesc.renderPassLayout.attachements[0].bindMode = RenderPassLayoutDesc::WRITE_DEPTH;
    depthPipelineStateDesc.renderPassLayout.attachements[0].targetState = RenderPassLayoutDesc::DONT_CARE;
    depthPipelineStateDesc.renderPassLayout.attachements[0].viewFormat = eImageFormat::IMAGE_FORMAT_R32_TYPELESS;

    depthPipelineStateDesc.resourceListLayout.resources[0] = { 0, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_SAMPLER };
    depthPipelineStateDesc.resourceListLayout.resources[1] = { 1, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
    depthPipelineStateDesc.resourceListLayout.resources[2] = { 8, SHADER_STAGE_VERTEX, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_GENERIC_BUFFER };

#if NYA_DEVBUILD
    depthPipelineStateDesc.resourceListLayout.resources[3] = { 3, SHADER_STAGE_VERTEX | SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_CBUFFER };
#endif

    uint32_t depthTextureBindIndex = 4u;
    for ( int32_t textureIdx = 0; textureIdx < depthOnlyTextureSetCount; textureIdx++ ) {
        depthPipelineStateDesc.resourceListLayout.resources[depthTextureBindIndex] = { textureIdx, SHADER_STAGE_PIXEL, ResourceListLayoutDesc::RESOURCE_LIST_RESOURCE_TYPE_TEXTURE };
        depthTextureBindIndex++;
    }

    depthOnlyPipelineState = renderDevice->createPipelineState( depthPipelineStateDesc );
}

void Material::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( defaultPipelineState );
    renderDevice->destroyPipelineState( probeCapturePipelineState );
    renderDevice->destroyPipelineState( depthOnlyPipelineState );
}

void Material::load( FileSystemObject* stream, GraphicsAssetCache* graphicsAssetCache )
{
#define NYA_CASE_READ_MATERIAL_FLAG( streamLine, variable ) case NYA_STRING_HASH( #variable ): editableMaterialData.variable = nya::core::StringToBoolean( streamLine ); break;
#define NYA_CASE_READ_MATERIAL_FLOAT( streamLine, layerIndex, variable ) case NYA_STRING_HASH( #variable ): editableMaterialData.layers[layerIndex].variable = std::stof( dictionaryValue.c_str() ); break;
#define NYA_CASE_READ_LAYER_PIXEL_INPUT( streamLine, layerIndex, variableIndex, variable )  case NYA_STRING_HASH( #variable ): ReadEditableMaterialInput( streamLine, layerIndex, variableIndex, graphicsAssetCache, editableMaterialData.layers[layerIndex].variable, defaultTextureSet, defaultTextureSetCount ); break;
#define NYA_CASE_READ_LAYER_VERTEX_INPUT( streamLine, layerIndex, variableIndex, variable )  case NYA_STRING_HASH( #variable ): ReadEditableMaterialInput( streamLine, layerIndex, variableIndex, graphicsAssetCache, editableMaterialData.layers[layerIndex].variable, vertexTextureSet, defaultTextureSetCount ); break;

    // Reset material inputs (incase of hot reloading)
    defaultTextureSetCount = 0;

    nyaString_t streamLine, dictionaryKey, dictionaryValue;

    int currentLayerIndex = -1;
    uint32_t slotBaseIndex = 0;
    while ( stream->isGood() ) {
        nya::core::ReadString( stream, streamLine );

        // Find seperator character offset in the line (if any)
        const auto commentSeparator = streamLine.find_first_of( NYA_STRING( ";" ) );

        // Remove user comments before reading the keypair value
        if ( commentSeparator != nyaString_t::npos ) {
            streamLine.erase( streamLine.begin() + static_cast<long>( commentSeparator ), streamLine.end() );
        }

        // Skip commented out and empty lines
        if ( streamLine.empty() ) {
            continue;
        }

        const auto keyValueSeparator = streamLine.find_first_of( ':' );

        // Check if this is a key value line
        if ( keyValueSeparator != nyaString_t::npos ) {
            dictionaryKey = streamLine.substr( 0, keyValueSeparator );
            dictionaryValue = streamLine.substr( keyValueSeparator + 1 );

            // Trim both key and values (useful if a stream has inconsistent spacing, ...)
            nya::core::TrimString( dictionaryKey );
            nya::core::TrimString( dictionaryValue );

            // Do the check after triming, since the value might be a space or a tab character
            if ( !dictionaryValue.empty() ) {
                auto keyHashcode = nya::core::CRC32( dictionaryKey.c_str() );

                switch ( keyHashcode ) {
                case NYA_STRING_HASH( "Name" ):
                    name = nya::core::WrappedStringToString( dictionaryValue );
                    break;

                case NYA_STRING_HASH( "ShadingModel" ):
                    sortKeyInfos.shadingModel = nya::graphics::StringToShadingModel( nya::core::CRC32( dictionaryValue ) );
                    getShadingModelResources( graphicsAssetCache );
                    break;

                case NYA_STRING_HASH( "Version" ):
                    builderVersion = std::stoi( dictionaryValue );
                    break;

                case NYA_STRING_HASH( "ScaleUVByModelScale" ):
                    sortKeyInfos.scaleUVByModelScale = nya::core::StringToBoolean( dictionaryValue );
                    break;

                case NYA_STRING_HASH( "UseBaseColorsRGBSource" ):
                    editableMaterialData.layers[currentLayerIndex].BaseColor.SamplingFlags = nya::core::StringToBoolean( dictionaryValue )
                        ? MaterialEditionInput::SRGB_SOURCE
                        : MaterialEditionInput::LINEAR_SOURCE;
                    break;

                case NYA_STRING_HASH( "UseAlphaRoughnessSource" ):
                    editableMaterialData.layers[currentLayerIndex].Roughness.SamplingFlags = nya::core::StringToBoolean( dictionaryValue )
                        ? MaterialEditionInput::ALPHA_ROUGHNESS_SOURCE
                        : MaterialEditionInput::ROUGHNESS_SOURCE;
                    break;

                case NYA_STRING_HASH( "IsNormalMapTangentSpace" ):
                    editableMaterialData.layers[currentLayerIndex].Normal.SamplingFlags = nya::core::StringToBoolean( dictionaryValue )
                        ? MaterialEditionInput::TANGENT_SPACE_SOURCE
                        : MaterialEditionInput::WORLD_SPACE_SOURCE;
                    break;

                case NYA_STRING_HASH( "IsSecondaryNormalMapTangentSpace" ):
                    editableMaterialData.layers[currentLayerIndex].SecondaryNormal.SamplingFlags = nya::core::StringToBoolean( dictionaryValue )
                        ? MaterialEditionInput::TANGENT_SPACE_SOURCE
                        : MaterialEditionInput::WORLD_SPACE_SOURCE;
                    break;

                // Material Flags
                NYA_CASE_READ_MATERIAL_FLAG( dictionaryValue, WriteVelocity )
                NYA_CASE_READ_MATERIAL_FLAG( dictionaryValue, EnableAlphaTest )
                NYA_CASE_READ_MATERIAL_FLAG( dictionaryValue, IsDoubleFace )
                NYA_CASE_READ_MATERIAL_FLAG( dictionaryValue, EnableAlphaBlend )
                NYA_CASE_READ_MATERIAL_FLAG( dictionaryValue, CastShadow )
                NYA_CASE_READ_MATERIAL_FLAG( dictionaryValue, ReceiveShadow )
                NYA_CASE_READ_MATERIAL_FLAG( dictionaryValue, AlphaToCoverage )

                // Shading Model Inputs
                //NYA_CASE_READ_LAYER_VERTEX_INPUT( dictionaryValue, currentLayerIndex, 0, Heightmap )
                //NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, slotBaseIndex, TerrainSplatMap )
                //NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 1 ), TerrainGrassMap )

                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, slotBaseIndex, BaseColor )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 1 ), AlphaMask )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 2 ), Reflectance )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 3 ), Roughness )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 4 ), Metalness )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 5 ), AmbientOcclusion )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 6 ), Normal )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 7 ), Emissivity )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 8 ), Displacement )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 9 ), SecondaryNormal )
                NYA_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 10 ), BlendMask )

                // Misc. / Blending Inputs
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, Refraction )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, RefractionIor )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, ClearCoat )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, ClearCoatGlossiness )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, DiffuseContribution )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, SpecularContribution )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, NormalContribution )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, AlphaCutoff )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, NormalMapStrength )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, SecondaryNormalMapStrength )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, DisplacementMapStrength )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, HeightmapWorldHeight )
                NYA_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, SSStrength )

                // Layer Scaling
                case NYA_STRING_HASH( "Offset" ):
                    editableMaterialData.layers[currentLayerIndex].LayerOffset = nya::core::StringTo2DVector( dictionaryValue );
                    break;

                case NYA_STRING_HASH( "Scale" ):
                    editableMaterialData.layers[currentLayerIndex].LayerScale = nya::core::StringTo2DVector( dictionaryValue );
                    break;
                }
            }
        } else {
            dictionaryKey = streamLine.substr( 0, keyValueSeparator );

            // Trim both key and values (useful if a stream has inconsistent spacing, ...)
            nya::core::TrimString( dictionaryKey );

            auto keyHashcode = nya::core::CRC32( dictionaryKey.c_str() );

            switch ( keyHashcode ) {
            case NYA_STRING_HASH( "Layer" ):
                if ( currentLayerIndex < MAX_LAYER_COUNT ) {
                    currentLayerIndex++;
                    slotBaseIndex = static_cast<uint32_t>( 1 + ( 10 * currentLayerIndex ) + currentLayerIndex );
                }
                break;
            }
        }
    }

    editableMaterialData.LayerCount = static_cast<uint32_t>( currentLayerIndex + 1 );

    stream->close();
}

uint32_t Material::getSortKey() const
{
    return sortKey;
}

bool Material::isOpaque() const
{
    return !sortKeyInfos.useTranslucidity;
}

nyaStringHash_t Material::getHashcode() const
{
    return nya::core::CRC32( name );
}

void Material::setName( const nyaString_t& meshName )
{
    name = meshName;
}

const nyaString_t& Material::getName() const
{
    return name;
}

void Material::bind( CommandList& cmdList, RenderPass& renderPass, ResourceList& resourceList ) const
{
    cmdList.bindRenderPass( defaultPipelineState, renderPass );
    cmdList.bindPipelineState( defaultPipelineState );
    bindDefaultTextureSet( resourceList );

   // cmdList->bindResourceList( defaultPipelineState, resourceList );
}

void Material::bindProbeCapture( CommandList& cmdList, RenderPass& renderPass, ResourceList& resourceList ) const
{
    cmdList.bindRenderPass( probeCapturePipelineState, renderPass );
    cmdList.bindPipelineState( probeCapturePipelineState );

    bindDefaultTextureSet( resourceList );

    //cmdList-bindResourceList( probeCapturePipelineState, resourceList );
}

void Material::bindDepthOnly( CommandList& cmdList, RenderPass& renderPass, ResourceList& resourceList ) const
{
    cmdList.bindRenderPass( depthOnlyPipelineState, renderPass );
    cmdList.bindPipelineState( depthOnlyPipelineState );

    // 0 -> Output RenderTarget
    int32_t textureBindIndex = 4;

    for ( int32_t textureIdx = 0; textureIdx < depthOnlyTextureSetCount; textureIdx++ ) {
        resourceList.resource[textureBindIndex].texture = depthOnlyTextureSet[textureIdx];
        textureBindIndex++;
    }

    //cmdList->bindResourceList( depthOnlyPipelineState, resourceList );
}

void Material::setCustomFlagset( const std::string& customFlagset )
{
    stageCustomFlagset = customFlagset;
}

#if NYA_DEVBUILD
const Material::EditorBuffer& Material::getEditorBuffer() const
{
    return editableMaterialData;
}
#endif

void Material::bindDefaultTextureSet( ResourceList& resourceList ) const
{
    // 0..3 -> Output RenderTargets
    // 4 -> Clusters (3D Tex)
    // 5 -> Sun ShadowMap
    // 6..8 -> IBL Probes (Diffuse/Specular/Captured (HDR))
    int32_t textureBindIndex = 11;

    for ( int32_t textureIdx = 0; textureIdx < defaultTextureSetCount; textureIdx++ ) {
        resourceList.resource[textureBindIndex].texture = defaultTextureSet[textureIdx];
        textureBindIndex++;
    }
}

void Material::getShadingModelResources( GraphicsAssetCache* graphicsAssetCache )
{
    switch ( sortKeyInfos.shadingModel ) {
    case eShadingModel::SHADING_MODEL_CLEAR_COAT:
    case eShadingModel::SHADING_MODEL_STANDARD: {
        // Load BRDF DFG LUT
        defaultTextureSet[0] = graphicsAssetCache->getTexture( NYA_STRING( "GameData/textures/DFG_LUT_Standard.dds" ) );
        defaultTextureSetCount++;
    } break;

    default:
        break;
    }
}
