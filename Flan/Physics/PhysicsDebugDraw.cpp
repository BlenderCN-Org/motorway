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

#include "Shared.h"

#if FLAN_DEVBUILD
#include "PhysicsDebugDraw.h"
#include "DynamicsWorld.h"

#include <Graphics/DrawCommandBuilder.h>

PhysicsDebugDraw::PhysicsDebugDraw()
    : activeDebugMode( btIDebugDraw::DBG_NoDebug )
{

}

PhysicsDebugDraw::~PhysicsDebugDraw()
{
    activeDebugMode = btIDebugDraw::DBG_NoDebug;
}

void PhysicsDebugDraw::create( DynamicsWorld* worldToDebug, DrawCommandBuilder* drawCommandBuilder )
{
    debuggedWorld = worldToDebug;
    drawCmdBuilder = drawCommandBuilder;

    activeDebugMode = btIDebugDraw::DBG_DrawWireframe;
    setDebugMode( activeDebugMode );

    // Provide debug draw implementation to Bullet native object
    worldToDebug->getNativeDynamicsWorld()->setDebugDrawer( this );
}

void PhysicsDebugDraw::onFrame()
{
    debuggedWorld->getNativeDynamicsWorld()->debugDrawWorld();
}

void PhysicsDebugDraw::reportErrorWarning( const char* warningString )
{
    FLAN_CWARN << warningString << std::endl;
}

void PhysicsDebugDraw::drawLine( const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor )
{
    drawCmdBuilder->addLineToRender( glm::vec3( from.getX(), from.getY(), from.getZ() ), glm::vec3( to.getX(), to.getY(), to.getZ() ), 1.0f, glm::vec4( fromColor.getX(), fromColor.getY(), fromColor.getZ(), 1.0f ) );
}

void PhysicsDebugDraw::drawLine( const btVector3& from, const btVector3& to, const btVector3& color )
{
    drawCmdBuilder->addLineToRender( glm::vec3( from.getX(), from.getY(), from.getZ() ), glm::vec3( to.getX(), to.getY(), to.getZ() ), 1.0f, glm::vec4( color.getX(), color.getY(), color.getZ(), 1.0f ) );
}

void PhysicsDebugDraw::drawSphere( const btVector3& p, btScalar radius, const btVector3& color )
{
    drawCmdBuilder->addWireframeSphere( glm::vec3( p.getX(), p.getY(), p.getZ() ), radius, glm::vec4( color.getX(), color.getY(), color.getZ(), 1.0f ) );
}

void PhysicsDebugDraw::drawTriangle( const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha )
{

}

void PhysicsDebugDraw::drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color )
{

}

void PhysicsDebugDraw::draw3dText( const btVector3& location, const char* textString )
{

}
#endif
