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

#if FLAN_DEVBUILD
#include <bullet3/src/LinearMath/btIDebugDraw.h>

class ShaderStageManager;
class DynamicsWorld;
class DrawCommandBuilder;

class PhysicsDebugDraw : public btIDebugDraw
{
public:
    virtual inline int  getDebugMode() const override { return activeDebugMode; }
    virtual inline void setDebugMode( int debugMode ) override { activeDebugMode = debugMode; }

public:
                        PhysicsDebugDraw();
                        PhysicsDebugDraw( PhysicsDebugDraw& ) = delete;
    virtual             ~PhysicsDebugDraw() override;

    void                create( DynamicsWorld* worldToDebug, DrawCommandBuilder* drawCommandBuilder );
    void                onFrame();

    virtual void        reportErrorWarning( const char* warningString ) override;
        
    virtual void        drawLine( const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor ) override;
    virtual void        drawLine( const btVector3& from, const btVector3& to, const btVector3& color ) override;
    virtual void        drawSphere( const btVector3& p, btScalar radius, const btVector3& color ) override;
    virtual void        drawTriangle( const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha ) override;
    virtual void        drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color ) override;
    virtual void        draw3dText( const btVector3& location, const char* textString ) override;

private:
    int                 activeDebugMode;
    DynamicsWorld*      debuggedWorld;
    DrawCommandBuilder* drawCmdBuilder;
};
#endif
