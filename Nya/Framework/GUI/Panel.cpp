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
#include "Panel.h"

#include <Graphics/DrawCommandBuilder.h>

GUIPanel::GUIPanel()
    : GUIWidget()
    , IsDraggable( false )
    , IsResizable( false )
    , IsScrollable( false )
    , canBeDragged( false )
{
    mousePressedCoordinates = nyaVec2f( 0.0f, 0.0f );
}

GUIPanel::~GUIPanel()
{
    IsDraggable = false;
    IsResizable = false;
    IsScrollable = false;

    canBeDragged = false;

    mousePressedCoordinates = nyaVec2f( 0.0f, 0.0f );
}

void GUIPanel::onMouseButtonDown( const double mouseX, const double mouseY )
{
    if ( canBeDragged ) {
        return;
    }

    auto panelMin = Position - Size;
    auto panelMax = Position + Size;

    const bool isMouseInsideX = ( mouseX >= panelMin.x && mouseX <= panelMax.x );
    const bool isMouseInsideY = ( mouseY >= panelMin.y && mouseY <= panelMax.y );

    canBeDragged = ( isMouseInsideX && isMouseInsideY );
    mousePressedCoordinates = Position - nyaVec2f( mouseX, mouseY );
}

void GUIPanel::onMouseButtonUp()
{
    canBeDragged = false;
}

void GUIPanel::onMouseCoordinatesUpdate( const double mouseX, const double mouseY )
{
    if ( IsDraggable && canBeDragged ) {
        Position = mousePressedCoordinates + nyaVec2f( mouseX, mouseY );

        for ( GUIWidget* child : children ) {
            child->Position = Position - Size + ( child->RelativePosition * Size * 2.0f );
        }
    }
}

void GUIPanel::addChildren( GUIWidget* widget )
{
    widget->Position = Position - Size + ( widget->RelativePosition * Size * 2.0f );
    children.push_back( widget );
}

void GUIPanel::collectDrawCmds( DrawCommandBuilder& drawCmdBuilder )
{
    drawCmdBuilder.addHUDRectangle( Position, Size, 0.0f, PanelMaterial );

    for ( GUIWidget* child : children ) {
        child->collectDrawCmds( drawCmdBuilder );
    }
}
