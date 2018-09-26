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

#include <Rendering/ImageFormat.h>

#if FLAN_GL460
#include "Extensions.h"

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT                  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT            0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT            0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT            0x8C4F

static constexpr GLenum GL_IMAGE_FORMAT[IMAGE_FORMAT_COUNT] =
{
    0x0000,

    0x0000,
    GL_RGBA32F,
    GL_RGBA32UI,
    GL_RGBA32I,
    0x0000,
    GL_RGB32F,
    GL_RGB32UI,
    GL_RGB32I,
    GL_RGBA16,
    GL_RGBA16F,
    0x0000,
    GL_RGBA16UI,
    GL_RGBA16_SNORM,
    0x0000,
    0x0000,
    GL_RG32F,
    GL_RG32UI,
    GL_RG32I,
    0x0000,
    GL_DEPTH32F_STENCIL8,
    0x0000,
    0x0000,
    GL_RGB10_A2,
    0x0000,
    GL_RGB10_A2UI,
    GL_R11F_G11F_B10F,

    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA8UI,
    GL_RGBA8_SNORM,
    GL_RGBA8I,

    0x0000,
    GL_RG16F,
    0x0000,
    GL_RG16UI,
    GL_RG16_SNORM,
    GL_RG16I,

    GL_DEPTH_COMPONENT,
    GL_DEPTH_COMPONENT32F,
    GL_R32F,
    GL_R32UI,
    GL_R32I,

    GL_DEPTH24_STENCIL8,
    0x0000,
    0x0000,
    0x0000,

    GL_RG8,
    0x0000,
    GL_RG8UI,
    GL_RG8_SNORM,
    GL_RG8I,

    GL_R16,
    GL_R16F,

    GL_DEPTH_COMPONENT16,
    0x0000,
    GL_R16UI,
    GL_R16_SNORM,
    GL_R16I,

    GL_R8,
    0x0000,
    GL_R8UI,
    GL_R8_SNORM,
    GL_R8I,

    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0000,

    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,

    0x0000,
    0x0000,
    0x0000,

    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,

    0x0000,
    0x0000,
    0x0000,

    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
};
#endif
