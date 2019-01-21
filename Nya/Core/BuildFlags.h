/*
    Project Nya Source Code
    Copyright (C) 2018 Prévost Baptiste

    This file is part of Project Nya source code.

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

#if defined( _M_X64 ) || defined( __amd64 ) || defined( __amd64__ )
#define NYA_X64
#define NYA_BUILD_PROC "x64"
#elif defined( _M_IX86 )
#define NYA_X86
#define NYA_BUILD_PROC "x86"
#else
#define NYA_BUILD_PROC
#endif

#if defined( NYA_X64 ) || defined( NYA_X86 )
#define NYA_SSE42 1
#define NYA_SIMD_LIB "SSE_4.2"
#elif defined( NYA_ARM )
#define NYA_NEON 1
#define NYA_SIMD_LIB "ARM_NEON"
#endif

#if defined( _DEBUG ) || defined( DEBUG ) || !defined( NDEBUG )
#define NYA_DEBUG
#define NYA_BUILD_TYPE "DEBUG"
#else
#define NYA_RELEASE
#define NYA_BUILD_TYPE "RELEASE"
#endif

#if NYA_WIN
#define NYA_BUILD_PLATFORM "WINDOWS"
#elif NYA_UNIX
#define NYA_BUILD_PLATFORM "UNIX"
#else
#define NYA_BUILD_PLATFORM "UNKNOWN"
#endif

#if NYA_D3D11
#define NYA_GFX_BACKEND "D3D11"
#elif NYA_GL460
#define NYA_GFX_BACKEND "GL460"
#elif NYA_VULKAN
#define NYA_GFX_BACKEND "VULKAN"
#else
#define NYA_GFX_BACKEND "SOFTWARE"
#endif

#if NYA_DEVBUILD
#define NYA_VERSION_TYPE "DEV_BUILD"
#else
#define NYA_VERSION_TYPE "PROD_BUILD"
#endif

#define NYA_VERSION "0.01-" NYA_VERSION_TYPE
#define NYA_BUILD NYA_BUILD_PLATFORM "-" NYA_BUILD_PROC "-" NYA_GFX_BACKEND "-"\
                    NYA_BUILD_TYPE "-" NYA_VERSION
#define NYA_BUILD_DATE __DATE__ " " __TIME__
