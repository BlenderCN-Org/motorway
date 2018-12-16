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

#if FLAN_GL460
#include "Texture.h"

#include "ImageFormat.h"
#include "ImageFormatSamplingMask.h"
#include "ImageFormatType.h"

bool IsTextureCompressed( const GLenum textureFormat )
{
    return textureFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT
        || textureFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
        || textureFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        || textureFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        || textureFormat == GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
        || textureFormat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
        || textureFormat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
        || textureFormat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
}

NativeTextureObject* flan::rendering::CreateTexture1DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    GLenum target = GL_TEXTURE_1D;

    const bool isArray = ( description.arraySize > 1 );
    if ( isArray ) {
        target = GL_TEXTURE_1D_ARRAY;
    }

    GLuint handle = 0;

    glCreateTextures( target, 1, &handle );
    glBindTexture( target, handle );

    if ( !isArray ) {
        glTexImage1D(
                target,
                0,
                GL_IMAGE_FORMAT[description.format],
                description.width,
                0,
                GL_IMAGE_FORMAT_SAMPLING_MASK[description.format],
                GL_IMAGE_FORMAT_TYPE[description.format],
                initialData );
    } else {
        glTexImage2D(
                target,
                0,
                GL_IMAGE_FORMAT[description.format],
                description.width,
                description.arraySize,
                0,
                GL_IMAGE_FORMAT_SAMPLING_MASK[description.format],
                GL_IMAGE_FORMAT_TYPE[description.format],
                initialData );
    }

    // Set default sampling parameters
    // Sampler should be defined as sampler objects (easier for abstraction between API + avoid unecessary stuff)
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( target, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( target, GL_TEXTURE_MAX_LEVEL, description.mipCount );

    glBindTexture( target, 0 );

    NativeTextureObject* nativeTextureObject = new NativeTextureObject();
    nativeTextureObject->target = target;
    nativeTextureObject->textureHandle = handle;

    return nativeTextureObject;
}

NativeTextureObject* flan::rendering::CreateTexture2DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    GLenum target = GL_TEXTURE_2D;
    const bool isMultisampled = ( description.samplerCount > 1 );
    const bool isArray = ( description.arraySize > 1 );

    if ( isMultisampled ) {
        target = GL_TEXTURE_2D_MULTISAMPLE;
    } else if ( description.flags.isCubeMap ) {
        target = isArray ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
    } else if ( isArray ) {
        target = GL_TEXTURE_2D_ARRAY;
    }

    GLuint handle = 0;
    GLint mipLevelCount = static_cast<GLint>( description.mipCount );

    glCreateTextures( target, 1, &handle );
    glBindTexture( target, handle );

    if ( isMultisampled ) {
        glTexImage2DMultisample( target, description.samplerCount, GL_IMAGE_FORMAT[description.format],
                                description.width, description.height, GL_TRUE );
    } else {
        if ( description.flags.isCubeMap ) {
            glTextureStorage3D( handle,
                mipLevelCount,
                GL_IMAGE_FORMAT[description.format],
                static_cast<GLsizei>( description.width ),
                static_cast<GLsizei>( description.height ),
                static_cast<GLsizei>( description.arraySize * description.depth ) );
        } else if ( isArray ) {
            glTexImage3D( target, 0, GL_IMAGE_FORMAT[description.format], description.width, description.height, description.arraySize, 0,
                          GL_IMAGE_FORMAT_SAMPLING_MASK[description.format], GL_IMAGE_FORMAT_TYPE[description.format],
                          initialData );
        } else {
            auto glFormat = GL_IMAGE_FORMAT[description.format];
            auto isCompressed = IsTextureCompressed( glFormat );

            unsigned int blockSize = ( glFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT || glFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ) ? 8 : 16;
            unsigned int offset = 0;

            auto width = description.width, height = description.height;
            for ( unsigned int level = 0; level < description.mipCount; ++level ) {
                unsigned int size = ( ( width + 3 ) / 4 ) * ( ( height + 3 ) / 4 ) * blockSize;

                if ( isCompressed )
                    glCompressedTexImage2D( target, level, glFormat, width, height, 0, size, ( void* )( ( char* )initialData + offset ) );
                else
                    glTexImage2D( target, level, GL_IMAGE_FORMAT[description.format], description.width, description.height, 0,
                        GL_IMAGE_FORMAT_SAMPLING_MASK[description.format], GL_IMAGE_FORMAT_TYPE[description.format],
                        ( void* )( ( char* )initialData + offset ) );

                offset += size;
                width >>= 1;
                height >>= 1;
            }
        }
    }

    // Set default sampling parameters
    // Sampler should be defined as sampler objects (easier for abstraction between API + avoid unecessary stuff)
    if ( !isMultisampled ) {
        glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    }

    glTexParameteri( target, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( target, GL_TEXTURE_MAX_LEVEL, mipLevelCount );

    glBindTexture( target, 0 );

    NativeTextureObject* nativeTextureObject = new NativeTextureObject();
    nativeTextureObject->target = target;
    nativeTextureObject->textureHandle = handle;

    return nativeTextureObject;
}

