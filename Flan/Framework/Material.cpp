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
#include <Shared.h>
#include "Material.h"

#include <Core/FileSystemIOHelpers.h>
#include <Core/LocaleHelpers.h>
#include <Core/PathHelpers.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/ShaderStageManager.h>
#include <Graphics/WorldRenderer.h>
#include <Graphics/CBufferIndexes.h>
#include <Graphics/TextureSlotIndexes.h>
#include <FileSystem/FileSystemObjectNative.h>
#include <Io/TextStreamHelpers.h>
#include <Rendering/Buffer.h>
#include <Rendering/Texture.h>
#include <Rendering/DepthStencilState.h>
#include <Rendering/PipelineState.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#if FLAN_D3D11
#include <Rendering/Direct3D11/Texture.h>
#elif FLAN_VULKAN
#include <Rendering/Vulkan/Texture.h>
#endif

Material::Material( const fnString_t& materialName )
    : name( materialName )
    , isEditable( true )
    , scaleUVByModelScale( false )
    , builderVersion( 1 )
    , sortKey( 0 )
    , shadingModel( flan::graphics::eShadingModel::SHADING_MODEL_STANDARD )
    , rebuildSpecularAAMaps{ 0 }
{
    editableMaterialData = { 0 };
    editableMaterialData.LayerCount = 1;
    //editableMaterialData.AlphaCutoff = 0.0f;
    //editableMaterialData.Refraction = 0.0f;
    //editableMaterialData.RefractionIor = 0.0f;
}

Material::Material( Material& material )
    : name( material.name )
    , isEditable( material.isEditable )
    , scaleUVByModelScale( material.scaleUVByModelScale )
    , builderVersion( material.builderVersion )
    , sortKey( material.sortKey )
    , shadingModel( material.shadingModel )
    , pixelTextureSet( material.pixelTextureSet )
    , vertexTextureSet( material.vertexTextureSet )
    , editableMaterialData( material.editableMaterialData )
{

}

Material::~Material()
{
    name.clear();
}

