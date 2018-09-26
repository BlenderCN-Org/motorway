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

#include "ComparisonFunctions.h"
#include "StencilOperation.h"

class CommandList;
class RenderDevice;

struct NativeDepthStencilStateObject;

struct DepthStencilStateDesc
{
    DepthStencilStateDesc()
        : enableDepthTest( true )
        , enableStencilTest( false )
        , enableDepthWrite( true )
        , enableDepthBoundsTest( false )
        , depthBoundsMin( 0.0f )
        , depthBoundsMax( 1.0f )
        , depthComparisonFunc( flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_ALWAYS )
        , stencilRefValue( 0x01 )
        , stencilReadMask( 0xFF )
        , stencilWriteMask( 0xFF )
    {
        front.comparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_ALWAYS;
        front.passOp = flan::rendering::eStencilOperation::STENCIL_OPERATION_REPLACE;
        front.failOp = flan::rendering::eStencilOperation::STENCIL_OPERATION_KEEP;
        front.zFailOp = flan::rendering::eStencilOperation::STENCIL_OPERATION_KEEP;

        back.comparisonFunc = flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_ALWAYS;
        back.passOp = flan::rendering::eStencilOperation::STENCIL_OPERATION_REPLACE;
        back.failOp = flan::rendering::eStencilOperation::STENCIL_OPERATION_KEEP;
        back.zFailOp = flan::rendering::eStencilOperation::STENCIL_OPERATION_KEEP;
    }

    bool                                    enableDepthTest;
    bool                                    enableStencilTest;
    bool                                    enableDepthWrite;
    bool                                    enableDepthBoundsTest;
    float                                   depthBoundsMin;
    float                                   depthBoundsMax;
    flan::rendering::eComparisonFunction    depthComparisonFunc;
    uint8_t                                 stencilRefValue;
    uint8_t                                 stencilWriteMask;
    uint8_t                                 stencilReadMask;

    struct {
        flan::rendering::eComparisonFunction    comparisonFunc;
        flan::rendering::eStencilOperation      passOp;
        flan::rendering::eStencilOperation      failOp;
        flan::rendering::eStencilOperation      zFailOp;
    } front, back;

    const bool operator == ( const DepthStencilStateDesc& desc ) const
    {
        return enableDepthTest == desc.enableDepthTest
            && enableStencilTest == desc.enableStencilTest
            && enableDepthWrite == desc.enableDepthWrite
            && enableDepthBoundsTest == desc.enableDepthBoundsTest
            && depthBoundsMin == desc.depthBoundsMin
            && depthBoundsMax == desc.depthBoundsMax
            && depthComparisonFunc == desc.depthComparisonFunc
            && stencilRefValue == desc.stencilRefValue
            && stencilWriteMask == desc.stencilWriteMask
            && stencilReadMask == desc.stencilReadMask
            && front.comparisonFunc == desc.front.comparisonFunc
            && front.passOp == desc.front.passOp
            && front.failOp == desc.front.failOp
            && front.zFailOp == desc.front.zFailOp
            && back.comparisonFunc == desc.back.comparisonFunc
            && back.passOp == desc.back.passOp
            && back.failOp == desc.back.failOp
            && back.zFailOp == desc.back.zFailOp;
    }
};

class DepthStencilState
{
public:
                DepthStencilState();
                DepthStencilState( DepthStencilState& ) = delete;
                DepthStencilState& operator = ( DepthStencilState& ) = delete;
                ~DepthStencilState();

    void        create( RenderDevice* renderDevice, const DepthStencilStateDesc& description );
    void        destroy( RenderDevice* renderDevice );

    void        bind( CommandList* cmdList );

    const DepthStencilStateDesc& getDescription() const;
    NativeDepthStencilStateObject* getNativeDepthStencilStateObject() const;

private:
    DepthStencilStateDesc                           depthStencilStateDesc;
    std::unique_ptr<NativeDepthStencilStateObject>  nativeDepthStencilStateObject;
};
