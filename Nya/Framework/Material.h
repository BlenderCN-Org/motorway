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

struct PipelineState;
struct ResourceList;

#include <Core/LazyEnum.h>

namespace nya
{
    namespace graphics
    {
#define ShadingModel( model )\
        model( SHADING_MODEL_STANDARD )\
        model( SHADING_MODEL_HUD_STANDARD )\
        model( SHADING_MODEL_EMISSIVE )\
        model( SHADING_MODEL_CLEAR_COAT )\
        model( SHADING_MODEL_TERRAIN_STANDARD )

        NYA_LAZY_ENUM( ShadingModel );
#undef ShadingModel
    }
}

class Material
{
public:
                                Material( const nyaString_t& matName = NYA_STRING( "Material" ) );
                                Material( Material& material ) = default;
                                Material& operator = ( Material& material ) = default;
                                ~Material();

    void                        create( RenderDevice* renderDevice, ShaderCache* shaderCache );
    void                        destroy( RenderDevice* renderDevice );

    const uint32_t              getSortKey() const;
    const bool                  isOpaque() const;

    nyaStringHash_t             getHashcode() const;

    void                        setName( const nyaString_t& meshName );
    const nyaString_t&          getName() const;

    void                        bind( CommandList* cmdList ) const;

private:
    nyaString_t                 name;

    PipelineState*              defaultPipelineState;

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

            uint32_t : 0;
        } sortKeyInfos;

        uint32_t                sortKey;
    };
};