void Material::create( RenderDevice* renderDevice, ShaderStageManager* shaderStageManager, PipelineStateDesc* customPipelineDescriptor )
{
    if ( customPipelineDescriptor == nullptr ) {
        BufferDesc bufferDesc;
        bufferDesc.Type = BufferDesc::CONSTANT_BUFFER;
        bufferDesc.Size = sizeof( editableMaterialData );

        editableMaterialBuffer.reset( new Buffer() );
        editableMaterialBuffer->create( renderDevice, bufferDesc );

        pipelineState.reset( new PipelineState() );
        pipelineStateProbe.reset( new PipelineState() );
        depthPipelineState.reset( new PipelineState() );
        reversedDepthPipelineState.reset( new PipelineState() );

        // Enum helper to figure out which vertex stage should be used
        enum class MaterialType
        {
            SURFACE,
            TERRAIN,
            HUD
        };

        MaterialType materialType = MaterialType::SURFACE;
        fnString_t compiledPixelStage = FLAN_STRING( "Surface" ),
                   compiledProbePixelStage = FLAN_STRING( "SurfaceProbeCapture" );

        switch ( shadingModel ) {
        case flan::graphics::eShadingModel::SHADING_MODEL_STANDARD:
            compiledPixelStage = FLAN_STRING( "SurfaceStandard" );
            compiledProbePixelStage = FLAN_STRING( "SurfaceStandardProbeCapture" );
            break;

        case flan::graphics::eShadingModel::SHADING_MODEL_EMISSIVE:
            compiledPixelStage = FLAN_STRING( "SurfaceEmissive" );
            compiledProbePixelStage = FLAN_STRING( "SurfaceEmissiveProbeCapture" );
            break;

        case flan::graphics::eShadingModel::SHADING_MODEL_CLEAR_COAT:
            compiledPixelStage = FLAN_STRING( "SurfaceClearCoat" );
            compiledProbePixelStage = FLAN_STRING( "SurfaceClearCoatProbeCapture" );
            break;

        case flan::graphics::eShadingModel::SHADING_MODEL_TERRAIN_STANDARD:
            compiledPixelStage = FLAN_STRING( "SurfaceTerrain" );
            compiledProbePixelStage = FLAN_STRING( "SurfaceTerrainProbeCapture" );

            materialType = MaterialType::TERRAIN;
            break;

        case flan::graphics::eShadingModel::SHADING_MODEL_HUD_STANDARD:
            compiledPixelStage = FLAN_STRING( "Primitive2D" );
            compiledProbePixelStage = FLAN_STRING( "" );

            materialType = MaterialType::HUD;
            break;

        default:
            break;
        };

        const bool isWorldMaterial = ( materialType != MaterialType::HUD );

        PipelineStateDesc descriptor;
        if ( materialType == MaterialType::SURFACE ) {
            descriptor.vertexStage = shaderStageManager->getOrUploadStage( ( scaleUVByModelScale )
                ? FLAN_STRING( "SurfaceScaledUVNormalMapping" )
                : FLAN_STRING( "SurfaceNormalMapping" ), SHADER_STAGE_VERTEX );
        } else if ( materialType == MaterialType::TERRAIN ) {
            descriptor.vertexStage = shaderStageManager->getOrUploadStage( ( scaleUVByModelScale )
                ? FLAN_STRING( "HeightfieldScaledUV" )
                : FLAN_STRING( "Heightfield" ), SHADER_STAGE_VERTEX );
        } else if ( materialType == MaterialType::HUD ) {
            descriptor.vertexStage = shaderStageManager->getOrUploadStage( FLAN_STRING( "Primitive2D" ), SHADER_STAGE_VERTEX );
        }

        descriptor.pixelStage = shaderStageManager->getOrUploadStage( compiledPixelStage.c_str(), SHADER_STAGE_PIXEL );
        descriptor.primitiveTopology = ( materialType == MaterialType::TERRAIN )
            ? flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
            : flan::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        RasterizerStateDesc rasterDesc;
        rasterDesc.fillMode = flan::rendering::eFillMode::FILL_MODE_SOLID;
        rasterDesc.cullMode = ( isWorldMaterial )
            ? ( editableMaterialData.IsDoubleFace ) ? flan::rendering::eCullMode::CULL_MODE_NONE : flan::rendering::eCullMode::CULL_MODE_BACK
            : flan::rendering::eCullMode::CULL_MODE_FRONT;
        rasterDesc.useTriangleCCW = true;

        DepthStencilStateDesc depthStencilDesc;
        depthStencilDesc.enableDepthWrite = false;
        depthStencilDesc.enableStencilTest = false;
        
        if ( isWorldMaterial ) {
            depthStencilDesc.enableDepthTest = true;
            depthStencilDesc.depthComparisonFunc = ( editableMaterialData.EnableAlphaBlend )
                                                ? flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_GEQUAL
                                                : flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_EQUAL;
        } else {
            depthStencilDesc.enableDepthTest = false;
            depthStencilDesc.depthComparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_ALWAYS;
        }

        descriptor.rasterizerState = new RasterizerState();
        descriptor.rasterizerState->create( renderDevice, rasterDesc );

        descriptor.depthStencilState = new DepthStencilState();
        descriptor.depthStencilState->create( renderDevice, depthStencilDesc );

        if ( isWorldMaterial ) {
            descriptor.inputLayout = {
                { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, false, "POSITION" },
                { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, true, "NORMAL" },
                { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0, 0, true, "TEXCOORD" },
            };
        } else {
            descriptor.inputLayout = {
                { 0, IMAGE_FORMAT_R32G32B32_FLOAT, 0, 0, 0, false, "POSITION" },
                { 0, IMAGE_FORMAT_R32G32_FLOAT, 0, 0, 0, true, "TEXCOORD" },
            };
        }

        if ( editableMaterialData.EnableAlphaBlend == 1 ) {
            descriptor.blendState = new BlendState();

            BlendStateDesc blendStateDesc;
            blendStateDesc.blendConfColor.operation = flan::rendering::eBlendOperation::BLEND_OPERATION_ADD;
            blendStateDesc.blendConfColor.source = flan::rendering::eBlendSource::BLEND_SOURCE_SRC_ALPHA;
            blendStateDesc.blendConfColor.dest = flan::rendering::eBlendSource::BLEND_SOURCE_INV_SRC_ALPHA;

            blendStateDesc.blendConfAlpha.operation = flan::rendering::eBlendOperation::BLEND_OPERATION_ADD;
            blendStateDesc.blendConfAlpha.source = flan::rendering::eBlendSource::BLEND_SOURCE_INV_DEST_ALPHA;
            blendStateDesc.blendConfAlpha.dest = flan::rendering::eBlendSource::BLEND_SOURCE_ONE;

            blendStateDesc.enableBlend = true;
            blendStateDesc.sampleMask = ~0;
            blendStateDesc.enableAlphaToCoverage = ( editableMaterialData.AlphaToCoverage == 1 );

            descriptor.blendState->create( renderDevice, blendStateDesc );
        }

        pipelineState->create( renderDevice, descriptor );

        // Probe Pipeline State
        if ( isWorldMaterial ) {
            descriptor.pixelStage = shaderStageManager->getOrUploadStage( compiledProbePixelStage.c_str(), SHADER_STAGE_PIXEL );
            pipelineStateProbe->create( renderDevice, descriptor );
        }

        // Depth Only Pipeline State
        descriptor.vertexStage = ( materialType == MaterialType::SURFACE ) 
            ? shaderStageManager->getOrUploadStage( FLAN_STRING( "DepthWrite" ), SHADER_STAGE_VERTEX )
            : shaderStageManager->getOrUploadStage( FLAN_STRING( "DepthWriteHeightmap" ), SHADER_STAGE_VERTEX );

        if ( editableMaterialData.EnableAlphaTest )
            descriptor.pixelStage = shaderStageManager->getOrUploadStage( FLAN_STRING( "SurfaceDepth" ), SHADER_STAGE_PIXEL );
        else
            descriptor.pixelStage = nullptr;

        depthStencilDesc = {};
        depthStencilDesc.depthComparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_LESS;
        depthStencilDesc.enableDepthWrite = true;
        depthStencilDesc.enableDepthTest = true;
        depthStencilDesc.enableStencilTest = false;

        descriptor.depthStencilState = new DepthStencilState();
        descriptor.depthStencilState->create( renderDevice, depthStencilDesc );

        rasterDesc = {};
        rasterDesc.fillMode = flan::rendering::eFillMode::FILL_MODE_SOLID;
        rasterDesc.cullMode = ( editableMaterialData.IsDoubleFace ) 
            ? flan::rendering::eCullMode::CULL_MODE_NONE 
            : flan::rendering::eCullMode::CULL_MODE_FRONT;
        rasterDesc.useTriangleCCW = true;

        descriptor.rasterizerState = new RasterizerState();
        descriptor.rasterizerState->create( renderDevice, rasterDesc );

        depthPipelineState->create( renderDevice, descriptor );

        // Reversed Depth Only
        if ( !editableMaterialData.EnableAlphaBlend ) {
            depthStencilDesc.depthComparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_GREATER;
            descriptor.depthStencilState = new DepthStencilState();
            descriptor.depthStencilState->create( renderDevice, depthStencilDesc );
            rasterDesc.cullMode = ( editableMaterialData.IsDoubleFace )
                ? flan::rendering::eCullMode::CULL_MODE_NONE
                : flan::rendering::eCullMode::CULL_MODE_FRONT;

            reversedDepthPipelineState->create( renderDevice, descriptor );
        } else {
            reversedDepthPipelineState.reset( nullptr );
        }
    } else {
        isEditable = false;

        pipelineState.reset( new PipelineState() );
        pipelineState->create( renderDevice, *customPipelineDescriptor );
    }

    sortKeyInfos.isDoubleFace = editableMaterialData.IsDoubleFace;
    sortKeyInfos.isAlphaTested = editableMaterialData.EnableAlphaTest;
    sortKeyInfos.isAlphaBlended = editableMaterialData.EnableAlphaBlend;
    sortKeyInfos.writeVelocity = editableMaterialData.WriteVelocity;
    sortKeyInfos.scaleUVByModelScale = scaleUVByModelScale;
    sortKeyInfos.useAlphaToCoverage = editableMaterialData.AlphaToCoverage;
    sortKeyInfos.receiveShadow = editableMaterialData.ReceiveShadow;
    sortKeyInfos.castShadow = editableMaterialData.CastShadow;
    sortKeyInfos.shadingModel = shadingModel;
    //sortKeyInfos.useRefraction = editableMaterialData.Refraction != 0.0f;
    sortKeyInfos.useTranslucidity = ( sortKeyInfos.isAlphaTested || sortKeyInfos.isAlphaBlended );
}

void Material::readEditableMaterialInput( const fnString_t& materialInputLine, const uint32_t layerIndex, const uint32_t inputTextureBindIndex, GraphicsAssetManager* graphicsAssetManager, MaterialEditionInput& materialComponent, fnTextureSet_t& textureSet )
{
    auto valueHashcode = flan::core::CRC32( materialInputLine.c_str() );
    const bool isNone = ( valueHashcode == FLAN_STRING_HASH( "None" ) ) || materialInputLine.empty();

    if ( isNone ) {
        materialComponent.InputType = MaterialEditionInput::NONE;
    } else if ( materialInputLine.front() == '{' && materialInputLine.back() == '}' ) {
        materialComponent.InputType = MaterialEditionInput::COLOR_3D;
        materialComponent.Input3D = flan::core::StringTo3DVector( materialInputLine );
    } else if ( materialInputLine.front() == '#' && materialInputLine.length() == 7 ) {
        // Assuming the following hex format: #RRGGBB (no explicit alpha)
        float redChannel = std::stoul( materialInputLine.substr( 1, 2 ).c_str(), nullptr, 16 ) / 255.0f;
        float greenChannel = std::stoul( materialInputLine.substr( 3, 2 ).c_str(), nullptr, 16 ) / 255.0f;
        float blueChannel = std::stoul( materialInputLine.substr( 5, 2 ).c_str(), nullptr, 16 ) / 255.0f;

        materialComponent.InputType = MaterialEditionInput::COLOR_3D;
        materialComponent.Input3D = glm::vec3( redChannel, greenChannel, blueChannel );
    } else if ( materialInputLine.front() == '"' && materialInputLine.back() == '"' ) {
        materialComponent.InputType = MaterialEditionInput::TEXTURE;
        materialComponent.InputTexture = graphicsAssetManager->getTexture( flan::core::WrappedStringToString( materialInputLine ).c_str() );

        textureSet.insert( std::make_pair( inputTextureBindIndex, materialComponent.InputTexture ) );
    } else {
        materialComponent.InputType = MaterialEditionInput::COLOR_1D;
        materialComponent.Input1D = std::stof( materialInputLine );
    }
}

