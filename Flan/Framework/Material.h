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
#pragma once

class RenderDevice;
class CommandList;
class GraphicsAssetManager;
class ShaderStageManager;
class FileSystemObject;
class Texture;
class Buffer;
class PipelineState;
struct PipelineStateDesc;
class WorldRenderer;
class RenderTarget;

#include <unordered_map>

#include <Graphics/ShadingModels.h>
#include <Shaders/MaterialsShared.h>

using fnTextureSet_t = std::unordered_map<int32_t, Texture*>;

// TODO Less atrocious Interface?
class Material
{
public:
                                Material( const fnString_t& materialName = FLAN_STRING( "Mesh" ) );
                                Material( Material& material );
                                Material& operator = ( Material& material ) = default;
                                ~Material();

    void                        create( RenderDevice* renderDevice, ShaderStageManager* shaderStageManager, PipelineStateDesc* customPipelineDescriptor = nullptr );
    void                        deserialize( FileSystemObject* file, GraphicsAssetManager* graphicsAssetManager );
    void                        serialize( FileSystemObject* file ) const;

    void                        bind( CommandList* cmdList ) const;

    // Conditional bindings
    bool                        bindReversedDepthOnly( CommandList* cmdList ) const; // Reversed Z rendering (e.g. depth prepass)
  
    // Regular Z rendering (e.g. shadow rendering, hi-z pyramid compute, etc.)
    void                        bindDepthOnly( CommandList* cmdList ) const;
    void                        bindForProbeRendering( CommandList* cmdList ) const;

    const uint32_t              getMaterialSortKey() const;
    const bool                  isOpaque() const;

    const fnString_t&           getName() const;

#if FLAN_DEVBUILD
    void                        drawInEditor( RenderDevice* renderDevice, ShaderStageManager* shaderStageManager, GraphicsAssetManager* graphicsAssetManager, WorldRenderer* worldRenderer );
#endif

    void setHeightmapTEST( Texture* tex ) { vertexTextureSet[0] = tex; editableMaterialData.layers[0].Heightmap.InputType = MaterialEditionInput::TEXTURE;  editableMaterialData.layers[0].Heightmap.InputTexture = tex; }

private:
    fnString_t  name;
    bool        isEditable;
    bool        scaleUVByModelScale;
    bool        snapToHeightmap;
    int         builderVersion;

    union {
        struct {
            bool    isDoubleFace : 1;
            bool    isAlphaTested : 1;
            bool    isAlphaBlended : 1;
            bool    writeVelocity : 1;
            bool    scaleUVByModelScale : 1;
            bool    useAlphaToCoverage : 1;
            bool    receiveShadow : 1;
            bool    castShadow : 1;

            flan::graphics::eShadingModel    shadingModel : 2;
            bool    useRefraction : 1;
            bool    useTranslucidity : 1;

            uint32_t : 0;
        } sortKeyInfos;
        uint32_t    sortKey;
    };

    flan::graphics::eShadingModel           shadingModel;

    fnTextureSet_t   pixelTextureSet;
    fnTextureSet_t   vertexTextureSet;

    std::unique_ptr<PipelineState>          pipelineState;
    std::unique_ptr<PipelineState>          depthPipelineState;
    std::unique_ptr<PipelineState>          reversedDepthPipelineState;
    std::unique_ptr<PipelineState>          pipelineStateProbe;
    std::unique_ptr<PipelineState>          pipelineStateVirtualFeedback;

    // TODO Editor specific stuff; move to editor project
    struct {
        // Per Material Input
        uint32_t WriteVelocity;
        uint32_t EnableAlphaTest;
        uint32_t EnableAlphaBlend;
        uint32_t IsDoubleFace;

        uint32_t CastShadow;
        uint32_t ReceiveShadow;
        uint32_t AlphaToCoverage;
        uint32_t LayerCount;

        MaterialLayer layers[MAX_LAYER_COUNT];
    } editableMaterialData;
    FLAN_IS_MEMORY_ALIGNED( 16, Material::editableMaterialData );

    std::unique_ptr<Buffer> editableMaterialBuffer;

    int rebuildSpecularAAMaps[MAX_LAYER_COUNT];
    RenderTarget* roughnessMapRT[MAX_LAYER_COUNT];

private:
    void displayInputConfiguration( GraphicsAssetManager* graphicsAssetManager, const std::string& displayName, MaterialEditionInput& input, const uint32_t inputTextureBindIndex, fnTextureSet_t& textureSet, const bool saturate = true );
};