GLint GetMipLevelCount( const unsigned int width, const unsigned int height )
{
    GLint mipLevelCount = 0;

    auto mipLevelWidth = width, mipLevelHeight = height;
    while ( mipLevelWidth > 0 && mipLevelHeight > 0 ) {
        mipLevelCount++;

        mipLevelWidth >>= 1;
        mipLevelHeight >>= 1;
    }

    return mipLevelCount;
}

void flan::rendering::CopyResouceImpl( NativeRenderContext* nativeRenderContext, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject )
{

}

void flan::rendering::CopyResouceAsynchronousImpl( NativeCommandList* nativeCommandList, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject )
{

}

NativeTextureObject* flan::rendering::CreateTexture3DImpl( NativeRenderContext* nativeRenderContext, const TextureDescription& description, void* initialData, const std::size_t initialDataSize )
{
    const bool isArray = ( description.arraySize > 1 );

    GLenum target = ( isArray ) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_3D;
    GLint mipLevelCount = ( description.mipCount == -1u )
            ? GetMipLevelCount( description.width, description.height )
            : static_cast<GLint>( description.mipCount );

    GLuint handle = 0;

    glCreateTextures( target, 1, &handle );

    if ( isArray ) {
        FLAN_CERR << "Texture3D Array is unsupported!" << std::endl;
    } else {
        glBindTexture( target, handle );
        glTexImage3D( target, 0, GL_IMAGE_FORMAT[description.format], description.width, description.height, description.depth, 0,
                      GL_IMAGE_FORMAT_SAMPLING_MASK[description.format], GL_IMAGE_FORMAT_TYPE[description.format],
                      initialData );
    }

    // Set default sampling parameters
    // Sampler should be defined as sampler objects (easier for abstraction between API + avoid unecessary stuff)
    glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );

    glTexParameteri( target, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( target, GL_TEXTURE_MAX_LEVEL, mipLevelCount );

    glBindTexture( target, 0 );

    NativeTextureObject* nativeTextureObject = new NativeTextureObject();
    nativeTextureObject->target = target;
    nativeTextureObject->textureHandle = handle;

    return nativeTextureObject;
}

void flan::rendering::DestroyTextureImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject )
{
    if ( textureObject->textureHandle != 0 ) {
        glDeleteTextures( 1, &textureObject->textureHandle );
        textureObject->textureHandle = 0;
    }

    textureObject->target = 0;
}

void flan::rendering::BindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    glActiveTexture( GL_TEXTURE0 + bindingIndex );
    glBindTexture( textureObject->target, textureObject->textureHandle );
}

void flan::rendering::UnbindTextureCmdImpl( NativeCommandList* nativeCmdList, NativeTextureObject* textureObject, const uint32_t bindingIndex, const uint32_t shaderStagesToBindTo )
{
    glActiveTexture( GL_TEXTURE0 + bindingIndex );
    glBindTexture( textureObject->target, 0 );
}

void flan::rendering::SetTextureDebugNameImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const std::string& debugName )
{
    glObjectLabel( GL_TEXTURE, textureObject->textureHandle, static_cast<GLsizei>( debugName.length() ), debugName.c_str() );
}

unsigned int GetChannelCount( const GLenum imageSamplingFormat )
{
    switch( imageSamplingFormat ) {
    case GL_RED:
    case GL_DEPTH_COMPONENT:
    case GL_STENCIL_INDEX:
        return 1;

    case GL_RG:
        return 2;

    case GL_RGB:
    case GL_BGR:
        return 3;

    case GL_RGBA:
    case GL_BGRA:
        return 4;

    default:
        return 0;
    }
}

