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
#include <Shared.h>
#include "BlueNoise.h"

#include "XorShift.h"

constexpr float sigma_i = 2.1f;
constexpr float sigma_s = 1.0f;

float ComputeTexelEnergy( const std::vector<float>& input, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height )
{
    float sum[1024] = { 0.0f };

    for ( int oy = -7; oy <= 7; ++oy ) {
        for ( int ox = -7; ox <= 7; ++ox ) {
            int sx = x + ox;
            if ( sx < 0 )
                sx += width;
            if ( sx >= static_cast<int32_t>( width ) )
                sx -= width;

            int sy = y + oy;
            if ( sy < 0 )
                sy += height;
            if ( sy >= static_cast<int32_t>( height ) )
                sy -= height;

            float dx = abs( (float)x - ( float )sx );
            if ( dx > width / 2 )
                dx = width - dx;

            float dy = abs( ( float )y - ( float )sy );
            if ( dy > height / 2 )
                dy = height - dy;
            const float a =
                ( dx * dx + dy * dy ) / ( sigma_i * sigma_i );

            const float b = sqrt( abs( input.at( x * width + y ) - input.at( sx * width + sy ) ) ) / ( sigma_s * sigma_s );

            sum[sy] += exp( -a - b );
        }
    }

    float total = 0;
    for ( unsigned int sy = 0; sy < height; ++sy )
        total += sum[sy];

    return total;
}

inline void TexelSwap( std::vector<float>& texels, const uint32_t i1, const uint32_t i2 )
{
    const auto tmp = texels.at( i1 );
    texels.at( i1 ) = texels.at( i2 );
    texels.at( i2 ) = tmp;
}

void flan::core::ComputeBlueNoise( const uint32_t width, const uint32_t height, std::vector<float>& result )
{
    result.resize( width * height );

    // Compute white noise
    for ( uint32_t texelIdx = 0; texelIdx < ( width * height ); texelIdx++ ) {
        result[texelIdx] = XorShiftNext01();
    }

    // Compute energy before figuring out distances between points
    {
        float** energyPtr = new float*[width];
        for ( uint32_t x = 0; x < width; x++ ) {
            energyPtr[x] = new float[height] { 0.0f };
        }

        for ( uint32_t y = 0; y < height; y++ ) {
            for ( uint32_t x = 0; x < width; x++ ) {
                energyPtr[y][x] = ComputeTexelEnergy( result, x, y, width, height );
            }
        }

        // random walk
        const int kMaxIteration = 10000;

        for ( int ite = 0; ite < kMaxIteration; ++ite ) {
            float current_energy = 0;

            for ( uint32_t y = 0; y < height; y++ ) {
                for ( uint32_t x = 0; x < width; x++ ) {
                    current_energy += energyPtr[y][x];
                }
            }

            const int px = XorShiftNext() % width;
            const int py = XorShiftNext() % height;
            const int qx = XorShiftNext() % width;
            const int qy = XorShiftNext() % height;

            float next_energy = current_energy;
            next_energy -= energyPtr[py][px];
            next_energy -= energyPtr[qy][qx];

            TexelSwap( result, px* width + py, qx * width + qy );

            const float e0 = ComputeTexelEnergy( result, px, py, width, height );
            const float e1 = ComputeTexelEnergy( result, qx, qy, width, height );

            next_energy += ( e0 + e1 );

            if ( next_energy < current_energy ) {
                energyPtr[py][px] = e0;
                energyPtr[qy][qx] = e1;
                continue;
            }

            TexelSwap( result, px* width + py, qx * width + qy );
        }
    }
}
