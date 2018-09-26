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

static constexpr GLenum GL_IMAGE_FORMAT_SAMPLING_MASK[IMAGE_FORMAT_COUNT] =
{
    0x0000,

    0x0000,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    0x0000,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGBA,
    GL_RGBA,
    0x0000,
    GL_RGBA,
    GL_RGBA,
    0x0000,
    0x0000,
    GL_RG,
    GL_RG,
    GL_RG,
    0x0000,
    GL_DEPTH_STENCIL,
    0x0000,
    0x0000,
    GL_RGBA,
    0x0000,
    GL_RGBA,
    GL_RGB,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    0x0000,
    GL_RG,
    0x0000,
    GL_RG,
    GL_RG,
    GL_RG,
    GL_DEPTH_COMPONENT,
    GL_DEPTH_COMPONENT,
    GL_RED,
    GL_RED,
    GL_RED,

    GL_DEPTH_STENCIL,
    0x0000,
    0x0000,
    0x0000,

    GL_RG,
    0x0000,
    GL_RG,
    GL_RG,
    GL_RG,

    GL_RED,
    GL_RED,

    GL_DEPTH_COMPONENT,
    0x0000,
    GL_RED,
    GL_RED,
    GL_RED,

    GL_RED,
    0x0000,
    GL_RED,
    GL_RED,
    GL_RED,

    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,

    GL_RGB,
    GL_RGB,
    GL_RGB,

    0x0000,
    0x0000,
    0x0000,

    GL_RGBA,
    GL_RGBA,
    GL_RGBA,

    0x0000,
    0x0000,
    0x0000,

    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
};
#endif
