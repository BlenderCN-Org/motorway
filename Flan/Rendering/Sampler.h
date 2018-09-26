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

#include <Rendering/ComparisonFunctions.h>
#include <Rendering/SamplerAddresses.h>
#include <Rendering/SamplerFilters.h>

class RenderDevice;
class CommandList;
struct NativeSamplerObject;

struct SamplerDesc
{
    SamplerDesc()
        : filter( flan::rendering::eSamplerFilter::SAMPLER_FILTER_BILINEAR )
        , addressU( flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP)
        , addressV( flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP )
        , addressW( flan::rendering::eSamplerAddress::SAMPLER_ADDRESS_WRAP )
        , comparisonFunction( flan::rendering::eComparisonFunction::COMPARISON_FUNCTION_ALWAYS )
        , minLOD( 0 )
        , maxLOD( 1000 )
    {

    }

    flan::rendering::eSamplerFilter         filter;
    flan::rendering::eSamplerAddress        addressU;
    flan::rendering::eSamplerAddress        addressV;
    flan::rendering::eSamplerAddress        addressW;
    flan::rendering::eComparisonFunction    comparisonFunction;
    int32_t                                 minLOD;
    int32_t                                 maxLOD;
};

class Sampler
{
public:
                Sampler();
                Sampler( Sampler& ) = delete;
                Sampler& operator = ( Sampler& ) = delete;
                ~Sampler();

    void        create( RenderDevice* renderDevice, const SamplerDesc& description );
    void        destroy( RenderDevice* renderDevice );

    void        bind( CommandList* cmdList, const uint32_t bindingIndex = 0 );

    const SamplerDesc* getDescription() const;

private:
    SamplerDesc samplerDescription;
    std::unique_ptr<NativeSamplerObject> nativeSamplerObject;
};
