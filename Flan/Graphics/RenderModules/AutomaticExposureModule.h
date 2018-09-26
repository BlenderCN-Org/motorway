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

struct AutoExposureBuffer
{
    Buffer* exposureBuffer;
};

class AutomaticExposureModule
{
public:
                                    AutomaticExposureModule();
                                    AutomaticExposureModule( AutomaticExposureModule& ) = delete;
                                    ~AutomaticExposureModule();

    fnPipelineMutableResHandle_t    addExposureComputePass( RenderPipeline* renderPipeline );
    void                            loadCachedResources( RenderDevice* renderDevice, GraphicsAssetManager* graphicsAssetManager );

private:
        Buffer              autoExposureBuffer[2];
        unsigned int        exposureTarget;
        AutoExposureBuffer  currentExposureBuffer;

        struct
        {
            float	            EngineLuminanceFactor;		// The actual factor to apply to values stored to the HDR render target (it's simply LuminanceFactor * WORLD_TO_BISOU_LUMINANCE so it's a division by about 100)
            float	            TargetLuminance;			// The target luminance to apply to the HDR luminance to bring it to the LDR luminance (warning: still in world units, you must multiply by WORLD_TO_BISOU_LUMINANCE for a valid engine factor)
            float	            MinLuminanceLDR;			// Minimum luminance (cd/m²) the screen will display as the value sRGB 1
            float	            MaxLuminanceLDR;			// Maximum luminance (cd/m²) the screen will display as the value sRGB 255
            float	            MiddleGreyLuminanceLDR;		// "Reference EV" luminance (cd/m²) the screen will display as the value sRGB 128 (55 linear)
            float	            EV;							// Absolute Exposure Value of middle grey (sRGB 128) from a reference luminance of 0.15 cd/m² (see above for an explanation on that magic value)
            float	            Fstop;						// The estimate F-stop number (overridden with env/autoexp/fstop_bias)
            unsigned int		PeakHistogramValue;			// The maximum value found in the browsed histogram (values at start and end of histogram are not accounted for based on start & end bucket indices
        } autoExposureInfos;
    
        struct
        {
            float	_delta_time;				// Clamped delta-time
            float	_white_level;				// (1.0) White level for tone mapping
            float	_clip_shadows;				// (0.0) Shadow cropping in histogram (first buckets will be ignored, leading to brighter image)
            float	_clip_highlights;			// (1.0) Highlights cropping in histogram (last buckets will be ignored, leading to darker image)
                   
            float	_EV;						// (0.0) Your typical EV setting
            float	_fstop_bias;				// (0.0) F-stop number bias to override automatic computation (NOTE: This will NOT change exposure, only the F number)
            float	_reference_camera_fps;		// (30.0) Default camera at 30 FPS
            float	_adapt_min_luminance;		// (0.03) Prevents the auto-exposure to adapt to luminances lower than this
                   
            float	_adapt_max_luminance;		// (2000.0) Prevents the auto-exposure to adapt to luminances higher than this
            float	_adapt_speed_up;			// (0.99) Adaptation speed from low to high luminances
            float	_adapt_speed_down;			// (0.99) Adaptation speed from high to low luminances
            int     __PADDING__;
        } parameters;

private:
    fnPipelineMutableResHandle_t addBinComputePass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t inputRenderTarget );
    fnPipelineMutableResHandle_t addHistogramMergePass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t perTileHistoBuffer );
    void                         autoExposureComputePass( RenderPipeline* renderPipeline, const fnPipelineMutableResHandle_t mergedHistoBuffer );
};
