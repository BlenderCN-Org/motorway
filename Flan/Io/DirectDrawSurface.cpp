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
#include "DirectDrawSurface.h"

#include <FileSystem/FileSystemObject.h>

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push,1)

static constexpr uint32_t DDS_MAGIC = 0x20534444; // "DDS "

// Copy/Pasted from msdn for better abstraction
struct DDS_PIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};

typedef struct {
  uint32_t           dwSize;
  uint32_t           dwFlags;
  uint32_t           dwHeight;
  uint32_t           dwWidth;
  uint32_t           dwPitchOrLinearSize;
  uint32_t           dwDepth;
  uint32_t           dwMipMapCount;
  uint32_t           dwReserved1[11];
  DDS_PIXELFORMAT    ddspf;
  uint32_t           dwCaps;
  uint32_t           dwCaps2;
  uint32_t           dwCaps3;
  uint32_t           dwCaps4;
  uint32_t           dwReserved2;
} DDS_HEADER;

typedef enum D3D10_RESOURCE_DIMENSION {
  D3D10_RESOURCE_DIMENSION_UNKNOWN    = 0,
  D3D10_RESOURCE_DIMENSION_BUFFER     = 1,
  D3D10_RESOURCE_DIMENSION_TEXTURE1D  = 2,
  D3D10_RESOURCE_DIMENSION_TEXTURE2D  = 3,
  D3D10_RESOURCE_DIMENSION_TEXTURE3D  = 4
} D3D10_RESOURCE_DIMENSION;

typedef enum IMAGE_FORMAT {
  DXGI_DIMENSION_UNKNOWN                     = 0,
  DXGI_DIMENSION_R32G32B32A32_TYPELESS       = 1,
  DXGI_DIMENSION_R32G32B32A32_FLOAT          = 2,
  DXGI_DIMENSION_R32G32B32A32_UINT           = 3,
  DXGI_DIMENSION_R32G32B32A32_SINT           = 4,
  DXGI_DIMENSION_R32G32B32_TYPELESS          = 5,
  DXGI_DIMENSION_R32G32B32_FLOAT             = 6,
  DXGI_DIMENSION_R32G32B32_UINT              = 7,
  DXGI_DIMENSION_R32G32B32_SINT              = 8,
  DXGI_DIMENSION_R16G16B16A16_TYPELESS       = 9,
  DXGI_DIMENSION_R16G16B16A16_FLOAT          = 10,
  DXGI_DIMENSION_R16G16B16A16_UNORM          = 11,
  DXGI_DIMENSION_R16G16B16A16_UINT           = 12,
  DXGI_DIMENSION_R16G16B16A16_SNORM          = 13,
  DXGI_DIMENSION_R16G16B16A16_SINT           = 14,
  DXGI_DIMENSION_R32G32_TYPELESS             = 15,
  DXGI_DIMENSION_R32G32_FLOAT                = 16,
  DXGI_DIMENSION_R32G32_UINT                 = 17,
  DXGI_DIMENSION_R32G32_SINT                 = 18,
  DXGI_DIMENSION_R32G8X24_TYPELESS           = 19,
  DXGI_DIMENSION_D32_FLOAT_S8X24_UINT        = 20,
  DXGI_DIMENSION_R32_FLOAT_X8X24_TYPELESS    = 21,
  DXGI_DIMENSION_X32_TYPELESS_G8X24_UINT     = 22,
  DXGI_DIMENSION_R10G10B10A2_TYPELESS        = 23,
  DXGI_DIMENSION_R10G10B10A2_UNORM           = 24,
  DXGI_DIMENSION_R10G10B10A2_UINT            = 25,
  DXGI_DIMENSION_R11G11B10_FLOAT             = 26,
  DXGI_DIMENSION_R8G8B8A8_TYPELESS           = 27,
  DXGI_DIMENSION_R8G8B8A8_UNORM              = 28,
  DXGI_DIMENSION_R8G8B8A8_UNORM_SRGB         = 29,
  DXGI_DIMENSION_R8G8B8A8_UINT               = 30,
  DXGI_DIMENSION_R8G8B8A8_SNORM              = 31,
  DXGI_DIMENSION_R8G8B8A8_SINT               = 32,
  DXGI_DIMENSION_R16G16_TYPELESS             = 33,
  DXGI_DIMENSION_R16G16_FLOAT                = 34,
  DXGI_DIMENSION_R16G16_UNORM                = 35,
  DXGI_DIMENSION_R16G16_UINT                 = 36,
  DXGI_DIMENSION_R16G16_SNORM                = 37,
  DXGI_DIMENSION_R16G16_SINT                 = 38,
  DXGI_DIMENSION_R32_TYPELESS                = 39,
  DXGI_DIMENSION_D32_FLOAT                   = 40,
  DXGI_DIMENSION_R32_FLOAT                   = 41,
  DXGI_DIMENSION_R32_UINT                    = 42,
  DXGI_DIMENSION_R32_SINT                    = 43,
  DXGI_DIMENSION_R24G8_TYPELESS              = 44,
  DXGI_DIMENSION_D24_UNORM_S8_UINT           = 45,
  DXGI_DIMENSION_R24_UNORM_X8_TYPELESS       = 46,
  DXGI_DIMENSION_X24_TYPELESS_G8_UINT        = 47,
  DXGI_DIMENSION_R8G8_TYPELESS               = 48,
  DXGI_DIMENSION_R8G8_UNORM                  = 49,
  DXGI_DIMENSION_R8G8_UINT                   = 50,
  DXGI_DIMENSION_R8G8_SNORM                  = 51,
  DXGI_DIMENSION_R8G8_SINT                   = 52,
  DXGI_DIMENSION_R16_TYPELESS                = 53,
  DXGI_DIMENSION_R16_FLOAT                   = 54,
  DXGI_DIMENSION_D16_UNORM                   = 55,
  DXGI_DIMENSION_R16_UNORM                   = 56,
  DXGI_DIMENSION_R16_UINT                    = 57,
  DXGI_DIMENSION_R16_SNORM                   = 58,
  DXGI_DIMENSION_R16_SINT                    = 59,
  DXGI_DIMENSION_R8_TYPELESS                 = 60,
  DXGI_DIMENSION_R8_UNORM                    = 61,
  DXGI_DIMENSION_R8_UINT                     = 62,
  DXGI_DIMENSION_R8_SNORM                    = 63,
  DXGI_DIMENSION_R8_SINT                     = 64,
  DXGI_DIMENSION_A8_UNORM                    = 65,
  DXGI_DIMENSION_R1_UNORM                    = 66,
  DXGI_DIMENSION_R9G9B9E5_SHAREDEXP          = 67,
  DXGI_DIMENSION_R8G8_B8G8_UNORM             = 68,
  DXGI_DIMENSION_G8R8_G8B8_UNORM             = 69,
  DXGI_DIMENSION_BC1_TYPELESS                = 70,
  DXGI_DIMENSION_BC1_UNORM                   = 71,
  DXGI_DIMENSION_BC1_UNORM_SRGB              = 72,
  DXGI_DIMENSION_BC2_TYPELESS                = 73,
  DXGI_DIMENSION_BC2_UNORM                   = 74,
  DXGI_DIMENSION_BC2_UNORM_SRGB              = 75,
  DXGI_DIMENSION_BC3_TYPELESS                = 76,
  DXGI_DIMENSION_BC3_UNORM                   = 77,
  DXGI_DIMENSION_BC3_UNORM_SRGB              = 78,
  DXGI_DIMENSION_BC4_TYPELESS                = 79,
  DXGI_DIMENSION_BC4_UNORM                   = 80,
  DXGI_DIMENSION_BC4_SNORM                   = 81,
  DXGI_DIMENSION_BC5_TYPELESS                = 82,
  DXGI_DIMENSION_BC5_UNORM                   = 83,
  DXGI_DIMENSION_BC5_SNORM                   = 84,
  DXGI_DIMENSION_B5G6R5_UNORM                = 85,
  DXGI_DIMENSION_B5G5R5A1_UNORM              = 86,
  DXGI_DIMENSION_B8G8R8A8_UNORM              = 87,
  DXGI_DIMENSION_B8G8R8X8_UNORM              = 88,
  DXGI_DIMENSION_R10G10B10_XR_BIAS_A2_UNORM  = 89,
  DXGI_DIMENSION_B8G8R8A8_TYPELESS           = 90,
  DXGI_DIMENSION_B8G8R8A8_UNORM_SRGB         = 91,
  DXGI_DIMENSION_B8G8R8X8_TYPELESS           = 92,
  DXGI_DIMENSION_B8G8R8X8_UNORM_SRGB         = 93,
  DXGI_DIMENSION_BC6H_TYPELESS               = 94,
  DXGI_DIMENSION_BC6H_UF16                   = 95,
  DXGI_DIMENSION_BC6H_SF16                   = 96,
  DXGI_DIMENSION_BC7_TYPELESS                = 97,
  DXGI_DIMENSION_BC7_UNORM                   = 98,
  DXGI_DIMENSION_BC7_UNORM_SRGB              = 99,
  DXGI_DIMENSION_AYUV                        = 100,
  DXGI_DIMENSION_Y410                        = 101,
  DXGI_DIMENSION_Y416                        = 102,
  DXGI_DIMENSION_NV12                        = 103,
  DXGI_DIMENSION_P010                        = 104,
  DXGI_DIMENSION_P016                        = 105,
  DXGI_DIMENSION_420_OPAQUE                  = 106,
  DXGI_DIMENSION_YUY2                        = 107,
  DXGI_DIMENSION_Y210                        = 108,
  DXGI_DIMENSION_Y216                        = 109,
  DXGI_DIMENSION_NV11                        = 110,
  DXGI_DIMENSION_AI44                        = 111,
  DXGI_DIMENSION_IA44                        = 112,
  DXGI_DIMENSION_P8                          = 113,
  DXGI_DIMENSION_A8P8                        = 114,
  DXGI_DIMENSION_B4G4R4A4_UNORM              = 115,
  DXGI_DIMENSION_P208                        = 130,
  DXGI_DIMENSION_V208                        = 131,
  DXGI_DIMENSION_V408                        = 132,
  DXGI_DIMENSION_FORCE_UINT                  = 0xffffffff
} DXGI_DIMENSION;

