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

#include "BlendSources.h"
#include "BlendOperations.h"

class RenderDevice;
class CommandList;

struct NativeBlendStateObject;

struct BlendStateDesc
{
    using KeyValue = uint64_t;

    BlendStateDesc()
        : writeMask{ true, true, true, true }
        , enableBlend( false )
        , useSeperateAlpha( false )
        , enableAlphaToCoverage( false )
        , sampleMask( ~0 )
    {
        blendConfColor.source = flan::rendering::eBlendSource::BLEND_SOURCE_ZERO;
        blendConfColor.dest = flan::rendering::eBlendSource::BLEND_SOURCE_ONE;
        blendConfColor.operation = flan::rendering::eBlendOperation::BLEND_OPERATION_ADD;

        blendConfAlpha.source = flan::rendering::eBlendSource::BLEND_SOURCE_ZERO;
        blendConfAlpha.dest = flan::rendering::eBlendSource::BLEND_SOURCE_ONE;
        blendConfAlpha.operation = flan::rendering::eBlendOperation::BLEND_OPERATION_ADD;

        rebuildStateKey();
    }

    bool        writeMask[4]; // R, G, B, A
    bool        enableBlend;
    bool        useSeperateAlpha;
    bool        enableAlphaToCoverage;
    uint32_t    sampleMask;

    struct {
        flan::rendering::eBlendSource       source;
        flan::rendering::eBlendSource       dest;
        flan::rendering::eBlendOperation    operation;
    } blendConfColor, blendConfAlpha;

    struct Key
    {
        uint8_t writeRed : 1;
        uint8_t writeGreen : 1;
        uint8_t writeBlue : 1;
        uint8_t writeAlpha : 1;
        uint8_t enableBlend : 1;
        uint8_t useSeperateAlpha : 1;
        uint8_t enableAlphaToCoverage : 1;
        uint8_t __PADDING__ : 1;

        uint8_t blendColorSrc : 4;
        uint8_t blendColorDest : 4;
        uint8_t blendColorOp : 4;
        uint8_t blendAlphaSrc : 4;
        uint8_t blendAlphaDest : 4;
        uint8_t blendAlphaOp : 4;
        uint32_t sampleMask;
    };

    union
    {
        Key         stateKey;
        KeyValue    stateKeyValue;
    };

    static_assert( sizeof( Key ) == sizeof( KeyValue ), "Bitfield/Value size mismatch!" );

    void rebuildStateKey()
    {
        stateKeyValue = 0;

        stateKey.writeRed = writeMask[0];
        stateKey.writeGreen = writeMask[1];
        stateKey.writeBlue = writeMask[2];
        stateKey.writeAlpha = writeMask[3];
        stateKey.enableBlend = enableBlend;
        stateKey.useSeperateAlpha = useSeperateAlpha;
        stateKey.enableAlphaToCoverage = enableAlphaToCoverage;
        stateKey.__PADDING__ = 0;
        
        stateKey.blendColorSrc = blendConfColor.source;
        stateKey.blendColorDest = blendConfColor.dest;
        stateKey.blendColorOp = blendConfColor.operation;

        stateKey.blendAlphaSrc = blendConfAlpha.source;
        stateKey.blendAlphaDest = blendConfAlpha.dest;
        stateKey.blendAlphaOp = blendConfAlpha.operation;

        stateKey.sampleMask = sampleMask;
    }

    const bool operator == ( const BlendStateDesc& desc ) const
    {
        return stateKeyValue == desc.stateKeyValue;
    }
};

class BlendState
{
public:
    BlendState();
    BlendState( BlendState& state );
    BlendState& operator = ( BlendState& state );
    ~BlendState();

    void        create( RenderDevice* renderDevice, const BlendStateDesc& description );
    void        destroy( RenderDevice* renderDevice );
    void        bind( CommandList* cmdList );

    const BlendStateDesc& getDescription() const;
    const BlendStateDesc::Key getKey() const;
    BlendStateDesc::KeyValue getKeyValue() const;

    NativeBlendStateObject* getNativeBlendStateObject() const;

private:
    BlendStateDesc                          blendStateDescription;
    std::unique_ptr<NativeBlendStateObject> nativeBlendStateObject;
};
