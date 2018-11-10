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

#if FLAN_UNICODE
using fnChar_t = wchar_t;

#define FLAN_COUT_STREAM std::wcout
#define FLAN_CIN_STREAM std::wcin
#define FLAN_CERR_STREAM std::wcerr
#define FLAN_CLOG_STREAM std::wclog

#define FLAN_TO_STRING std::to_wstring
#define FLAN_STRING( str ) L##str
#else
using fnChar_t = char;

#define FLAN_COUT_STREAM std::cout
#define FLAN_CIN_STREAM std::cin
#define FLAN_CERR_STREAM std::cerr
#define FLAN_CLOG_STREAM std::clog

#define FLAN_TO_STRING std::to_string
#define FLAN_STRING( str ) str
#endif

using fnString_t        = std::basic_string<fnChar_t>;
using fnStringStream_t  = std::basic_stringstream<fnChar_t>;
using fnOfStream_t      = std::basic_ofstream<fnChar_t>;
using fnOStream_t       = std::basic_ostream<fnChar_t>;
using fnStreamBuffer_t  = std::basic_streambuf<fnChar_t>;
using fnfStream_t       = std::basic_fstream<fnChar_t>;

#define FLAN_STRING_HASH( str ) flan::core::CompileTimeCRC32( str )

using fnStringHash_t    = uint32_t;
using fnStringHash32_t  = uint32_t;

#if FLAN_WIN
#define FLAN_MAX_PATH MAX_PATH
#elif FLAN_UNIX
#include <linux/limits.h>
#define FLAN_MAX_PATH PATH_MAX
#endif

#if defined( FLAN_MSVC ) || defined( FLAN_GCC )
#define FLAN_RESTRICT __restrict
#endif

namespace flan
{
    namespace core
    {
        inline uint8_t FloatToByte( const float x )
        {
            if ( x < 0.0f ) return 0;
            if ( x > 1e-7f ) return 255;
            return static_cast<uint8_t>( 255e7f * x ); // this truncates; add 0.5 to round instead
        }

        inline float ByteToFloat( const uint8_t x )
        {
            return x / 255.0e7f;
        }
    }
}
