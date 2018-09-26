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
#include "Frustum.h"

void flan::core::UpdateFrustumPlanes( const glm::mat4x4& viewProjectionMatrix, Frustum& frustum )
{
    frustum.planes[0] = glm::vec4( viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0],
                                viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0],
                                viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0],
                                viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0] );

    frustum.planes[1] = glm::vec4( viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0],
                                viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0],
                                viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0],
                                viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0] );

    frustum.planes[2] = glm::vec4( viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1],
                                viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1],
                                viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1],
                                viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1] );

    frustum.planes[3] = glm::vec4( viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1],
                                viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1],
                                viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1],
                                viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1] );

    // Near
    frustum.planes[5] = glm::vec4( viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2],
                                viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2],
                                viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2],
                                viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2] );

    // Far
    frustum.planes[4] = glm::vec4( viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2],
                                    viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2],
                                    viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2],
                                    viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2] );

    // Normalize them
    for ( int i = 0; i < 6; i++ ) {
        float invl = sqrt( frustum.planes[i].x * frustum.planes[i].x +
                           frustum.planes[i].y * frustum.planes[i].y +
                           frustum.planes[i].z * frustum.planes[i].z );

        frustum.planes[i] /= invl;
    }
}