typedef struct {
  DXGI_DIMENSION              dxgiFormat;
  D3D10_RESOURCE_DIMENSION resourceDimension;
  uint32_t                     miscFlag;
  uint32_t                     arraySize;
  uint32_t                     miscFlags2;
} DDS_HEADER_DXT10;

typedef enum D3D11_RESOURCE_MISC_FLAG {
  D3D11_RESOURCE_MISC_GENERATE_MIPS                    = 0x1L,
  D3D11_RESOURCE_MISC_SHARED                           = 0x2L,
  D3D11_RESOURCE_MISC_TEXTURECUBE                      = 0x4L,
  D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS                = 0x10L,
  D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS           = 0x20L,
  D3D11_RESOURCE_MISC_BUFFER_STRUCTURED                = 0x40L,
  D3D11_RESOURCE_MISC_RESOURCE_CLAMP                   = 0x80L,
  D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX                = 0x100L,
  D3D11_RESOURCE_MISC_GDI_COMPATIBLE                   = 0x200L,
  D3D11_RESOURCE_MISC_SHARED_NTHANDLE                  = 0x800L,
  D3D11_RESOURCE_MISC_RESTRICTED_CONTENT               = 0x1000L,
  D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE         = 0x2000L,
  D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER  = 0x4000L,
  D3D11_RESOURCE_MISC_GUARDED                          = 0x8000L,
  D3D11_RESOURCE_MISC_TILE_POOL                        = 0x20000L,
  D3D11_RESOURCE_MISC_TILED                            = 0x40000L,
  D3D11_RESOURCE_MISC_HW_PROTECTED                     = 0x80000L
} D3D11_RESOURCE_MISC_FLAG;

enum DDS_FLAGS
{
    DDS_FLAGS_NONE = 0x0,

    DDS_FLAGS_LEGACY_DWORD = 0x1,
    // Assume pitch is DWORD aligned instead of BYTE aligned (used by some legacy DDS files) 

    DDS_FLAGS_NO_LEGACY_EXPANSION = 0x2,
    // Do not implicitly convert legacy formats that result in larger pixel sizes (24 bpp, 3:3:2, A8L8, A4L4, P8, A8P8)  

    DDS_FLAGS_NO_R10B10G10A2_FIXUP = 0x4,
    // Do not use work-around for long-standing D3DX DDS file format issue which reversed the 10:10:10:2 color order masks 

    DDS_FLAGS_FORCE_RGB = 0x8,
    // Convert DXGI 1.1 BGR formats to DXGI_DIMENSION_R8G8B8A8_UNORM to avoid use of optional WDDM 1.1 formats 

    DDS_FLAGS_NO_16BPP = 0x10,
    // Conversions avoid use of 565, 5551, and 4444 formats and instead expand to 8888 to avoid use of optional WDDM 1.2 formats 