uint32_t GetLayerBaseSlotIndex( const uint32_t layerIndex )
{
    return ( TEXTURE_SLOT_INDEX_MATERIAL_BEGIN + ( layerIndex * 10 ) + layerIndex );
}

void Material::deserialize( FileSystemObject* file, GraphicsAssetManager* graphicsAssetManager )
{
#define FLAN_CASE_READ_MATERIAL_FLAG( streamLine, variable ) case FLAN_STRING_HASH( #variable ): editableMaterialData.variable = flan::core::StringToBoolean( streamLine ); break;
#define FLAN_CASE_READ_MATERIAL_FLOAT( streamLine, layerIndex, variable ) case FLAN_STRING_HASH( #variable ): editableMaterialData.layers[layerIndex].variable = std::stof( dictionaryValue.c_str() ); break;
#define FLAN_CASE_READ_LAYER_PIXEL_INPUT( streamLine, layerIndex, variableIndex, variable )  case FLAN_STRING_HASH( #variable ): readEditableMaterialInput( streamLine, layerIndex, variableIndex, graphicsAssetManager, editableMaterialData.layers[layerIndex].variable, pixelTextureSet ); break;
#define FLAN_CASE_READ_LAYER_VERTEX_INPUT( streamLine, layerIndex, variableIndex, variable )  case FLAN_STRING_HASH( #variable ): readEditableMaterialInput( streamLine, layerIndex, variableIndex, graphicsAssetManager, editableMaterialData.layers[layerIndex].variable, vertexTextureSet ); break;

    // Reset material inputs (incase of hot reloading)
    vertexTextureSet.clear();
    pixelTextureSet.clear();

    fnString_t streamLine, dictionaryKey, dictionaryValue;

    int currentLayerIndex = -1;
    uint32_t slotBaseIndex = 0;
    while ( file->isGood() ) {
        flan::core::ReadString( file, streamLine );

        // Find seperator character offset in the line (if any)
        const auto commentSeparator = streamLine.find_first_of( FLAN_STRING( ";" ) );

        // Remove user comments before reading the keypair value
        if ( commentSeparator != fnString_t::npos ) {
            streamLine.erase( streamLine.begin() + commentSeparator, streamLine.end() );
        }

        // Skip commented out and empty lines
        if ( streamLine.empty() ) {
            continue;
        }

        const auto keyValueSeparator = streamLine.find_first_of( ':' );

        // Check if this is a key value line
        if ( keyValueSeparator != fnString_t::npos ) {
            dictionaryKey = streamLine.substr( 0, keyValueSeparator );
            dictionaryValue = streamLine.substr( keyValueSeparator + 1 );

            // Trim both key and values (useful if a file has inconsistent spacing, ...)
            flan::core::TrimString( dictionaryKey );
            flan::core::TrimString( dictionaryValue );

            // Do the check after triming, since the value might be a space or a tab character
            if ( !dictionaryValue.empty() ) {
                auto keyHashcode = flan::core::CRC32( dictionaryKey.c_str() );

                switch ( keyHashcode ) {
                case FLAN_STRING_HASH( "Name" ):
                    name = flan::core::WrappedStringToString( dictionaryValue );
                    break;

                case FLAN_STRING_HASH( "ShadingModel" ):
                    shadingModel = flan::graphics::StringToShadingModel( flan::core::CRC32( dictionaryValue ) );
                    break;

                case FLAN_STRING_HASH( "Version" ):
                    builderVersion = std::stoi( dictionaryValue );
                    break;

                case FLAN_STRING_HASH( "ScaleUVByModelScale" ):
                    scaleUVByModelScale = flan::core::StringToBoolean( dictionaryValue );
                    break;

                case FLAN_STRING_HASH( "UseBaseColorsRGBSource" ):
                    editableMaterialData.layers[currentLayerIndex].BaseColor.SamplingFlags = flan::core::StringToBoolean( dictionaryValue ) 
                        ? MaterialEditionInput::SRGB_SOURCE 
                        : MaterialEditionInput::LINEAR_SOURCE;
                    break;

                case FLAN_STRING_HASH( "UseAlphaRoughnessSource" ):
                    editableMaterialData.layers[currentLayerIndex].Roughness.SamplingFlags = flan::core::StringToBoolean( dictionaryValue )
                        ? MaterialEditionInput::ALPHA_ROUGHNESS_SOURCE
                        : MaterialEditionInput::ROUGHNESS_SOURCE;
                    break;

                case FLAN_STRING_HASH( "IsNormalMapTangentSpace" ):
                    editableMaterialData.layers[currentLayerIndex].Normal.SamplingFlags = flan::core::StringToBoolean( dictionaryValue )
                        ? MaterialEditionInput::TANGENT_SPACE_SOURCE
                        : MaterialEditionInput::WORLD_SPACE_SOURCE;
                    break;

                case FLAN_STRING_HASH( "IsSecondaryNormalMapTangentSpace" ):
                    editableMaterialData.layers[currentLayerIndex].SecondaryNormal.SamplingFlags = flan::core::StringToBoolean( dictionaryValue )
                        ? MaterialEditionInput::TANGENT_SPACE_SOURCE
                        : MaterialEditionInput::WORLD_SPACE_SOURCE;
                    break;

                // Material Flags
                FLAN_CASE_READ_MATERIAL_FLAG( dictionaryValue, WriteVelocity );
                FLAN_CASE_READ_MATERIAL_FLAG( dictionaryValue, EnableAlphaTest );
                FLAN_CASE_READ_MATERIAL_FLAG( dictionaryValue, IsDoubleFace );
                FLAN_CASE_READ_MATERIAL_FLAG( dictionaryValue, EnableAlphaBlend );
                FLAN_CASE_READ_MATERIAL_FLAG( dictionaryValue, CastShadow );
                FLAN_CASE_READ_MATERIAL_FLAG( dictionaryValue, ReceiveShadow );
                FLAN_CASE_READ_MATERIAL_FLAG( dictionaryValue, AlphaToCoverage );

                // Shading Model Inputs
                if ( currentLayerIndex == 0 ) {
                    FLAN_CASE_READ_LAYER_VERTEX_INPUT( dictionaryValue, currentLayerIndex, slotBaseIndex, Heightmap )
                }

                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, slotBaseIndex, BaseColor )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 2 ), Reflectance )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 3 ), Roughness )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 4 ), Metalness )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 5 ), AmbientOcclusion )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 6 ), Normal )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 7 ), Emissivity )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 1 ), AlphaMask )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 8 ), Displacement )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 9 ), SecondaryNormal )
                FLAN_CASE_READ_LAYER_PIXEL_INPUT( dictionaryValue, currentLayerIndex, ( slotBaseIndex + 10 ), BlendMask )

                // Misc. / Blending Inputs
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, Refraction )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, RefractionIor )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, ClearCoat )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, ClearCoatGlossiness )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, DiffuseContribution )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, SpecularContribution )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, NormalContribution )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, AlphaCutoff )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, NormalMapStrength )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, SecondaryNormalMapStrength )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, DisplacementMapStrength )
                FLAN_CASE_READ_MATERIAL_FLOAT( dictionaryValue, currentLayerIndex, HeightmapWorldHeight )

                // Layer Scaling
                case FLAN_STRING_HASH( "Offset" ):
                    editableMaterialData.layers[currentLayerIndex].LayerOffset = flan::core::StringTo2DVector( dictionaryValue );
                    break;

                case FLAN_STRING_HASH( "Scale" ):
                    editableMaterialData.layers[currentLayerIndex].LayerScale = flan::core::StringTo2DVector( dictionaryValue );
                    break;
                }
            }
        } else {
            dictionaryKey = streamLine.substr( 0, keyValueSeparator );

            // Trim both key and values (useful if a file has inconsistent spacing, ...)
            flan::core::TrimString( dictionaryKey );

            auto keyHashcode = flan::core::CRC32( dictionaryKey.c_str() );

            switch ( keyHashcode ) {
            case FLAN_STRING_HASH( "Layer" ):
                if ( currentLayerIndex < MAX_LAYER_COUNT ) {
                    currentLayerIndex++; 
                    slotBaseIndex = GetLayerBaseSlotIndex( currentLayerIndex );
                }
                break;
            }
        }
    }

    editableMaterialData.LayerCount = ( currentLayerIndex + 1 );

    file->close();
}