void flan::rendering::RetrieveTextureTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<uint8_t>& texels )
{
    auto imageSamplingFormat = GL_IMAGE_FORMAT_SAMPLING_MASK[description.format];
    auto channelCount = GetChannelCount( imageSamplingFormat );

    for ( unsigned int arrayIdx = 0; arrayIdx < description.arraySize; arrayIdx++ ) {
        for ( unsigned int depthIdx = 0; depthIdx < description.depth; depthIdx++ ) {

            auto width = description.width;
            auto height = description.height;
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                auto mipSize = width * height * channelCount;
                auto texelsSize = texels.size();
                texels.resize( ( texelsSize + mipSize ) );

                auto texelsOffset = ( texelsSize == 0 ) ? 0 : static_cast<unsigned int>( texelsSize - 1u );

                glGetTextureSubImage( textureObject->textureHandle, mipLevel, 0, 0, arrayIdx, width, height, ( depthIdx + 1 ),
                                      imageSamplingFormat, GL_UNSIGNED_BYTE, mipSize, &texels[texelsOffset] );

                width >>= 1;
                height >>= 1;
            }
        }
    }
}

void flan::rendering::RetrieveTextureLayerTexelsLDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, const unsigned int layerIndex, const unsigned int mipLevel, std::vector<uint8_t>& texels )
{
    auto imageSamplingFormat = GL_IMAGE_FORMAT_SAMPLING_MASK[description.format];
    auto channelCount = GetChannelCount( imageSamplingFormat );

    auto width = description.width >> mipLevel;
    auto height = description.height >> mipLevel;
    auto mipSize = width * height * channelCount;
    auto texelsSize = texels.size();
    texels.resize( ( texelsSize + mipSize ) );

    auto texelsOffset = ( texelsSize == 0 ) ? 0 : static_cast<unsigned int>( texelsSize - 1u );

    glGetTextureSubImage( textureObject->textureHandle, mipLevel, 0, 0, layerIndex, width, height, 1,
                          imageSamplingFormat, GL_UNSIGNED_BYTE, mipSize, &texels[texelsOffset] );
}

void flan::rendering::RetrieveTextureTexelsHDRImpl( NativeRenderContext* nativeRenderContext, NativeTextureObject* textureObject, const TextureDescription& description, std::vector<float>& texels )
{
    auto imageSamplingFormat = GL_IMAGE_FORMAT_SAMPLING_MASK[description.format];
    auto channelCount = GetChannelCount( imageSamplingFormat );

    for ( unsigned int arrayIdx = 0; arrayIdx < description.arraySize; arrayIdx++ ) {
        for ( unsigned int depthIdx = 0; depthIdx < description.depth; depthIdx++ ) {

            auto width = description.width;
            auto height = description.height;
            for ( unsigned int mipLevel = 0; mipLevel < description.mipCount; mipLevel++ ) {
                auto mipSize = width * height * channelCount * sizeof( float );
                auto texelsSize = texels.size();
                texels.resize( ( texelsSize + mipSize ) );

                auto texelsOffset = ( texelsSize == 0 ) ? 0 : static_cast<unsigned int>( texelsSize - 1u );

                glGetTextureSubImage( textureObject->textureHandle, mipLevel, 0, 0, arrayIdx, width, height, ( depthIdx + 1 ),
                                      imageSamplingFormat, GL_FLOAT, static_cast<GLsizei>( mipSize ), &texels[texelsOffset] );

                width >>= 1;
                height >>= 1;
            }
        }
    }
}

void flan::rendering::CopySubresouceRegionImpl( NativeRenderContext* nativeRenderContext, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc, const uint32_t arrayIdxSrc, const uint32_t mipDst, const uint32_t arrayIdxDst )
{

}

void flan::rendering::CopySubresouceRegionAsynchronousImpl( NativeCommandList* nativeCmdList, const NativeTextureObject* srcTextureObject, const NativeTextureObject* dstTextureObject, const uint32_t mipSrc, const uint32_t arrayIdxSrc, const uint32_t mipDst, const uint32_t arrayIdxDst )
{

}

void flan::rendering::UpdateSubresourceImpl( NativeCommandList* nativeCommandList, NativeTextureObject* textureObject, const TextureCopyBox& copyBox, const uint32_t regionWidth, const uint32_t regionHeight, const uint32_t regionComposition, const void* regionData )
{

}

void flan::rendering::CopyRenderTargetImpl( NativeCommandList* nativeCmdList, NativeTextureObject* source, NativeTextureObject* dest )
{

}
#endif
