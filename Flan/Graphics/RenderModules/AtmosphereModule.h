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

#include <Rendering/RenderDevice.h>
#include <Rendering/PipelineState.h>
#include <Graphics/GraphicsAssetManager.h>
#include <Graphics/RenderPipeline.h>

class AtmosphereModule
{
public:
    struct Parameters
    {
        glm::vec3               EarthCenter;
        float                   SunSizeX;
        glm::vec3               SunDirection;
        float                   SunSizeY;
    };

public:
                                AtmosphereModule();
                                AtmosphereModule( AtmosphereModule& ) = delete;
                                ~AtmosphereModule();

    fnPipelineMutableResHandle_t renderAtmosphere( RenderPipeline* renderPipeline, bool renderSunDisk = true );
    void                         loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager );

private:
    Texture*                    transmittanceTexture;
    Texture*                    scatteringTexture;
    Texture*                    irradianceTexture;

    float                       sunVerticalAngle;
    float                       sunHorizontalAngle;
    float                       sunAngularRadius;

    Parameters                  parameters;
};