void Material::serialize( FileSystemObject* file ) const
{
    auto WriteInput = [this]( const MaterialEditionInput& input, const uint32_t inputIndex, const fnTextureSet_t& textureSet ) {
        switch ( input.InputType ) {
        case MaterialEditionInput::COLOR_3D:
            return "{ " + std::to_string( input.Input3D.r ) + ", " + std::to_string( input.Input3D.g ) + ", " + std::to_string( input.Input3D.b ) + " }";
        case MaterialEditionInput::COLOR_1D:
            return std::to_string( input.Input1D );
        case MaterialEditionInput::TEXTURE:
            return "\"" + textureSet.at( inputIndex )->getResourceName() + "\"";
        default:
        case MaterialEditionInput::NONE:
            return std::string( "None" );
        }
    };

    file->writeString( "Name: \"" + flan::core::WideStringToString( name ) + "\"\n" );
    file->writeString( "Version: 3\n" );
    file->writeString( "ShadingModel:" + std::string( flan::graphics::ShadingModelToString[shadingModel] ) + "\n\n");

    file->writeString( "WriteVelocity:" + std::to_string( editableMaterialData.WriteVelocity ) + "\n" );
    file->writeString( "EnableAlphaTest:" + std::to_string( editableMaterialData.EnableAlphaTest ) + "\n" );
    file->writeString( "EnableAlphaToCoverage:" + std::to_string( editableMaterialData.AlphaToCoverage ) + "\n\n" );
    file->writeString( "IsDoubleFace:" + std::to_string( editableMaterialData.IsDoubleFace ) + "\n" );
    file->writeString( "EnableAlphaBlend:" + std::to_string( editableMaterialData.EnableAlphaBlend ) + "\n" );
    file->writeString( "CastShadow:" + std::to_string( editableMaterialData.CastShadow ) + "\n" );
    file->writeString( "ReceiveShadow:" + std::to_string( editableMaterialData.ReceiveShadow ) + "\n\n" );
    file->writeString( "ScaleUVByModelScale:" + std::to_string( scaleUVByModelScale ) + "\n\n" );

#define FLAN_WRITE_INPUT_PIXEL( input, inputIdx ) file->writeString( "\t" #input## ": " + WriteInput( layer.input, inputIdx, pixelTextureSet ) + "\n" );
#define FLAN_WRITE_INPUT_VERTEX( input, inputIdx ) file->writeString( "\t" #input## ": " + WriteInput( layer.input, inputIdx, vertexTextureSet ) + "\n" );
#define FLAN_WRITE_VARIABLE( input ) file->writeString( "\t" #input## ": " +std::to_string( layer.input ) + "\n" );

    for ( uint32_t i = 0; i < editableMaterialData.LayerCount; i++ ) {
        const auto& layer = editableMaterialData.layers[i];
        auto slotBaseIndex = GetLayerBaseSlotIndex( i );

        file->writeString( "Layer\n{\n" );

        if ( i == 0 ) {
            FLAN_WRITE_INPUT_VERTEX( Heightmap, 0 )
        }

        FLAN_WRITE_INPUT_PIXEL( BaseColor, slotBaseIndex )
        FLAN_WRITE_INPUT_PIXEL( Reflectance, ( slotBaseIndex + 2 ) )
        FLAN_WRITE_INPUT_PIXEL( Roughness, ( slotBaseIndex + 3 ) )
        FLAN_WRITE_INPUT_PIXEL( Metalness, ( slotBaseIndex + 4 ) )
        FLAN_WRITE_INPUT_PIXEL( AmbientOcclusion, ( slotBaseIndex + 5 ) )
        FLAN_WRITE_INPUT_PIXEL( Normal, ( slotBaseIndex + 6 ) )
        FLAN_WRITE_INPUT_PIXEL( Emissivity, ( slotBaseIndex + 7 ) )
        FLAN_WRITE_INPUT_PIXEL( AlphaMask, ( slotBaseIndex + 1 ) )
        FLAN_WRITE_INPUT_PIXEL( Displacement, ( slotBaseIndex + 8 ) )
        FLAN_WRITE_INPUT_PIXEL( SecondaryNormal, ( slotBaseIndex + 9 ) )
        FLAN_WRITE_INPUT_PIXEL( BlendMask, ( slotBaseIndex + 10 ) )

        FLAN_WRITE_VARIABLE( Refraction )
        FLAN_WRITE_VARIABLE( RefractionIor )
        FLAN_WRITE_VARIABLE( ClearCoat )
        FLAN_WRITE_VARIABLE( ClearCoatGlossiness )
        FLAN_WRITE_VARIABLE( DiffuseContribution )
        FLAN_WRITE_VARIABLE( SpecularContribution )
        FLAN_WRITE_VARIABLE( NormalContribution )
        FLAN_WRITE_VARIABLE( AlphaCutoff )
        FLAN_WRITE_VARIABLE( NormalMapStrength )
        FLAN_WRITE_VARIABLE( SecondaryNormalMapStrength )
        FLAN_WRITE_VARIABLE( DisplacementMapStrength )

        file->writeString( "\tOffset: { " + std::to_string( layer.LayerOffset.x ) + ", " + std::to_string( layer.LayerOffset.y ) + " }\n" );
        file->writeString( "\tScale: { " + std::to_string( layer.LayerScale.x ) + ", " + std::to_string( layer.LayerScale.y ) + " }\n" );

        file->writeString( "\tUseBaseColorsRGBSource: " + std::string( ( editableMaterialData.layers[i].BaseColor.SamplingFlags == MaterialEditionInput::SRGB_SOURCE ) ? "True" : "False" ) + "\n" );
        file->writeString( "\tUseAlphaRoughnessSource: " + std::string( ( editableMaterialData.layers[i].Roughness.SamplingFlags == MaterialEditionInput::ALPHA_ROUGHNESS_SOURCE ) ? "True" : "False" ) + "\n" );
        file->writeString( "\tIsNormalMapTangentSpace: " + std::string( ( editableMaterialData.layers[i].Normal .SamplingFlags == MaterialEditionInput::TANGENT_SPACE_SOURCE ) ? "True" : "False" ) + "\n" );
        file->writeString( "\tIsSecondaryNormalMapTangentSpace: " + std::string( ( editableMaterialData.layers[i].SecondaryNormal.SamplingFlags == MaterialEditionInput::TANGENT_SPACE_SOURCE ) ? "True" : "False" ) + "\n" );

        file->writeString( "}\n\n" );
    }
}

