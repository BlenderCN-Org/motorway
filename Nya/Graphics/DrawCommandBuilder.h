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

class RenderDevice;
class WorldRenderer;
class Mesh;
class VertexArrayObject;

struct CameraData;

namespace glm
{
    enum qualifier;
    template<int i, int j, typename T, qualifier Q>
    struct mat;

    typedef mat<4, 4, float, ( qualifier )0>    mat4;
}

class DrawCommandBuilder
{
public:
                        DrawCommandBuilder( BaseAllocator* allocator );
                        DrawCommandBuilder( DrawCommandBuilder& ) = delete;
                        DrawCommandBuilder& operator = ( DrawCommandBuilder& ) = delete;
                        ~DrawCommandBuilder();
        
    void                addGeometryToRender( const Mesh* meshResource, const glm::mat4* modelMatrix );
    void                addCamera( const CameraData* cameraData );

    void                buildRenderQueues( WorldRenderer* worldRenderer );

private:
    struct MeshInstance {
        const Mesh*         mesh;
        const glm::mat4*    modelMatrix;
    };

private:
    const CameraData**  cameras;
    MeshInstance*       meshes;

    uint32_t            cameraCount;
    uint32_t            meshCount;

private:
    void                resetEntityCounters();
};
