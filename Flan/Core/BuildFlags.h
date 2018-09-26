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

#if defined( _M_X64 ) || defined( __amd64 ) || defined( __amd64__ )
#define FLAN_X64
#define FLAN_BUILD_PROC "x64"
#elif defined( _M_IX86 )
#define FLAN_X86
#define FLAN_BUILD_PROC "x86"
#else
#define FLAN_BUILD_PROC
#endif

#if defined( FLAN_X64 ) || defined( FLAN_X86 )
#define FLAN_SSE42 1
#define FLAN_SIMD_LIB "SSE_4.2"
#elif defined( FLAN_ARM )
#define FLAN_NEON 1
#define FLAN_SIMD_LIB "ARM_NEON"
#endif

#if defined( _DEBUG ) || defined( DEBUG ) || !defined( NDEBUG )
#define FLAN_DEBUG
#define FLAN_BUILD_TYPE "DEBUG"
#else
#define FLAN_RELEASE
#define FLAN_BUILD_TYPE "RELEASE"
#endif

#if FLAN_WIN
#define FLAN_BUILD_PLATFORM "WINDOWS"
#elif FLAN_UNIX
#define FLAN_BUILD_PLATFORM "UNIX"
#else
#define FLAN_BUILD_PLATFORM "UNKNOWN"
#endif

#if FLAN_D3D11
#define FLAN_GFX_BACKEND "D3D11"
#elif FLAN_GL460
#define FLAN_GFX_BACKEND "GL460"
#elif FLAN_VULKAN
#define FLAN_GFX_BACKEND "VULKAN"
#else
#define FLAN_GFX_BACKEND "SOFTWARE"
#endif

#if FLAN_OPENAL
#define FLAN_AUDIO_BACKEND "OpenAL"
#elif FLAN_XAUDIO2
#define FLAN_AUDIO_BACKEND "XAudio2"
#endif

#if FLAN_DEVBUILD
#define FLAN_VERSION_TYPE "DEV_BUILD"
#else
#define FLAN_VERSION_TYPE "PROD_BUILD"
#endif

#define FLAN_VERSION "0.010-" FLAN_VERSION_TYPE
#define FLAN_BUILD FLAN_BUILD_PLATFORM "-" FLAN_BUILD_PROC "-" FLAN_GFX_BACKEND "-" FLAN_AUDIO_BACKEND "-"\
                    FLAN_BUILD_TYPE "-" FLAN_VERSION
#define FLAN_BUILD_DATE __DATE__ " " __TIME__
