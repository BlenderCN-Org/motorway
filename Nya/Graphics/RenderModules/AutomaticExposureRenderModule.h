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
class GraphicsAssetCache;

struct Texture;
struct PipelineState;

#include <Graphics/RenderPipeline.h>

class AutomaticExposureModule
{
public:
                        AutomaticExposureModule();
                        AutomaticExposureModule( AutomaticExposureModule& ) = delete;
                        AutomaticExposureModule& operator = ( AutomaticExposureModule& ) = delete;
                        ~AutomaticExposureModule();

    ResHandle_t         computeExposure( RenderPipeline* renderPipeline, ResHandle_t lightRenderTarget, const nyaVec2f& screenSize );
    void                destroy( RenderDevice* renderDevice );
    void                loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache, GraphicsAssetCache* graphicsAssetCache );

private:
    Buffer*             autoExposureBuffer[2];
    unsigned int        exposureTarget;

    PipelineState*      binComputePso;
    PipelineState*      mergeHistoPso;
    PipelineState*      tileHistoComputePso;

    struct {
        float               EngineLuminanceFactor;
        float               TargetLuminance;
        float               MinLuminanceLDR;
        float               MaxLuminanceLDR;
        float               MiddleGreyLuminanceLDR;
        float               EV;
        float               Fstop;
        unsigned int        PeakHistogramValue;
    } autoExposureInfos;

    struct {
        float    _delta_time;
        float    _white_level;
        float    _clip_shadows;
        float    _clip_highlights;

        float    _EV;
        float    _fstop_bias;
        float    _reference_camera_fps;
        float    _adapt_min_luminance;

        float    _adapt_max_luminance;
        float    _adapt_speed_up;
        float    _adapt_speed_down;
        int     __PADDING__;
    } parameters;

private:
    MutableResHandle_t  addBinComputePass( RenderPipeline* renderPipeline, const ResHandle_t inputRenderTarget, const nyaVec2u& screenSize );
    MutableResHandle_t  addHistogramMergePass( RenderPipeline* renderPipeline, const ResHandle_t perTileHistoBuffer, const nyaVec2u& screenSize );
    ResHandle_t         addExposureComputePass( RenderPipeline* renderPipeline, const ResHandle_t mergedHistoBuffer );
};
