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

#include "Viewport.h"
#include "RenderTarget.h"
#include "BlendState.h"
#include "RasterizerState.h"

class DisplaySurface;
struct NativeRenderContext;
class PipelineState;

class RenderDevice
{
public:
                            RenderDevice();
                            RenderDevice( RenderDevice& ) = delete;
                            RenderDevice& operator = ( RenderDevice& ) = delete;
                            ~RenderDevice();

    NativeRenderContext*    getNativeRenderContext() const;

    void                    create( DisplaySurface* surface );
    void                    present();
    void                    setVSyncState( const bool enabled = false );

private:
    std::unique_ptr<NativeRenderContext> nativeRenderContext;

    RasterizerStateDesc::KeyValue   currentRasterizerState;
    BlendStateDesc::KeyValue        currentBlendState;
};
