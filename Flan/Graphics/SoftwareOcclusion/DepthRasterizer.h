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

class DepthRasterizer
{
public:
            DepthRasterizer();
            DepthRasterizer( DepthRasterizer& ) = default;
            DepthRasterizer& operator = ( DepthRasterizer& ) = default;
            ~DepthRasterizer();

    void    setFramebufferSize( const uint32_t width, const uint32_t height );

private:
    static constexpr int BUFFER_COUNT = 2;

private:
    uint32_t fbWidth;
    uint32_t fbHeight;
    uint32_t fbTileWidth;
    uint32_t fbTileHeight;

    uint32_t fbTileCountWidth;
    uint32_t fbTileCountHeight;
    uint32_t fbTileCount;

    uint32_t yOffset1;
    uint32_t xOffset1;
    uint32_t yOffset2;
    uint32_t xOffset2;

    uint32_t* indicesBins[BUFFER_COUNT];
    uint16_t* meshIndices[BUFFER_COUNT];
    uint16_t* triangleCountInBins[BUFFER_COUNT];

    int8_t* depthBuffer[BUFFER_COUNT];
    glm::mat4x4 viewportMatrix;
};
