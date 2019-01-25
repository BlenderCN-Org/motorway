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

#include "Shared.h"
#include "Material.h"

#include <Graphics/ShaderCache.h>

#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

using namespace nya::graphics;

Material::Material( const nyaString_t& materialName )
    : name( materialName )
    , defaultPipelineState( nullptr )
    , sortKey( 0u )
{

}

Material::~Material()
{
    name.clear();
    defaultPipelineState = nullptr;
    sortKey = 0u;
}

void Material::create( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc defaultPipelineStateDesc = {};

    defaultPipelineState = renderDevice->createPipelineState( defaultPipelineStateDesc );
}

void Material::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( defaultPipelineState );
}

const uint32_t Material::getSortKey() const
{
    return sortKey;
}

const bool Material::isOpaque() const
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

void Material::bind( CommandList* cmdList ) const
{
    cmdList->bindPipelineState( defaultPipelineState );
}
