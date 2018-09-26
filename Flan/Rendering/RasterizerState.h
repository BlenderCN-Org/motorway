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
struct NativeRasterizerStateObject;

#include "CullModes.h"
#include "FillModes.h"

struct RasterizerStateDesc
{
    using KeyValue = uint32_t;

    flan::rendering::eCullMode  cullMode;
    flan::rendering::eFillMode  fillMode;
    float                       depthBias;
    float                       slopeScale;
    float                       depthBiasClamp;
    bool                        useTriangleCCW;

    struct Key
    {
        uint8_t useTriangleCCW : 1;
        uint8_t cullMode : 2;
        uint8_t fillMode : 2;
        uint8_t useDepthBias : 1;
        uint8_t useSlopeScale : 1;
        uint8_t useDepthBiasClamp : 1;
        uint8_t depthBiasValue;
        uint8_t slopeScaleValue;
        uint8_t depthBiasClampValue;
    };

    union
    {
        Key         stateKey;
        KeyValue    stateKeyValue;
    };

    RasterizerStateDesc()
        : cullMode( flan::rendering::eCullMode::CULL_MODE_NONE )
        , fillMode( flan::rendering::eFillMode::FILL_MODE_SOLID )
        , depthBias( 0.0f )
        , slopeScale( 0.0f )
        , depthBiasClamp( 0.0f )
        , useTriangleCCW( true )
    {
        rebuildStateKey();
    }

    void rebuildStateKey()
    {
        stateKeyValue = 0;

        stateKey.useTriangleCCW = useTriangleCCW;
        stateKey.cullMode = cullMode;
        stateKey.fillMode = fillMode;
        stateKey.useDepthBias = ( depthBias != 0.0f );
        stateKey.useSlopeScale = ( slopeScale != 0.0f );
        stateKey.useDepthBiasClamp = ( depthBiasClamp != 0.0f );
        stateKey.depthBiasValue = flan::core::FloatToByte( depthBias );
        stateKey.slopeScaleValue = flan::core::FloatToByte( slopeScale );
        stateKey.depthBiasClampValue = flan::core::FloatToByte( depthBiasClamp );
    }

    const bool operator == ( const RasterizerStateDesc& desc ) const
    {
        return stateKeyValue == desc.stateKeyValue;
    }
};

class RasterizerState
{
public:
                RasterizerState();
                RasterizerState( RasterizerState& ) = delete;
                RasterizerState& operator = ( RasterizerState& ) = delete;
                ~RasterizerState();

    void        create( RenderDevice* renderDevice, const RasterizerStateDesc& description );
    void        destroy( RenderDevice* renderDevice );

    void        bind( CommandList* cmdList );

    const RasterizerStateDesc& getDescription() const;
    const RasterizerStateDesc::Key getKey() const;
    RasterizerStateDesc::KeyValue getKeyValue() const;

    NativeRasterizerStateObject* getNativeRasterizerStateObject() const;

private:
    RasterizerStateDesc                          rasterizerStateDescription;
    std::unique_ptr<NativeRasterizerStateObject> nativeRasterizerStateObject;
};