void Material::bind( CommandList* cmdList ) const
{
    cmdList->bindPipelineStateCmd( pipelineState.get() );

    for ( auto& textureSlot : vertexTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_VERTEX );
    }

    for ( auto& textureSlot : pixelTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_PIXEL );
    }

    if ( isEditable ) {
        editableMaterialBuffer->bind( cmdList, CBUFFER_INDEX_MATERIAL_EDITOR, SHADER_STAGE_PIXEL );
        editableMaterialBuffer->updateAsynchronous( cmdList, &editableMaterialData, sizeof( editableMaterialData ) );
    }
}

bool Material::bindReversedDepthOnly( CommandList* cmdList ) const
{
    // Alpha blended surfaces (e.g. glass) should skip z prepass
    if ( reversedDepthPipelineState == nullptr ) {
        return false;
    }

    cmdList->bindPipelineStateCmd( reversedDepthPipelineState.get() );

    for ( auto& textureSlot : vertexTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_VERTEX );
    }

    for ( auto& textureSlot : pixelTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_PIXEL );
    }

    if ( isEditable ) {
        editableMaterialBuffer->bind( cmdList, CBUFFER_INDEX_MATERIAL_EDITOR, SHADER_STAGE_PIXEL );
        editableMaterialBuffer->updateAsynchronous( cmdList, &editableMaterialData, sizeof( editableMaterialData ) );
    }

    return true;
}

void Material::bindDepthOnly( CommandList* cmdList ) const
{
    cmdList->bindPipelineStateCmd( depthPipelineState.get() );

    for ( auto& textureSlot : vertexTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_VERTEX );
    }

    for ( auto& textureSlot : pixelTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_PIXEL );
    }

    if ( isEditable ) {
        editableMaterialBuffer->bind( cmdList, CBUFFER_INDEX_MATERIAL_EDITOR, SHADER_STAGE_PIXEL );
        editableMaterialBuffer->updateAsynchronous( cmdList, &editableMaterialData, sizeof( editableMaterialData ) );
    }
}

void Material::bindForProbeRendering( CommandList* cmdList ) const
{
    cmdList->bindPipelineStateCmd( pipelineStateProbe.get() );

    for ( auto& textureSlot : vertexTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_VERTEX );
    }

    for ( auto& textureSlot : pixelTextureSet ) {
        textureSlot.second->bind( cmdList, textureSlot.first, SHADER_STAGE_PIXEL );
    }

    if ( isEditable ) {
        editableMaterialBuffer->bind( cmdList, CBUFFER_INDEX_MATERIAL_EDITOR, SHADER_STAGE_PIXEL );
        editableMaterialBuffer->updateAsynchronous( cmdList, &editableMaterialData, sizeof( editableMaterialData ) );
    }
}

const uint32_t Material::getMaterialSortKey() const
{
    return sortKey;
}

const bool Material::isOpaque() const
{
    return !sortKeyInfos.useTranslucidity;
}

const fnString_t& Material::getName() const
{
    return name;
}

#if FLAN_DEVBUILD
#include <imgui/imgui.h>
#include <Core/FileSystemIOHelpers.h>
#include <Core/Environment.h>
#include <Graphics/TextureSaveTools.h>

