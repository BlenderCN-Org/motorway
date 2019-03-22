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

#if NYA_UNICODE
using nyaChar_t = wchar_t;

#define NYA_COUT_STREAM std::wcout
#define NYA_CIN_STREAM std::wcin
#define NYA_CERR_STREAM std::wcerr
#define NYA_CLOG_STREAM std::wclog

#define NYA_TO_STRING std::to_wstring
#define NYA_STRING( str ) L##str
#else
using nyaChar_t = char;

#define NYA_COUT_STREAM std::cout
#define NYA_CIN_STREAM std::cin
#define NYA_CERR_STREAM std::cerr
#define NYA_CLOG_STREAM std::clog

#define NYA_TO_STRING std::to_string
#define NYA_STRING( str ) str
#endif

using nyaString_t        = std::basic_string<nyaChar_t>;
using nyaStringStream_t  = std::basic_stringstream<nyaChar_t>;
using nyaOfStream_t      = std::basic_ofstream<nyaChar_t>;
using nyaOStream_t       = std::basic_ostream<nyaChar_t>;
using nyaStreamBuffer_t  = std::basic_streambuf<nyaChar_t>;
using nyafStream_t       = std::basic_fstream<nyaChar_t>;

#define NYA_STRING_HASH( str ) nya::core::CompileTimeCRC32( str )

using nyaStringHash_t    = std::uint32_t;
using nyaStringHash32_t  = std::uint32_t;

#if NYA_WIN
#define NYA_MAX_PATH MAX_PATH
#elif NYA_UNIX
#include <linux/limits.h>
#define NYA_MAX_PATH PATH_MAX
#endif

#if defined( NYA_MSVC ) || defined( NYA_GCC )
#define NYA_RESTRICT __restrict
#endif

#define NYA_UNUSED_VARIABLE( x ) (void)x
