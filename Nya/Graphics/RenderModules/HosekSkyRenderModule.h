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
struct PipelineState;

#include <Graphics/RenderPipeline.h>
#include <glm/glm/glm.hpp>

class HosekSkyRenderModule
{
public:
                        HosekSkyRenderModule();
                        HosekSkyRenderModule( HosekSkyRenderModule& ) = delete;
                        HosekSkyRenderModule& operator = ( HosekSkyRenderModule& ) = delete;
                        ~HosekSkyRenderModule();

    MutableResHandle_t  renderSky( RenderPipeline* renderPipeline, bool renderSunDisk = true );
    
    void                recompute( const glm::vec3& worldSpaceSunDirection, float turbidity, float albedo, float normalizedSunY ) noexcept;
    void                destroy( RenderDevice* renderDevice );
    void                loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache );

private:
    struct {
        glm::vec3 A;
        uint32_t __PADDING1__;
        glm::vec3 B;
        uint32_t __PADDING2__;
        glm::vec3 C;
        uint32_t __PADDING3__;
        glm::vec3 D;
        uint32_t __PADDING4__;
        glm::vec3 E;
        uint32_t __PADDING5__;
        glm::vec3 F;
        uint32_t __PADDING6__;
        glm::vec3 G;
        uint32_t __PADDING7__;
        glm::vec3 H;
        uint32_t __PADDING8__;
        glm::vec3 I;
        uint32_t __PADDING9__;
        glm::vec3 Z;
        uint32_t __PADDING10__;

        glm::vec3 SunDirection;
        uint32_t __PADDING11__;
    } coefficients;

    glm::vec3       sunColor;
    PipelineState*  skyRenderPso;
};