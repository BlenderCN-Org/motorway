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

#if FLAN_GL460
#include <Rendering/ImageFormat.h>

#include "Extensions.h"

static constexpr GLenum GL_IMAGE_FORMAT_TYPE[IMAGE_FORMAT_COUNT] =
{
    0x0000,

    0x0000,
    GL_FLOAT,
    GL_UNSIGNED_INT,
    GL_INT,
    0x0000,
    GL_FLOAT,
    GL_UNSIGNED_INT,
    GL_INT,
    GL_SHORT,
    GL_HALF_FLOAT,
    0x0000,
    GL_UNSIGNED_SHORT,
    GL_SHORT,
    0x0000,
    0x0000,
    GL_FLOAT,
    GL_UNSIGNED_INT,
    GL_INT,
    0x0000,
    GL_FLOAT,
    0x0000,
    0x0000,
    GL_SHORT,
    0x0000,
    GL_UNSIGNED_SHORT,
    GL_HALF_FLOAT,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_BYTE,
    GL_BYTE,
    GL_BYTE,
    0x0000,
    GL_HALF_FLOAT,
    0x0000,
    GL_UNSIGNED_SHORT,
    GL_SHORT,
    GL_SHORT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_UNSIGNED_INT,
    GL_INT,

    GL_INT,
    0x0000,
    0x0000,
    0x0000,

    GL_BYTE,
    0x0000,
    GL_UNSIGNED_BYTE,
    GL_BYTE,
    GL_BYTE,

    GL_SHORT,
    GL_HALF_FLOAT,

    GL_SHORT,
    0x0000,
    GL_UNSIGNED_SHORT,
    GL_SHORT,
    GL_SHORT,

    GL_BYTE,
    0x0000,
    GL_UNSIGNED_BYTE,
    GL_BYTE,
    GL_BYTE,

    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,

    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,

    0x0000,
    0x0000,
    0x0000,

    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,

    0x0000,
    0x0000,
    0x0000,

    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
};
#endif
