/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr�vost Baptiste

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

class Material;

#include "Widget.h"

#include <vector>

class GUIPanel : public GUIWidget
{
public:
    bool                    IsDraggable;
    bool                    IsResizable;
    bool                    IsScrollable;

    Material*               PanelMaterial;

public:
                            GUIPanel();
                            GUIPanel( GUIPanel& widget ) = default;
                            GUIPanel& operator = ( GUIPanel& widget ) = default;
                            ~GUIPanel();

    virtual void            onMouseButtonDown( const double mouseX, const double mouseY );
    virtual void            onMouseButtonUp();
    virtual void            onMouseCoordinatesUpdate( const double mouseX, const double mouseY );

    void                    addChild( GUIWidget* widget );
    void                    collectDrawCmds( DrawCommandBuilder& drawCmdBuilder ) override;
    void                    setScreenPosition( const nyaVec2f& screenSpacePosition ) override;

protected:
    bool                    isMouseInside;

private:
    nyaVec2f                mousePressedCoordinates;
    std::vector<GUIWidget*> children;
};