void Material::displayInputConfiguration( GraphicsAssetManager* graphicsAssetManager, const std::string& displayName, MaterialEditionInput& input, const uint32_t inputTextureBindIndex, fnTextureSet_t& textureSet, const bool saturate )
{
    static constexpr char* SlotInputTypeLabels[MaterialEditionInput::EDITABLE_MATERIAL_COMPONENT_TYPE_COUNT] = {
        "None",
        "1D Constant Value",
        "3D Constant Value",
        "Texture"
    };

    // Input label
    ImGui::LabelText( ( "##hidden_" + displayName + std::to_string( inputTextureBindIndex ) ).c_str(), displayName.c_str() );

    ImGui::PushItemWidth( 100.0f );
    ImGui::SameLine( 128.0f );

    auto previousInputType = input.InputType;
    if ( ImGui::Combo( ( "##hidden_" + displayName + "Input" + std::to_string(inputTextureBindIndex ) ).c_str(), ( int* )&input.InputType, SlotInputTypeLabels, MaterialEditionInput::EDITABLE_MATERIAL_COMPONENT_TYPE_COUNT ) ) {
        // Check if the new value is different (so that there is no pointless material recompilation)
        if ( previousInputType != input.InputType ) {
            if ( input.InputType == MaterialEditionInput::COLOR_1D ) {
                input.Input1D = 0.0f;
            } else if ( input.InputType == MaterialEditionInput::COLOR_3D ) {
                input.Input3D = glm::vec3( 0, 0, 0 );
            }
        }
    }

    ImGui::PopItemWidth();
    
    if ( input.InputType == MaterialEditionInput::NONE ) {
        return;
    }

    ImGui::SameLine( 233.0f );
    if ( input.InputType == MaterialEditionInput::COLOR_3D ) {
        ImGui::ColorEdit3( ( "##hidden_BaseColorPickerRGB" + displayName + std::to_string(inputTextureBindIndex ) ).c_str(), &input.Input3D[0] );
    } else if ( input.InputType == MaterialEditionInput::COLOR_1D ) {
        if ( saturate ) {
            ImGui::SliderFloat( ( "##hidden_BaseColorPickerR" + displayName + std::to_string( inputTextureBindIndex ) ).c_str(), &input.Input1D, 0.001f, 1.0f );
        } else {
            ImGui::DragFloat( ( "##hidden_BaseColorPickerR" + displayName + std::to_string(inputTextureBindIndex ) ).c_str(), &input.Input1D, 1.0f, 0.0f );
        }
    } else if ( input.InputType == MaterialEditionInput::TEXTURE ) {
        // Input label
        ImVec2 texInfosSize = ImVec2( 318.0f, 64.0f );
        std::string imageInfos = "No texture selected...";

        auto textureSetIterator = textureSet.find( inputTextureBindIndex );
        if ( textureSetIterator != textureSet.end()
            && textureSetIterator->second != nullptr ) {
            // Build Image Infos Text
            imageInfos.clear();

            const auto& textureName = textureSetIterator->second->getResourceName();
            const auto& textureDesc = textureSetIterator->second->getDescription();

            imageInfos += "Name: " + textureName + "\n";
            imageInfos += "Dimensions: " + std::to_string( textureDesc.width ) + "x"
                + std::to_string( textureDesc.height ) + "\n";
            imageInfos += "Mip Count: " + std::to_string( textureDesc.mipCount );


#if FLAN_D3D11
            auto nativeObj = textureSetIterator->second->getNativeObject();
            if ( ImGui::ImageButton( nativeObj->textureShaderResourceView, ImVec2( 58, 58 ) ) ) {
#elif defined( FLAN_VULKAN ) || defined( FLAN_GL460 )
            auto nativeObj = textures[textureIndex]->getNativeObject();
            if ( ImGui::Button( "balh", ImVec2( 58, 58 ) ) ) {
#endif
                fnString_t filenameBuffer;
                if ( flan::core::DisplayFileOpenPrompt( filenameBuffer, L"All (*.dds, *.jpg, *.png, *.tga)\0*.dds;*.jpg;*.png;*.tga\0DirectDraw Surface (*.dds)\0*.dds\0JPG (*.jpg)\0*.jpg\0PNG (*.png)\0*.png\0TGA (*.tga)\0*.tga\0", L"/.", L"Select a Texture..." ) ) {
                    fnString_t assetPath;
                    flan::core::ExtractFilenameFromPath( filenameBuffer, assetPath );

                    input.InputTexture = graphicsAssetManager->getTexture( ( FLAN_STRING( "GameData/Textures/" ) + assetPath ).c_str() );
                    pixelTextureSet[inputTextureBindIndex] = input.InputTexture;
                }
            }
        } else {
            if ( ImGui::Button( ( "+##hidden_" + displayName + std::to_string( inputTextureBindIndex ) ).c_str(), ImVec2( 64, 64 ) ) ) {
                fnString_t filenameBuffer;
                if ( flan::core::DisplayFileOpenPrompt( filenameBuffer, L"All (*.dds, *.jpg, *.png, *.tga)\0*.dds;*.jpg;*.png;*.tga\0DirectDraw Surface (*.dds)\0*.dds\0JPG (*.jpg)\0*.jpg\0PNG (*.png)\0*.png\0TGA (*.tga)\0*.tga\0", L"/.", L"Select a Texture..." ) ) {
                    fnString_t assetPath;
                    flan::core::ExtractFilenameFromPath( filenameBuffer, assetPath );

                    input.InputTexture = graphicsAssetManager->getTexture( ( FLAN_STRING( "GameData/Textures/" ) + assetPath ).c_str() );
                    textureSet[inputTextureBindIndex] = input.InputTexture;
                }
            }
            texInfosSize.x += 2;
        }

        ImGui::SameLine();
        ImGui::InputTextMultiline( ( "##hidden__TexInfos" + displayName + std::to_string( inputTextureBindIndex ) ).c_str(), &imageInfos.front(), imageInfos.length(), ImVec2( 0, 0 ), ImGuiInputTextFlags_ReadOnly );
    }
}

void Material::drawInEditor( RenderDevice* renderDevice, ShaderStageManager* shaderStageManager, GraphicsAssetManager* graphicsAssetManager, WorldRenderer* worldRenderer )
{
    if ( ImGui::Button( "New..." ) ) {
        editableMaterialData.WriteVelocity = 1;
        editableMaterialData.EnableAlphaTest = 0;
        editableMaterialData.EnableAlphaBlend = 0;
        editableMaterialData.IsDoubleFace = 0;

        editableMaterialData.CastShadow = 1;
        editableMaterialData.ReceiveShadow = 1;
        editableMaterialData.AlphaToCoverage = 0;
        editableMaterialData.LayerCount = 1;
    }

    ImGui::SameLine();

    if ( ImGui::Button( "Load From File" ) ) {
    }

    ImGui::SameLine();

    if ( ImGui::Button( "Save To File" ) ) {
        fnString_t materialName;
        if ( flan::core::DisplayFileSavePrompt( materialName, FLAN_STRING( "Asset Material file (*.amat)\0*.amat" ), FLAN_STRING( "./" ), FLAN_STRING( "Save as a Material asset" ) ) ) {
            materialName = fnString_t( materialName.c_str() );
            materialName.append( FLAN_STRING( ".amat" ) );
            auto file = new FileSystemObjectNative( materialName );
            file->open( std::ios::out );
            serialize( file );
            file->close();
            delete file;
        }
    }

    FLAN_IMPORT_VAR_PTR( dev_IsInputText, bool )

    auto strName = flan::core::WideStringToString( name );
    ImGui::LabelText( "##hidden_Name_0", "Name" );

    ImGui::SameLine( 128.0f );

    if ( ImGui::InputText( "##hidden_Name", &strName[0], 256, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue ) ) {
        *dev_IsInputText = !*dev_IsInputText;
    }

    if ( ImGui::IsItemClicked() ) {
        *dev_IsInputText = !*dev_IsInputText;
    }

    if ( isEditable ) {
        ImGui::LabelText( "##hidden_ShadingModel_0", "Shading Model" );
        ImGui::SameLine( 128.0f );
        if ( ImGui::Combo( "##hidden_ShadingModel", ( int* )&shadingModel, flan::graphics::ShadingModelToString, flan::graphics::eShadingModel::ShadingModel_COUNT ) ) {
            create( renderDevice, shaderStageManager );
        }

        ImGui::BeginGroup();

        if ( ImGui::Checkbox( "Is Double Face", ( bool* )&editableMaterialData.IsDoubleFace ) ) {
            create( renderDevice, shaderStageManager );
        }

        // Alpha Test Flags
        if ( ImGui::Checkbox( "Enable Alpha Test", ( bool* )&editableMaterialData.EnableAlphaTest ) ) {
            create( renderDevice, shaderStageManager );
        }

        if ( ImGui::Checkbox( "Enable Alpha Blend", ( bool* )&editableMaterialData.EnableAlphaBlend ) ) {
            create( renderDevice, shaderStageManager );
        }

        if ( editableMaterialData.EnableAlphaTest == 1 ) {
            if ( ImGui::Checkbox( "Use Alpha To Coverage", ( bool* )&editableMaterialData.AlphaToCoverage ) ) {
                create( renderDevice, shaderStageManager );
            }
        }

        if ( ImGui::Checkbox( "Write Velocity", ( bool* )&editableMaterialData.WriteVelocity ) ) {
            create( renderDevice, shaderStageManager );
        }

        if ( ImGui::Checkbox( "Scale UV Coordinates by Model Scale", &scaleUVByModelScale ) ) {
            create( renderDevice, shaderStageManager );
        }

        // Shadows
        if ( ImGui::Checkbox( "Receive Shadow", ( bool* )&editableMaterialData.ReceiveShadow ) ) {
            create( renderDevice, shaderStageManager );
        }

        if ( ImGui::Checkbox( "Cast Shadow", ( bool* )&editableMaterialData.CastShadow ) ) {
            create( renderDevice, shaderStageManager );
        }

        ImGui::EndGroup();

        // Material Layers
        if ( ImGui::Button( "Add Layer" ) && editableMaterialData.LayerCount < MAX_LAYER_COUNT ) {
            editableMaterialData.LayerCount++;

            auto& addedLayer = editableMaterialData.layers[( editableMaterialData.LayerCount - 1 )];

            addedLayer.BaseColor.InputType = MaterialEditionInput::COLOR_3D;
            addedLayer.BaseColor.Input3D = { 0.42f, 0.42f, 0.42f };

            addedLayer.Reflectance.InputType = MaterialEditionInput::COLOR_1D;
            addedLayer.Reflectance.Input1D = 1.0f;

            addedLayer.Roughness.InputType = MaterialEditionInput::COLOR_1D;
            addedLayer.Roughness.Input1D = 0.80f;

            addedLayer.Metalness.InputType = MaterialEditionInput::COLOR_1D;
            addedLayer.Metalness.Input1D = 0.0f;

            addedLayer.AmbientOcclusion.InputType = MaterialEditionInput::NONE;
            addedLayer.Normal.InputType = MaterialEditionInput::NONE;
            addedLayer.Emissivity.InputType = MaterialEditionInput::NONE;
            addedLayer.AlphaMask.InputType = MaterialEditionInput::NONE;
            addedLayer.Displacement.InputType = MaterialEditionInput::NONE;
            addedLayer.SecondaryNormal.InputType = MaterialEditionInput::NONE;

            addedLayer.BlendMask.InputType = MaterialEditionInput::COLOR_1D;
            addedLayer.BlendMask.Input1D = 0.5f;

            addedLayer.Refraction = 0.0f;
            addedLayer.RefractionIor = 0.0f;
            addedLayer.ClearCoat = 0.0f;
            addedLayer.ClearCoatGlossiness = 0.0f;

            addedLayer.NormalMapStrength = 1.0f;
            addedLayer.SecondaryNormalMapStrength = 1.0f;
            addedLayer.DisplacementMapStrength = 1.0f;

            addedLayer.DiffuseContribution = 1.0f;
            addedLayer.SpecularContribution = 1.0f;
            addedLayer.NormalContribution = 1.0f;
            addedLayer.AlphaCutoff = 0.50f;

            addedLayer.LayerScale = glm::vec2( 1.0f, 1.0f );
            addedLayer.LayerOffset = glm::vec2( 0.0f, 0.0f );
        }

        for ( uint32_t i = 0; i < editableMaterialData.LayerCount; i++ ) {
            const bool isBaseLayer = ( i == 0 );
            uint32_t slotBaseIndex = GetLayerBaseSlotIndex( i );

            auto& layer = editableMaterialData.layers[i];
            auto layerLabel = "Layer" + std::to_string( i );

            if ( rebuildSpecularAAMaps[i] == 1 ) {
                fnString_t roughnessMapName = name + FLAN_STRING( "_Layer" ) + FLAN_TO_STRING( i ) + FLAN_STRING( "_Roughness" );
                worldRenderer->saveTexture( roughnessMapRT[i], roughnessMapName );

                rebuildSpecularAAMaps[i] = 2;
            } else if ( rebuildSpecularAAMaps[i] == 2 ) {
                fnString_t roughnessMapName = name + FLAN_STRING( "_Layer" ) + FLAN_TO_STRING( i ) + FLAN_STRING( "_Roughness" );

                layer.Roughness.InputType = MaterialEditionInput::TEXTURE;
                layer.Roughness.InputTexture = graphicsAssetManager->getTexture( ( FLAN_STRING( "GameData/Textures/" ) + roughnessMapName + FLAN_STRING( ".dds" ) ).c_str(), true );
                layer.Roughness.SamplingFlags = MaterialEditionInput::ALPHA_ROUGHNESS_SOURCE;

                pixelTextureSet[( slotBaseIndex + 3 )] = layer.Roughness.InputTexture;

                rebuildSpecularAAMaps[i] = 0;
            }

            if ( !isBaseLayer 
              && ImGui::Button( ( "-##" + std::to_string( i ) ).c_str() ) ) {
                editableMaterialData.LayerCount--;
                i++;
                continue;
            }

            ImGui::SameLine( 200.0f );
            if ( ImGui::TreeNode( layerLabel.c_str() ) ) {
                ImGui::SameLine();
                if ( ImGui::Button( "Precompute Anti-Aliased Roughness Map" )
                    && layer.Normal.InputType == MaterialEditionInput::TEXTURE ) {
                    roughnessMapRT[i] = new RenderTarget();

                    auto& nmDesc = layer.Normal.InputTexture->getDescription();
                    TextureDescription renderTargetDesc;
                    renderTargetDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
                    renderTargetDesc.width = nmDesc.width;
                    renderTargetDesc.height = nmDesc.height;
                    renderTargetDesc.depth = 1;
                    renderTargetDesc.arraySize = 1;
                    renderTargetDesc.mipCount = flan::rendering::ComputeMipCount( nmDesc.width, nmDesc.height );
                    renderTargetDesc.flags.useHardwareMipGen = 1;
                    renderTargetDesc.format = IMAGE_FORMAT_R8_UNORM;
                    roughnessMapRT[i]->createAsRenderTarget2D( renderDevice, renderTargetDesc );

                    if ( layer.Roughness.InputType == MaterialEditionInput::TEXTURE )
                        worldRenderer->precomputeVMF( layer.Normal.InputTexture, layer.Roughness.InputTexture, roughnessMapRT[i] );
                    else
                        worldRenderer->precomputeVMF( layer.Normal.InputTexture, layer.Roughness.Input1D, roughnessMapRT[i] );

                    // Post pone render target retrieval to the next frame (wait for previous cmd list to complete)
                    rebuildSpecularAAMaps[i] = 1;
                }

                displayInputConfiguration( graphicsAssetManager, "Heightmap", layer.Heightmap, 0, vertexTextureSet, false );

                displayInputConfiguration( graphicsAssetManager, "BaseColor", layer.BaseColor, slotBaseIndex, pixelTextureSet );
                bool isSRGBInput = ( layer.BaseColor.SamplingFlags == MaterialEditionInput::SRGB_SOURCE );
                if ( ImGui::Checkbox( "sRGB Input", &isSRGBInput ) ) {
                    layer.BaseColor.SamplingFlags = ( isSRGBInput ) ? MaterialEditionInput::SRGB_SOURCE : MaterialEditionInput::LINEAR_SOURCE;
                }
                
                displayInputConfiguration( graphicsAssetManager, "Emissivity", layer.Emissivity, ( slotBaseIndex + 8 ), pixelTextureSet, false );

                if ( shadingModel != flan::graphics::eShadingModel::SHADING_MODEL_EMISSIVE ) {
                    displayInputConfiguration( graphicsAssetManager, "Reflectance", layer.Reflectance, ( slotBaseIndex + 2 ), pixelTextureSet );
                    displayInputConfiguration( graphicsAssetManager, "Roughness", layer.Roughness, ( slotBaseIndex + 3 ), pixelTextureSet );
                    bool isAlphaRoughness = ( layer.Roughness.SamplingFlags == MaterialEditionInput::ALPHA_ROUGHNESS_SOURCE );
                    if ( ImGui::Checkbox( "Alpha Roughness Input", &isAlphaRoughness ) ) {
                        layer.Roughness.SamplingFlags = ( isAlphaRoughness ) ? MaterialEditionInput::ALPHA_ROUGHNESS_SOURCE : MaterialEditionInput::ROUGHNESS_SOURCE;
                    }

                    displayInputConfiguration( graphicsAssetManager, "Metalness", layer.Metalness, ( slotBaseIndex + 4 ), pixelTextureSet );
                    displayInputConfiguration( graphicsAssetManager, "AmbientOcclusion", layer.AmbientOcclusion, ( slotBaseIndex + 5 ), pixelTextureSet );
                    displayInputConfiguration( graphicsAssetManager, "Normal", layer.Normal, ( slotBaseIndex + 6 ), pixelTextureSet );
                    bool isTangentSpace = ( layer.Normal.SamplingFlags == MaterialEditionInput::TANGENT_SPACE_SOURCE );
                    if ( ImGui::Checkbox( "Tangent Space Input##Normal", &isTangentSpace ) ) {
                        layer.Normal.SamplingFlags = ( isTangentSpace ) ? MaterialEditionInput::TANGENT_SPACE_SOURCE : MaterialEditionInput::WORLD_SPACE_SOURCE;
                    }

                    ImGui::SliderFloat( "Normal Map Strength", &layer.NormalMapStrength, 0.0f, 1.0f );

                    displayInputConfiguration( graphicsAssetManager, "Displacement", layer.Displacement, ( slotBaseIndex + 7 ), pixelTextureSet );
                    ImGui::SliderFloat( "Displacement Strength", &layer.DisplacementMapStrength, 0.0f, 1.0f );

                    displayInputConfiguration( graphicsAssetManager, "AlphaMask", layer.AlphaMask, ( slotBaseIndex + 1 ), pixelTextureSet );


                    if ( editableMaterialData.EnableAlphaTest == 1u ) {
                        ImGui::LabelText( "##hidden_AlphaCutoff_0", "Alpha Cutoff" );
                        ImGui::SameLine( 128.0f );
                        ImGui::DragFloat( "##hidden_AlphaCutoff", &layer.AlphaCutoff );
                    }
                }

                if ( shadingModel == flan::graphics::eShadingModel::SHADING_MODEL_CLEAR_COAT ) {
                    ImGui::LabelText( "##hidden_ClearCoat", "ClearCoat" );
                    ImGui::SameLine( 200.0f );
                    ImGui::PushItemWidth( 318.0f + 64.0f + 10.0f + 155.0f );
                    ImGui::DragFloat( "##hidden_ClearCoatInput", &layer.ClearCoat, 0.001f, 0.0f, 1.0f );

                    ImGui::LabelText( "##hidden_ClearCoatGlossiness", "ClearCoat Glossiness" );
                    ImGui::SameLine( 200.0f );
                    ImGui::DragFloat( "##hidden_ClearCoatGlossinessInput", &layer.ClearCoatGlossiness, 0.001f, 0.0f, 1.0f );
                    ImGui::PopItemWidth();

                    displayInputConfiguration( graphicsAssetManager, "Secondary NormalMap", layer.SecondaryNormal, ( slotBaseIndex + 9 ), pixelTextureSet );
                    bool isTangentSpace = ( layer.SecondaryNormal.SamplingFlags == MaterialEditionInput::TANGENT_SPACE_SOURCE );
                    if ( ImGui::Checkbox( "Tangent Space Input##SecondaryNormal", &isTangentSpace ) ) {
                        layer.SecondaryNormal.SamplingFlags = ( isTangentSpace ) ? MaterialEditionInput::TANGENT_SPACE_SOURCE : MaterialEditionInput::WORLD_SPACE_SOURCE;
                    }

                    ImGui::SliderFloat( "Secondary NormalMap Strength", &layer.SecondaryNormalMapStrength, 0.0f, 1.0f );
                }

                if ( i != 0 ) {
                    displayInputConfiguration( graphicsAssetManager, "BlendMask", layer.BlendMask, ( slotBaseIndex + 10 ), pixelTextureSet );

                    ImGui::LabelText( "##hidden_DiffuseContribution", "Diffuse Contribution" );
                    ImGui::SameLine( 200.0f );
                    ImGui::SliderFloat( "##hidden_DiffuseContributionInput", &layer.DiffuseContribution, 0.0f, 1.0f );

                    ImGui::LabelText( "##hidden_SpecularContribution", "Specular Contribution" );
                    ImGui::SameLine( 200.0f );
                    ImGui::SliderFloat( "##hidden_SpecularContributionInput", &layer.SpecularContribution, 0.0f, 1.0f );

                    ImGui::LabelText( "##hidden_NormalMapContribution", "NormalMap Contribution" );
                    ImGui::SameLine( 200.0f );
                    ImGui::SliderFloat( "##hidden_NormalMapContributionInput", &layer.NormalContribution, 0.0f, 1.0f );
                }

                ImGui::LabelText( "##hidden_Refraction_0", "Refraction" );
                ImGui::SameLine( 128.0f );
                ImGui::DragFloat( "##hidden_Refraction", &layer.Refraction );

                ImGui::LabelText( "##hidden_RefractionIor_0", "Refraction Ior" );
                ImGui::SameLine( 128.0f );
                ImGui::DragFloat( "##hidden_RefractionIor", &layer.RefractionIor, 0.01f, 0.0f );

                ImGui::LabelText( "##hidden_LayerScale_0", "Layer Scale" );
                ImGui::SameLine( 128.0f );
                ImGui::DragFloat2( "##hidden_LayerScale", &layer.LayerScale[0], 0.01f, 0.01f, 1024.0f );

                ImGui::LabelText( "##hidden_LayerScale_0", "Layer Offset" );
                ImGui::SameLine( 128.0f );
                ImGui::DragFloat2( "##hidden_LayerOffset", &layer.LayerOffset[0], 0.01f, 0.01f, 1024.0f );

                // End of Layer
                ImGui::TreePop();
            }
        }
    }
}
#endif
