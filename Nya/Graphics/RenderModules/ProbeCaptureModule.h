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

#include <Maths/Vector.h>

struct RenderTarget;
struct FontDescriptor;
struct PipelineState;
struct Buffer;

class RenderPipeline;
class RenderDevice;
class GraphicsAssetCache;
class ShaderCache;

using ResHandle_t = uint32_t;

class ProbeCaptureModule
{
public:
                                ProbeCaptureModule();
                                ProbeCaptureModule( ProbeCaptureModule& ) = delete;
                                ~ProbeCaptureModule();

    void                        destroy( RenderDevice* renderDevice );
    void                        loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache );

    void                        importResourcesToPipeline( RenderPipeline* renderPipeline );
    void                        convoluteProbeFace( RenderPipeline* renderPipeline, const int32_t probeArrayIndex, const uint16_t probeCaptureStep, const int32_t mipLevel = 0 );
    void                        saveCapturedProbeFace( RenderPipeline* renderPipeline, ResHandle_t capturedFace, const int32_t probeArrayIndex, const int16_t probeCaptureStep );
    
private:
    RenderTarget*               capturedProbesArray;
    RenderTarget*               diffuseProbesArray;
    RenderTarget*               specularProbesArray;

    PipelineState*              copyPso;
    PipelineState*              convolutionPso;
};
