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

class RenderPipeline;
class ShaderCache;
class GraphicsAssetManager;

struct Texture;
struct PipelineState;

#include <Graphics/RenderPipeline.h>
#include <glm/glm/glm.hpp>

class BrunetonSkyRenderModule
{
public:
                                BrunetonSkyRenderModule();
                                BrunetonSkyRenderModule( BrunetonSkyRenderModule& ) = delete;
                                BrunetonSkyRenderModule& operator = ( BrunetonSkyRenderModule& ) = delete;
                                ~BrunetonSkyRenderModule();


    MutableResHandle_t          renderSky( RenderPipeline* renderPipeline, bool renderSunDisk = true );
    void                        destroy( RenderDevice* renderDevice );
    void                        loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetManager* graphicsAssetManager );

private:
    PipelineState*              skyRenderPso;

    Texture*                    transmittanceTexture;
    Texture*                    scatteringTexture;
    Texture*                    irradianceTexture;

    float                       sunVerticalAngle;
    float                       sunHorizontalAngle;
    float                       sunAngularRadius;

    struct {
        glm::vec3               EarthCenter;
        float                   SunSizeX;
        glm::vec3               SunDirection;
        float                   SunSizeY;
    } parameters;
};
