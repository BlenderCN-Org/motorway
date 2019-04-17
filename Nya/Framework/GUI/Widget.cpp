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
#include "Widget.h"

GUIWidget::GUIWidget()
    : RelativePosition( 0.0f, 0.0f )
    , Position( 0.0f, 0.0f )
    , Size( 1.0f, 1.0f )
{

}

GUIWidget::~GUIWidget()
{
    RelativePosition = nyaVec2f( 0.0f, 0.0f );
    Position = nyaVec2f( 0.0f );
    Size = nyaVec2f( 0.0f );
}