    DDS_FLAGS_EXPAND_LUMINANCE = 0x20,
    // When loading legacy luminance formats expand replicating the color channels rather than leaving them packed (L8, L16, A8L8) 

    DDS_FLAGS_FORCE_DX10_EXT = 0x10000,
    // Always use the 'DX10' header extension for DDS writer (i.e. don't try to write DX9 compatible DDS files) 

    DDS_FLAGS_FORCE_DX10_EXT_MISC2 = 0x20000,
    // DDS_FLAGS_FORCE_DX10_EXT including miscFlags2 information (result may not be compatible with D3DX10 or D3DX11) 
};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

static constexpr uint32_t DDS_CAPS = 0x00000001;
static constexpr uint32_t DDS_DEPTH = 0x00800000;

enum DDS_MISC_FLAGS2
{
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

#pragma pack(pop)

std::size_t BitsPerPixel( const DXGI_DIMENSION fmt )
{
    switch ( fmt ) {
    case DXGI_DIMENSION_R32G32B32A32_TYPELESS:
    case DXGI_DIMENSION_R32G32B32A32_FLOAT:
    case DXGI_DIMENSION_R32G32B32A32_UINT:
    case DXGI_DIMENSION_R32G32B32A32_SINT:
        return 128;

    case DXGI_DIMENSION_R32G32B32_TYPELESS:
    case DXGI_DIMENSION_R32G32B32_FLOAT:
    case DXGI_DIMENSION_R32G32B32_UINT:
    case DXGI_DIMENSION_R32G32B32_SINT:
        return 96;

    case DXGI_DIMENSION_R16G16B16A16_TYPELESS:
    case DXGI_DIMENSION_R16G16B16A16_FLOAT:
    case DXGI_DIMENSION_R16G16B16A16_UNORM:
    case DXGI_DIMENSION_R16G16B16A16_UINT:
    case DXGI_DIMENSION_R16G16B16A16_SNORM:
    case DXGI_DIMENSION_R16G16B16A16_SINT:
    case DXGI_DIMENSION_R32G32_TYPELESS:
    case DXGI_DIMENSION_R32G32_FLOAT:
    case DXGI_DIMENSION_R32G32_UINT:
    case DXGI_DIMENSION_R32G32_SINT:
    case DXGI_DIMENSION_R32G8X24_TYPELESS:
    case DXGI_DIMENSION_D32_FLOAT_S8X24_UINT:
    case DXGI_DIMENSION_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_DIMENSION_X32_TYPELESS_G8X24_UINT:
    case DXGI_DIMENSION_Y416:
    case DXGI_DIMENSION_Y210:
    case DXGI_DIMENSION_Y216:
        return 64;

    case DXGI_DIMENSION_R10G10B10A2_TYPELESS:
    case DXGI_DIMENSION_R10G10B10A2_UNORM:
    case DXGI_DIMENSION_R10G10B10A2_UINT:
    case DXGI_DIMENSION_R11G11B10_FLOAT:
    case DXGI_DIMENSION_R8G8B8A8_TYPELESS:
    case DXGI_DIMENSION_R8G8B8A8_UNORM:
    case DXGI_DIMENSION_R8G8B8A8_UNORM_SRGB:
    case DXGI_DIMENSION_R8G8B8A8_UINT:
    case DXGI_DIMENSION_R8G8B8A8_SNORM:
    case DXGI_DIMENSION_R8G8B8A8_SINT:
    case DXGI_DIMENSION_R16G16_TYPELESS:
    case DXGI_DIMENSION_R16G16_FLOAT:
    case DXGI_DIMENSION_R16G16_UNORM:
    case DXGI_DIMENSION_R16G16_UINT:
    case DXGI_DIMENSION_R16G16_SNORM:
    case DXGI_DIMENSION_R16G16_SINT:
    case DXGI_DIMENSION_R32_TYPELESS:
    case DXGI_DIMENSION_D32_FLOAT:
    case DXGI_DIMENSION_R32_FLOAT:
    case DXGI_DIMENSION_R32_UINT:
    case DXGI_DIMENSION_R32_SINT:
    case DXGI_DIMENSION_R24G8_TYPELESS:
    case DXGI_DIMENSION_D24_UNORM_S8_UINT:
    case DXGI_DIMENSION_R24_UNORM_X8_TYPELESS:
    case DXGI_DIMENSION_X24_TYPELESS_G8_UINT:
    case DXGI_DIMENSION_R9G9B9E5_SHAREDEXP:
    case DXGI_DIMENSION_R8G8_B8G8_UNORM:
    case DXGI_DIMENSION_G8R8_G8B8_UNORM:
    case DXGI_DIMENSION_B8G8R8A8_UNORM:
    case DXGI_DIMENSION_B8G8R8X8_UNORM:
    case DXGI_DIMENSION_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_DIMENSION_B8G8R8A8_TYPELESS:
    case DXGI_DIMENSION_B8G8R8A8_UNORM_SRGB:
    case DXGI_DIMENSION_B8G8R8X8_TYPELESS:
    case DXGI_DIMENSION_B8G8R8X8_UNORM_SRGB:
    case DXGI_DIMENSION_AYUV:
    case DXGI_DIMENSION_Y410:
    case DXGI_DIMENSION_YUY2:
        return 32;

    case DXGI_DIMENSION_P010:
    case DXGI_DIMENSION_P016:
        return 24;

    case DXGI_DIMENSION_R8G8_TYPELESS:
    case DXGI_DIMENSION_R8G8_UNORM:
    case DXGI_DIMENSION_R8G8_UINT:
    case DXGI_DIMENSION_R8G8_SNORM:
    case DXGI_DIMENSION_R8G8_SINT:
    case DXGI_DIMENSION_R16_TYPELESS:
    case DXGI_DIMENSION_R16_FLOAT:
    case DXGI_DIMENSION_D16_UNORM:
    case DXGI_DIMENSION_R16_UNORM:
    case DXGI_DIMENSION_R16_UINT:
    case DXGI_DIMENSION_R16_SNORM:
    case DXGI_DIMENSION_R16_SINT:
    case DXGI_DIMENSION_B5G6R5_UNORM:
    case DXGI_DIMENSION_B5G5R5A1_UNORM:
    case DXGI_DIMENSION_A8P8:
    case DXGI_DIMENSION_B4G4R4A4_UNORM:
        return 16;

    case DXGI_DIMENSION_NV12:
    case DXGI_DIMENSION_420_OPAQUE:
    case DXGI_DIMENSION_NV11:
        return 12;

    case DXGI_DIMENSION_R8_TYPELESS:
    case DXGI_DIMENSION_R8_UNORM:
    case DXGI_DIMENSION_R8_UINT:
    case DXGI_DIMENSION_R8_SNORM:
    case DXGI_DIMENSION_R8_SINT:
    case DXGI_DIMENSION_A8_UNORM:
    case DXGI_DIMENSION_AI44:
    case DXGI_DIMENSION_IA44:
    case DXGI_DIMENSION_P8:
        return 8;

    case DXGI_DIMENSION_R1_UNORM:
        return 1;

    case DXGI_DIMENSION_BC1_TYPELESS:
    case DXGI_DIMENSION_BC1_UNORM:
    case DXGI_DIMENSION_BC1_UNORM_SRGB:
    case DXGI_DIMENSION_BC4_TYPELESS:
    case DXGI_DIMENSION_BC4_UNORM:
    case DXGI_DIMENSION_BC4_SNORM:
        return 4;

    case DXGI_DIMENSION_BC2_TYPELESS:
    case DXGI_DIMENSION_BC2_UNORM:
    case DXGI_DIMENSION_BC2_UNORM_SRGB:
    case DXGI_DIMENSION_BC3_TYPELESS:
    case DXGI_DIMENSION_BC3_UNORM:
    case DXGI_DIMENSION_BC3_UNORM_SRGB:
    case DXGI_DIMENSION_BC5_TYPELESS:
    case DXGI_DIMENSION_BC5_UNORM:
    case DXGI_DIMENSION_BC5_SNORM:
    case DXGI_DIMENSION_BC6H_TYPELESS:
    case DXGI_DIMENSION_BC6H_UF16:
    case DXGI_DIMENSION_BC6H_SF16:
    case DXGI_DIMENSION_BC7_TYPELESS:
    case DXGI_DIMENSION_BC7_UNORM:
    case DXGI_DIMENSION_BC7_UNORM_SRGB:
        return 8;

    default:
        return 0;
    }
}

#define ISBITMASK( r,g,b,a ) ( ddpf.dwRBitMask == r && ddpf.dwGBitMask == g && ddpf.dwBBitMask == b && ddpf.dwABitMask == a )

DXGI_DIMENSION GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
{
    if (ddpf.dwFlags & DDS_RGB)
    {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf.dwRGBBitCount)
        {
        case 32:
            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return DXGI_DIMENSION_R8G8B8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
            {
                return DXGI_DIMENSION_B8G8R8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
            {
                return DXGI_DIMENSION_B8G8R8X8_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

            // Note that many common DDS reader/writers (including D3DX) swap the
            // the RED/BLUE masks for 10:10:10:2 formats. We assume
            // below that the 'backwards' header mask is being used since it is most
            // likely written by D3DX. The more robust solution is to use the 'DX10'
            // header extension and specify the DXGI_DIMENSION_R10G10B10A2_UNORM format directly

            // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
            if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
            {
                return DXGI_DIMENSION_R10G10B10A2_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

            if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
            {
                return DXGI_DIMENSION_R16G16_UNORM;
            }

            if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
            {
                // Only 32-bit color channel format in D3D9 was R32F
                return DXGI_DIMENSION_R32_FLOAT; // D3DX writes this out as a FourCC of 114
            }
            break;

        case 24:
            // No 24bpp DXGI formats aka D3DFMT_R8G8B8
            break;

        case 16:
            if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
            {
                return DXGI_DIMENSION_B5G5R5A1_UNORM;
            }
            if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
            {
                return DXGI_DIMENSION_B5G6R5_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

            if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
            {
                return DXGI_DIMENSION_B4G4R4A4_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

            // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
            break;
        }
    }
    else if (ddpf.dwFlags & DDS_LUMINANCE)
    {
        if (8 == ddpf.dwRGBBitCount)
        {
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
            {
                return DXGI_DIMENSION_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }

            // No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
            {
                return DXGI_DIMENSION_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
            }
        }

        if (16 == ddpf.dwRGBBitCount)
        {
            if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
            {
                return DXGI_DIMENSION_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
            {
                return DXGI_DIMENSION_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
        }
    }
    else if (ddpf.dwFlags & DDS_ALPHA)
    {
        if (8 == ddpf.dwRGBBitCount)
        {
            return DXGI_DIMENSION_A8_UNORM;
        }
    }
    else if (ddpf.dwFlags & DDS_BUMPDUDV)
    {
        if (16 == ddpf.dwRGBBitCount)
        {
            if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
            {
                return DXGI_DIMENSION_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
            }
        }

        if (32 == ddpf.dwRGBBitCount)
        {
            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return DXGI_DIMENSION_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
            }
            if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
            {
                return DXGI_DIMENSION_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
            }

            // No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
        }
    }
    else if (ddpf.dwFlags & DDS_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC1_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC3_UNORM;
        }

        // While pre-multiplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC3_UNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC4_SNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_BC5_SNORM;
        }

        // BC6H and BC7 are written using the "DX10" extended header

        if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_R8G8_B8G8_UNORM;
        }
        if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_G8R8_G8B8_UNORM;
        }

        if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.dwFourCC)
        {
            return DXGI_DIMENSION_YUY2;
        }

        // Check for D3DFORMAT enums being set here
        switch (ddpf.dwFourCC)
        {
        case 36: // D3DFMT_A16B16G16R16
            return DXGI_DIMENSION_R16G16B16A16_UNORM;

        case 110: // D3DFMT_Q16W16V16U16
            return DXGI_DIMENSION_R16G16B16A16_SNORM;

        case 111: // D3DFMT_R16F
            return DXGI_DIMENSION_R16_FLOAT;

        case 112: // D3DFMT_G16R16F
            return DXGI_DIMENSION_R16G16_FLOAT;

        case 113: // D3DFMT_A16B16G16R16F
            return DXGI_DIMENSION_R16G16B16A16_FLOAT;

        case 114: // D3DFMT_R32F
            return DXGI_DIMENSION_R32_FLOAT;

        case 115: // D3DFMT_G32R32F
            return DXGI_DIMENSION_R32G32_FLOAT;

        case 116: // D3DFMT_A32B32G32R32F
            return DXGI_DIMENSION_R32G32B32A32_FLOAT;
        }
    }

    return DXGI_DIMENSION_UNKNOWN;
}

void flan::core::LoadDirectDrawSurface( FileSystemObject* stream, DirectDrawSurface& data )
{
    uint32_t fileMagic;
    stream->read( fileMagic );

    if ( fileMagic != DDS_MAGIC ) {
        FLAN_CERR << "Invalid DDS file!" << std::endl;
        return;
    }

    DDS_HEADER hdr;
    stream->read<DDS_HEADER>( hdr );

    // Verify header to validate DDS file
    if ( hdr.dwSize != sizeof( DDS_HEADER )
      || hdr.ddspf.dwSize != sizeof( DDS_PIXELFORMAT ) ) {
        return;
    }

    TextureDescription desc;
    desc.width = hdr.dwWidth;
    desc.height = hdr.dwHeight;
    desc.depth = hdr.dwDepth;
    desc.mipCount = hdr.dwMipMapCount;
    desc.arraySize = 1;

    bool isDXT10Header = ( hdr.ddspf.dwFlags & DDS_FOURCC ) && ( MAKEFOURCC('D', 'X', '1', '0') == hdr.ddspf.dwFourCC );

    if ( isDXT10Header ) {
        DDS_HEADER_DXT10 d3d10ext;
        stream->read( d3d10ext );

        desc.arraySize = d3d10ext.arraySize;
        desc.format = static_cast<eImageFormat>( d3d10ext.dxgiFormat );

        switch ( d3d10ext.resourceDimension ) {
        case D3D10_RESOURCE_DIMENSION_TEXTURE1D:
            desc.height = desc.depth = 1;
            break;

        case D3D10_RESOURCE_DIMENSION_TEXTURE2D:
            if ( d3d10ext.miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE ) {
                desc.arraySize *= 6;
                desc.flags.isCubeMap = 1;
            }
            desc.depth = 1;
            break;

        case D3D10_RESOURCE_DIMENSION_TEXTURE3D:
            if ( !( hdr.dwFlags & DDS_HEADER_FLAGS_VOLUME ) ) {
                return;
            }

            if ( desc.arraySize > 1 ) {
                return;
            }
            break;

        default:
            return;
        }

        desc.dimension = static_cast<decltype( TextureDescription::dimension )>( d3d10ext.resourceDimension );
    } else {
        desc.format = static_cast<eImageFormat>( GetDXGIFormat( hdr.ddspf ) );

        if ( desc.format == IMAGE_FORMAT_UNKNOWN ) {
            return;
        }

        if ( hdr.dwFlags & DDS_HEADER_FLAGS_VOLUME ) {
            desc.dimension = TextureDescription::DIMENSION_TEXTURE_3D;
        } else {
            if ( hdr.dwCaps2 & DDS_CUBEMAP ) {
                if ( ( hdr.dwCaps2 & DDS_CUBEMAP_ALLFACES ) != DDS_CUBEMAP_ALLFACES ) {
                    return;
                }

                desc.arraySize = 6;
                desc.flags.isCubeMap = true;
            }

            desc.depth = 1;
            desc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;

            // NOTE There's no way for a legacy Direct3D 9 DDS to express a '1D' texture
        }
    }

    auto streamSize = stream->getSize();
    auto texelsSize = streamSize - stream->tell();

    data.textureDescription = desc;
    data.textureData.resize( texelsSize );

    stream->read( &data.textureData[0], texelsSize );
}

bool IsUsingCompressedFormat( const eImageFormat format )
{
    return format == IMAGE_FORMAT_BC1_TYPELESS
        || format == IMAGE_FORMAT_BC1_UNORM
        || format == IMAGE_FORMAT_BC1_UNORM_SRGB
        || format == IMAGE_FORMAT_BC2_TYPELESS
        || format == IMAGE_FORMAT_BC2_UNORM
        || format == IMAGE_FORMAT_BC2_UNORM_SRGB
        || format == IMAGE_FORMAT_BC3_TYPELESS
        || format == IMAGE_FORMAT_BC3_UNORM
        || format == IMAGE_FORMAT_BC3_UNORM_SRGB
        || format == IMAGE_FORMAT_BC4_TYPELESS
        || format == IMAGE_FORMAT_BC4_UNORM
        || format == IMAGE_FORMAT_BC4_SNORM
        || format == IMAGE_FORMAT_BC5_TYPELESS
        || format == IMAGE_FORMAT_BC5_UNORM
        || format == IMAGE_FORMAT_BC5_SNORM;
}

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHAPIXELS 0x00000001  // DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8
#define DDS_PAL8A       0x00000021  // DDPF_PALETTEINDEXED8 | DDPF_ALPHAPIXELS
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV
#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX
#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME


enum CP_FLAGS
{
    CP_FLAGS_NONE = 0x0,      // Normal operation
    CP_FLAGS_LEGACY_DWORD = 0x1,      // Assume pitch is DWORD aligned instead of BYTE aligned
    CP_FLAGS_PARAGRAPH = 0x2,      // Assume pitch is 16-byte aligned instead of BYTE aligned
    CP_FLAGS_YMM = 0x4,      // Assume pitch is 32-byte aligned instead of BYTE aligned
    CP_FLAGS_ZMM = 0x8,      // Assume pitch is 64-byte aligned instead of BYTE aligned
    CP_FLAGS_PAGE4K = 0x200,    // Assume pitch is 4096-byte aligned instead of BYTE aligned
    CP_FLAGS_BAD_DXTN_TAILS = 0x1000,   // BC formats with malformed mipchain blocks smaller than 4x4
    CP_FLAGS_24BPP = 0x10000,  // Override with a legacy 24 bits-per-pixel format size
    CP_FLAGS_16BPP = 0x20000,  // Override with a legacy 16 bits-per-pixel format size
    CP_FLAGS_8BPP = 0x40000,  // Override with a legacy 8 bits-per-pixel format size
};

bool ComputePitch( eImageFormat fmt, size_t width, size_t height, size_t& rowPitch, size_t& slicePitch, DWORD flags )
{
    uint64_t pitch = 0;
    uint64_t slice = 0;

    switch ( static_cast<int>( fmt ) ) {
    case IMAGE_FORMAT_BC1_TYPELESS:
    case IMAGE_FORMAT_BC1_UNORM:
    case IMAGE_FORMAT_BC1_UNORM_SRGB:
    case IMAGE_FORMAT_BC4_TYPELESS:
    case IMAGE_FORMAT_BC4_UNORM:
    case IMAGE_FORMAT_BC4_SNORM:
    {
        if ( flags & CP_FLAGS_BAD_DXTN_TAILS ) {
            size_t nbw = width >> 2;
            size_t nbh = height >> 2;
            pitch = std::max<uint64_t>( 1u, uint64_t( nbw ) * 8u );
            slice = std::max<uint64_t>( 1u, pitch * uint64_t( nbh ) );
        } else {
            uint64_t nbw = std::max<uint64_t>( 1u, ( uint64_t( width ) + 3u ) / 4u );
            uint64_t nbh = std::max<uint64_t>( 1u, ( uint64_t( height ) + 3u ) / 4u );
            pitch = nbw * 8u;
            slice = pitch * nbh;
        }
    }
    break;

    case IMAGE_FORMAT_BC2_TYPELESS:
    case IMAGE_FORMAT_BC2_UNORM:
    case IMAGE_FORMAT_BC2_UNORM_SRGB:
    case IMAGE_FORMAT_BC3_TYPELESS:
    case IMAGE_FORMAT_BC3_UNORM:
    case IMAGE_FORMAT_BC3_UNORM_SRGB:
    case IMAGE_FORMAT_BC5_TYPELESS:
    case IMAGE_FORMAT_BC5_UNORM:
    case IMAGE_FORMAT_BC5_SNORM:
    case IMAGE_FORMAT_BC6H_TYPELESS:
    case IMAGE_FORMAT_BC6H_UF16:
    case IMAGE_FORMAT_BC6H_SF16:
    case IMAGE_FORMAT_BC7_TYPELESS:
    case IMAGE_FORMAT_BC7_UNORM:
    case IMAGE_FORMAT_BC7_UNORM_SRGB:
    {
        if ( flags & CP_FLAGS_BAD_DXTN_TAILS ) {
            size_t nbw = width >> 2;
            size_t nbh = height >> 2;
            pitch = std::max<uint64_t>( 1u, uint64_t( nbw ) * 16u );
            slice = std::max<uint64_t>( 1u, pitch * uint64_t( nbh ) );
        } else {
            uint64_t nbw = std::max<uint64_t>( 1u, ( uint64_t( width ) + 3u ) / 4u );
            uint64_t nbh = std::max<uint64_t>( 1u, ( uint64_t( height ) + 3u ) / 4u );
            pitch = nbw * 16u;
            slice = pitch * nbh;
        }
    }
    break;

    case IMAGE_FORMAT_R8G8_B8G8_UNORM:
    case IMAGE_FORMAT_G8R8_G8B8_UNORM:
    case IMAGE_FORMAT_YUY2:
        pitch = ( ( uint64_t( width ) + 1u ) >> 1 ) * 4u;
        slice = pitch * uint64_t( height );
        break;

    case IMAGE_FORMAT_Y210:
    case IMAGE_FORMAT_Y216:
        pitch = ( ( uint64_t( width ) + 1u ) >> 1 ) * 8u;
        slice = pitch * uint64_t( height );
        break;

    case IMAGE_FORMAT_NV12:
    case IMAGE_FORMAT_420_OPAQUE:
        pitch = ( ( uint64_t( width ) + 1u ) >> 1 ) * 2u;
        slice = pitch * ( uint64_t( height ) + ( ( uint64_t( height ) + 1u ) >> 1 ) );
        break;

    case IMAGE_FORMAT_P010:
    case IMAGE_FORMAT_P016:
#if FLAN_DURANGO
    case XBOX_IMAGE_FORMAT_D16_UNORM_S8_UINT:
    case XBOX_IMAGE_FORMAT_R16_UNORM_X8_TYPELESS:
    case XBOX_IMAGE_FORMAT_X16_TYPELESS_G8_UINT:
#endif
        pitch = ( ( uint64_t( width ) + 1u ) >> 1 ) * 4u;
        slice = pitch * ( uint64_t( height ) + ( ( uint64_t( height ) + 1u ) >> 1 ) );
        break;

    case IMAGE_FORMAT_NV11:
        pitch = ( ( uint64_t( width ) + 3u ) >> 2 ) * 4u;
        slice = pitch * uint64_t( height ) * 2u;
        break;

    default:
    {
        size_t bpp;

        if ( flags & CP_FLAGS_24BPP )
            bpp = 24;
        else if ( flags & CP_FLAGS_16BPP )
            bpp = 16;
        else if ( flags & CP_FLAGS_8BPP )
            bpp = 8;
        else
            bpp = BitsPerPixel( (DXGI_DIMENSION)fmt );

        if ( !bpp )
            return E_INVALIDARG;

        if ( flags & ( CP_FLAGS_LEGACY_DWORD | CP_FLAGS_PARAGRAPH | CP_FLAGS_YMM | CP_FLAGS_ZMM | CP_FLAGS_PAGE4K ) ) {
            if ( flags & CP_FLAGS_PAGE4K ) {
                pitch = ( ( uint64_t( width ) * bpp + 32767u ) / 32768u ) * 4096u;
                slice = pitch * uint64_t( height );
            } else if ( flags & CP_FLAGS_ZMM ) {
                pitch = ( ( uint64_t( width ) * bpp + 511u ) / 512u ) * 64u;
                slice = pitch * uint64_t( height );
            } else if ( flags & CP_FLAGS_YMM ) {
                pitch = ( ( uint64_t( width ) * bpp + 255u ) / 256u ) * 32u;
                slice = pitch * uint64_t( height );
            } else if ( flags & CP_FLAGS_PARAGRAPH ) {
                pitch = ( ( uint64_t( width ) * bpp + 127u ) / 128u ) * 16u;
                slice = pitch * uint64_t( height );
            } else // DWORD alignment
            {
                // Special computation for some incorrectly created DDS files based on
                // legacy DirectDraw assumptions about pitch alignment
                pitch = ( ( uint64_t( width ) * bpp + 31u ) / 32u ) * sizeof( uint32_t );
                slice = pitch * uint64_t( height );
            }
        } else {
            // Default byte alignment
            pitch = ( uint64_t( width ) * bpp + 7u ) / 8u;
            slice = pitch * uint64_t( height );
        }
    }
    break;
    }

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
    static_assert( sizeof( size_t ) == 4, "Not a 32-bit platform!" );
    if ( pitch > UINT32_MAX || slice > UINT32_MAX ) {
        rowPitch = slicePitch = 0;
        return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );
    }

#else
    static_assert( sizeof( size_t ) == 8, "Not a 64-bit platform!" );
#endif

