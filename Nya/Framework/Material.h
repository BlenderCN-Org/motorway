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
class ShaderCache;
class CommandList;
class FileSystemObject;
class GraphicsAssetCache;

struct PipelineState;
struct ResourceList;
struct RenderPass;

#include <Core/LazyEnum.h>
#include <Shaders/MaterialShared.h>

namespace nya
{
    namespace graphics
    {
#define ShadingModel( model )\
        model( SHADING_MODEL_NONE )\
        model( SHADING_MODEL_STANDARD )\
        model( SHADING_MODEL_HUD_STANDARD )\
        model( SHADING_MODEL_EMISSIVE )\
        model( SHADING_MODEL_CLEAR_COAT )

        NYA_LAZY_ENUM( ShadingModel );
#undef ShadingModel
    }
}

class Material
{
public:
    struct EditorBuffer {
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
    };

public:
                                Material( const nyaString_t& matName = NYA_STRING( "Material" ) );
                                ~Material();

    void                        create( RenderDevice* renderDevice, ShaderCache* shaderCache );
    void                        destroy( RenderDevice* renderDevice );

    void                        load( FileSystemObject* stream, GraphicsAssetCache* graphicsAssetCache );

    uint32_t                    getSortKey() const;
    bool                        isOpaque() const;

    nyaStringHash_t             getHashcode() const;

    void                        setName( const nyaString_t& meshName );
    const nyaString_t&          getName() const;

    void                        bind( CommandList* cmdList, RenderPass& renderPass, ResourceList& resourceList ) const;
    void                        bindProbeCapture( CommandList* cmdList, RenderPass& renderPass, ResourceList& resourceList ) const;
    void                        bindDepthOnly( CommandList* cmdList, RenderPass& renderPass, ResourceList& resourceList ) const;

    // NOTE -This function should only be used in case of override/debug materials
    //      -This function implicitly drops regular flagset (based on sortkey infos)
    //      -Flagset should be formated like a regular permutation flag (e.g.: "+FLAG_1+FLAG_2")
    void                        setCustomFlagset( const std::string& customFlagset );

#if NYA_DEVBUILD
    const EditorBuffer&         getEditorBuffer() const;
#endif

private:
    nyaString_t                 name;
    int32_t                     builderVersion;

    std::string                 stageCustomFlagset;

    PipelineState*              defaultPipelineState;
    PipelineState*              probeCapturePipelineState;
    Texture*                    defaultTextureSet[12];
    int32_t                     defaultTextureSetCount;

    PipelineState*              depthOnlyPipelineState;
    Texture*                    depthOnlyTextureSet[3];
    int32_t                     depthOnlyTextureSetCount;

    EditorBuffer                editableMaterialData;

    union {
        struct {
            bool                            isDoubleFace : 1;
            bool                            isAlphaTested : 1;
            bool                            isAlphaBlended : 1;
            bool                            writeVelocity : 1;
            bool                            scaleUVByModelScale : 1;
            bool                            useAlphaToCoverage : 1;
            bool                            receiveShadow : 1;
            bool                            castShadow : 1;

            nya::graphics::eShadingModel    shadingModel : 2;
            bool                            useRefraction : 1;
            bool                            useTranslucidity : 1;
            bool                            useLodAlphaBlending : 1;

            uint32_t : 0;
        } sortKeyInfos;

        uint32_t                sortKey;
    };

private:
    void                        bindDefaultTextureSet( ResourceList& resourceList ) const;
    void                        getShadingModelResources( GraphicsAssetCache* graphicsAssetCache );
};
