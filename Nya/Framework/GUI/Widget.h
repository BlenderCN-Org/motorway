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

class DrawCommandBuilder;

#include <Maths/Vector.h>

class GUIWidget
{
public:
    nyaVec2f        RelativePosition;
    nyaVec2f        Position;
    nyaVec2f        Size;

public:
                    GUIWidget();
                    GUIWidget( GUIWidget& widget ) = default;
                    GUIWidget& operator = ( GUIWidget& widget ) = default;
                    ~GUIWidget();

    virtual void    collectDrawCmds( DrawCommandBuilder& drawCmdBuilder ) = 0;
};