    rowPitch = static_cast<size_t>( pitch );
    slicePitch = static_cast<size_t>( slice );

    return true;
}

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_DXT1 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D','X','T','1' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_DXT2 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D','X','T','2' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_DXT3 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D','X','T','3' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_DXT4 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D','X','T','4' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_DXT5 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D','X','T','5' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_BC4_UNORM =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B','C','4','U' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_BC4_SNORM =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B','C','4','S' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_BC5_UNORM =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B','C','5','U' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_BC5_SNORM =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B','C','5','S' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'R','G','B','G' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'G','R','G','B' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_YUY2 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'Y','U','Y','2' ), 0, 0, 0, 0, 0 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_A8R8G8B8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_X8R8G8B8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_A8B8G8R8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_X8B8G8R8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGB,  0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_G16R16 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_R5G6B5 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_A1R5G5B5 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_A4R4G4B4 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_R8G8B8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_L8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_LUMINANCE, 0,  8, 0xff, 0x00, 0x00, 0x00 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_L16 =
{ sizeof( DDS_PIXELFORMAT ), DDS_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_A8L8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_A8L8_ALT =
{ sizeof( DDS_PIXELFORMAT ), DDS_LUMINANCEA, 0, 8, 0x00ff, 0x0000, 0x0000, 0xff00 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_A8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_V8U8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_Q8W8V8U8 =
{ sizeof( DDS_PIXELFORMAT ), DDS_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_V16U16 =
{ sizeof( DDS_PIXELFORMAT ), DDS_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

// D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue

// This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
extern __declspec( selectany ) const DDS_PIXELFORMAT DDSPF_DX10 =
{ sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D','X','1','0' ), 0, 0, 0, 0, 0 };

void flan::core::SaveDirectDrawSurface( FileSystemObject* stream, std::vector<float>& texels, const TextureDescription& description )
{
    // DDS Header
    uint32_t flags = 0;

    if ( description.arraySize > 1 ) {
        if ( ( description.arraySize != 6 ) 
            || ( description.dimension != TextureDescription::DIMENSION_TEXTURE_2D ) 
            || description.flags.isCubeMap == 0 ) {
            // Texture1D arrays, Texture2D arrays, and Cubemap arrays must be stored using 'DX10' extended header
            flags |= DDS_FLAGS_FORCE_DX10_EXT;
        }
    }

    if ( flags & DDS_FLAGS_FORCE_DX10_EXT_MISC2 ) {
        flags |= DDS_FLAGS_FORCE_DX10_EXT;
    }

    DDS_PIXELFORMAT ddpf = {};
    if ( !( flags & DDS_FLAGS_FORCE_DX10_EXT ) ) {
        switch ( description.format ) {
        case IMAGE_FORMAT_R8G8B8A8_UNORM:        memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_A8B8G8R8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R16G16_UNORM:          memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_G16R16, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R8G8_UNORM:            memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_A8L8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R16_UNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_L16, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R8_UNORM:              memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_L8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_A8_UNORM:              memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_A8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R8G8_B8G8_UNORM:       memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_R8G8_B8G8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_G8R8_G8B8_UNORM:       memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_G8R8_G8B8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_BC1_UNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_DXT1, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_BC2_UNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_DXT3, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_BC3_UNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_DXT5, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_BC4_UNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_BC4_UNORM, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_BC4_SNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_BC4_SNORM, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_BC5_UNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_BC5_UNORM, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_BC5_SNORM:             memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_BC5_SNORM, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_B5G6R5_UNORM:          memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_R5G6B5, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_B5G5R5A1_UNORM:        memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_A1R5G5B5, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R8G8_SNORM:            memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_V8U8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R8G8B8A8_SNORM:        memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_Q8W8V8U8, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_R16G16_SNORM:          memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_V16U16, sizeof( DDS_PIXELFORMAT ) ); break;
        case IMAGE_FORMAT_B8G8R8A8_UNORM:        memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_A8R8G8B8, sizeof( DDS_PIXELFORMAT ) ); break; // DXGI 1.1
        case IMAGE_FORMAT_B8G8R8X8_UNORM:        memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_X8R8G8B8, sizeof( DDS_PIXELFORMAT ) ); break; // DXGI 1.1
        case IMAGE_FORMAT_B4G4R4A4_UNORM:        memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_A4R4G4B4, sizeof( DDS_PIXELFORMAT ) ); break; // DXGI 1.2
        case IMAGE_FORMAT_YUY2:                  memcpy_s( &ddpf, sizeof( ddpf ), &DDSPF_YUY2, sizeof( DDS_PIXELFORMAT ) ); break; // DXGI 1.2

                                                                                                                                  // Legacy D3DX formats using D3DFMT enum value as FourCC
        case IMAGE_FORMAT_R32G32B32A32_FLOAT:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 116;  // D3DFMT_A32B32G32R32F
            break;
        case IMAGE_FORMAT_R16G16B16A16_FLOAT:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 113;  // D3DFMT_A16B16G16R16F
            break;
        case IMAGE_FORMAT_R16G16B16A16_UNORM:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 36;  // D3DFMT_A16B16G16R16
            break;
        case IMAGE_FORMAT_R16G16B16A16_SNORM:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 110;  // D3DFMT_Q16W16V16U16
            break;
        case IMAGE_FORMAT_R32G32_FLOAT:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 115;  // D3DFMT_G32R32F
            break;
        case IMAGE_FORMAT_R16G16_FLOAT:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 112;  // D3DFMT_G16R16F
            break;
        case IMAGE_FORMAT_R32_FLOAT:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 114;  // D3DFMT_R32F
            break;
        case IMAGE_FORMAT_R16_FLOAT:
            ddpf.dwSize = sizeof( DDS_PIXELFORMAT ); ddpf.dwFlags = DDS_FOURCC; ddpf.dwFourCC = 111;  // D3DFMT_R16F
            break;

        default:
            break;
        }
    }

    std::size_t required = sizeof( uint32_t ) + sizeof( DDS_HEADER );

    if ( ddpf.dwSize == 0 )
        required += sizeof( DDS_HEADER_DXT10 );

    DDS_HEADER header;
    header.dwSize = sizeof( DDS_HEADER );

    header.dwFlags = DDS_HEADER_FLAGS_TEXTURE;
    header.dwCaps = DDS_SURFACE_FLAGS_TEXTURE;

    if ( description.mipCount > 0 ) {
        header.dwFlags |= DDS_HEADER_FLAGS_MIPMAP;

        header.dwMipMapCount = static_cast<uint32_t>( description.mipCount );

        if ( header.dwMipMapCount > 1 )
            header.dwCaps |= DDS_SURFACE_FLAGS_MIPMAP;
    }


    switch ( description.dimension ) {
    case TextureDescription::DIMENSION_TEXTURE_1D:
        header.dwWidth = static_cast<uint32_t>( description.width );
        header.dwHeight = header.dwDepth = 1;
        break;

    case TextureDescription::DIMENSION_TEXTURE_2D:
        header.dwWidth = static_cast<uint32_t>( description.width );
        header.dwHeight = static_cast<uint32_t>( description.height );
        header.dwDepth = 1;

        if ( description.flags.isCubeMap == 1 ) {
            header.dwCaps |= DDS_SURFACE_FLAGS_CUBEMAP;
            header.dwCaps2 |= DDS_CUBEMAP_ALLFACES;
        }
        break;

    case TextureDescription::DIMENSION_TEXTURE_3D:
        header.dwFlags |= DDS_HEADER_FLAGS_VOLUME;
        header.dwCaps2 |= DDS_FLAGS_VOLUME;
        header.dwWidth = static_cast<uint32_t>( description.width );
        header.dwHeight = static_cast<uint32_t>( description.height );
        header.dwDepth = static_cast<uint32_t>( description.depth );
        break;
    }

    size_t rowPitch, slicePitch;
    ComputePitch( description.format, description.width, description.height, rowPitch, slicePitch, CP_FLAGS_NONE );
    if ( IsUsingCompressedFormat( description.format ) ) {
        header.dwFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        header.dwPitchOrLinearSize = static_cast<uint32_t>( slicePitch );
    } else {
        header.dwFlags |= DDS_HEADER_FLAGS_PITCH;
        header.dwPitchOrLinearSize = static_cast<uint32_t>( rowPitch );
    }

    if ( ddpf.dwSize == 0 ) {
        memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_DX10, sizeof( DDS_PIXELFORMAT ) );

        DDS_HEADER_DXT10 ext;
        ext.dxgiFormat = (DXGI_DIMENSION) description.format;
        ext.resourceDimension = description.dimension;

        ext.miscFlag = metadata.miscFlags & ~TEX_MISC_TEXTURECUBE;

        if ( metadata.miscFlags & TEX_MISC_TEXTURECUBE ) {
            ext->miscFlag |= TEX_MISC_TEXTURECUBE;
            assert( ( metadata.arraySize % 6 ) == 0 );
            ext->arraySize = static_cast<UINT>( metadata.arraySize / 6 );
        } else {
            ext->arraySize = static_cast<UINT>( metadata.arraySize );
        }

        if ( flags & DDS_FLAGS_FORCE_DX10_EXT_MISC2 ) {
            // This was formerly 'reserved'. D3DX10 and D3DX11 will fail if this value is anything other than 0
            ext->miscFlags2 = metadata.miscFlags2;
        }
    }


    // Write images
    switch ( description.dimension ) {
    case TextureDescription::DIMENSION_TEXTURE_1D:
    case TextureDescription::DIMENSION_TEXTURE_2D:
    {
        size_t index = 0;
        for ( size_t item = 0; item < description.arraySize; ++item ) {
            for ( size_t level = 0; level < description.mipLevels; ++level, ++index ) {
                if ( index >= nimages )
                    return E_FAIL;

                if ( !images[index].pixels )
                    return E_POINTER;

                assert( images[index].rowPitch > 0 );
                assert( images[index].slicePitch > 0 );

                size_t ddsRowPitch, ddsSlicePitch;
                hr = ComputePitch( metadata.format, images[index].width, images[index].height, ddsRowPitch, ddsSlicePitch, CP_FLAGS_NONE );
                if ( FAILED( hr ) )
                    return hr;

                if ( ( images[index].slicePitch == ddsSlicePitch ) && ( ddsSlicePitch <= UINT32_MAX ) ) {
                    if ( !WriteFile( hFile.get(), images[index].pixels, static_cast<DWORD>( ddsSlicePitch ), &bytesWritten, nullptr ) ) {
                        return HRESULT_FROM_WIN32( GetLastError() );
                    }

                    if ( bytesWritten != ddsSlicePitch ) {
                        return E_FAIL;
                    }
                } else {
                    size_t rowPitch = images[index].rowPitch;
                    if ( rowPitch < ddsRowPitch ) {
                        // DDS uses 1-byte alignment, so if this is happening then the input pitch isn't actually a full line of data
                        return E_FAIL;
                    }

                    if ( ddsRowPitch > UINT32_MAX )
                        return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );

                    const uint8_t * __restrict sPtr = images[index].pixels;

                    size_t lines = ComputeScanlines( metadata.format, images[index].height );
                    for ( size_t j = 0; j < lines; ++j ) {
                        if ( !WriteFile( hFile.get(), sPtr, static_cast<DWORD>( ddsRowPitch ), &bytesWritten, nullptr ) ) {
                            return HRESULT_FROM_WIN32( GetLastError() );
                        }

                        if ( bytesWritten != ddsRowPitch ) {
                            return E_FAIL;
                        }

                        sPtr += rowPitch;
                    }
                }
            }
        }
    }
    break;

    case DDS_DIMENSION_TEXTURE3D:
    {
        if ( metadata.arraySize != 1 )
            return E_FAIL;

        size_t d = metadata.depth;

        size_t index = 0;
        for ( size_t level = 0; level < metadata.mipLevels; ++level ) {
            for ( size_t slice = 0; slice < d; ++slice, ++index ) {
                if ( index >= nimages )
                    return E_FAIL;

                if ( !images[index].pixels )
                    return E_POINTER;

                assert( images[index].rowPitch > 0 );
                assert( images[index].slicePitch > 0 );

                size_t ddsRowPitch, ddsSlicePitch;
                hr = ComputePitch( metadata.format, images[index].width, images[index].height, ddsRowPitch, ddsSlicePitch, CP_FLAGS_NONE );
                if ( FAILED( hr ) )
                    return hr;

                if ( ( images[index].slicePitch == ddsSlicePitch ) && ( ddsSlicePitch <= UINT32_MAX ) ) {
                    if ( !WriteFile( hFile.get(), images[index].pixels, static_cast<DWORD>( ddsSlicePitch ), &bytesWritten, nullptr ) ) {
                        return HRESULT_FROM_WIN32( GetLastError() );
                    }

                    if ( bytesWritten != ddsSlicePitch ) {
                        return E_FAIL;
                    }
                } else {
                    size_t rowPitch = images[index].rowPitch;
                    if ( rowPitch < ddsRowPitch ) {
                        // DDS uses 1-byte alignment, so if this is happening then the input pitch isn't actually a full line of data
                        return E_FAIL;
                    }

                    if ( ddsRowPitch > UINT32_MAX )
                        return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );

                    const uint8_t * __restrict sPtr = images[index].pixels;

                    size_t lines = ComputeScanlines( metadata.format, images[index].height );
                    for ( size_t j = 0; j < lines; ++j ) {
                        if ( !WriteFile( hFile.get(), sPtr, static_cast<DWORD>( ddsRowPitch ), &bytesWritten, nullptr ) ) {
                            return HRESULT_FROM_WIN32( GetLastError() );
                        }

                        if ( bytesWritten != ddsRowPitch ) {
                            return E_FAIL;
                        }

                        sPtr += rowPitch;
                    }
                }
            }

            if ( d > 1 )
                d >>= 1;
        }
    }
    break;

    default:
        break;
    }
}
