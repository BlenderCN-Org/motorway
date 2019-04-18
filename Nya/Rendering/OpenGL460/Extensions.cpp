/*
    Project Motorway Source Code
    Copyright (C) 2018 Pr?vost Baptiste

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

#if NYA_GL460
#include "Extensions.h"

PFNGLVERTEXARRAYATTRIBBINDINGPROC pglVertexArrayAttribBinding = nullptr;
PFNGLBLENDCOLORPROC pglBlendColor = nullptr;
PFNGLDRAWELEMENTSBASEVERTEXPROC pglDrawElementsBaseVertex = nullptr;
PFNGLATTACHSHADERPROC pglAttachShader = nullptr;
PFNGLBINDBUFFERPROC pglBindBuffer = nullptr;
PFNGLBINDVERTEXARRAYPROC pglBindVertexArray = nullptr;
PFNGLBUFFERDATAPROC pglBufferData = nullptr;
PFNGLCOMPILESHADERPROC pglCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC pglCreateProgram = nullptr;
PFNGLCREATESHADERPROC pglCreateShader = nullptr;
PFNGLDELETEBUFFERSPROC pglDeleteBuffers = nullptr;
PFNGLDELETEPROGRAMPROC pglDeleteProgram = nullptr;
PFNGLDELETESHADERPROC pglDeleteShader = nullptr;
PFNGLDELETEVERTEXARRAYSPROC pglDeleteVertexArrays = nullptr;
PFNGLDETACHSHADERPROC pglDetachShader = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC pglEnableVertexAttribArray = nullptr;
PFNGLGENBUFFERSPROC pglGenBuffers = nullptr;
PFNGLGENVERTEXARRAYSPROC pglGenVertexArrays = nullptr;
PFNGLGETATTRIBLOCATIONPROC pglGetAttribLocation = nullptr;
PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog = nullptr;
PFNGLGETPROGRAMIVPROC pglGetProgramiv = nullptr;
PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog = nullptr;
PFNGLGETSHADERIVPROC pglGetShaderiv = nullptr;
PFNGLLINKPROGRAMPROC pglLinkProgram = nullptr;
PFNGLSHADERSOURCEPROC pglShaderSource = nullptr;
PFNGLUSEPROGRAMPROC pglUseProgram = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC pglVertexAttribPointer = nullptr;
PFNGLBINDATTRIBLOCATIONPROC pglBindAttribLocation = nullptr;
PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX2FVPROC pglUniformMatrix2fv = nullptr;
PFNGLUNIFORMMATRIX3FVPROC pglUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC pglUniformMatrix4fv = nullptr;
PFNGLACTIVETEXTUREPROC pglActiveTexture = nullptr;
PFNGLUNIFORM1IPROC pglUniform1i = nullptr;
PFNGLUNIFORM1FPROC pglUniform1f = nullptr;
PFNGLUNIFORM1IVPROC pglUniform1iv = nullptr;
PFNGLUNIFORM1FVPROC pglUniform1fv = nullptr;
PFNGLUNIFORM2IPROC pglUniform2i = nullptr;
PFNGLUNIFORM2FPROC pglUniform2f = nullptr;
PFNGLUNIFORM2IVPROC pglUniform2iv = nullptr;
PFNGLUNIFORM2FVPROC pglUniform2fv = nullptr;
PFNGLGENERATEMIPMAPPROC pglGenerateMipmap = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC pglDisableVertexAttribArray = nullptr;
PFNGLUNIFORM3IPROC pglUniform3i = nullptr;
PFNGLUNIFORM3FPROC pglUniform3f = nullptr;
PFNGLUNIFORM3IVPROC pglUniform3iv = nullptr;
PFNGLUNIFORM3FVPROC pglUniform3fv = nullptr;
PFNGLUNIFORM4IPROC pglUniform4i = nullptr;
PFNGLUNIFORM4FPROC pglUniform4f = nullptr;
PFNGLUNIFORM4IVPROC pglUniform4iv = nullptr;
PFNGLUNIFORM4FVPROC pglUniform4fv = nullptr;
PFNGLGENFRAMEBUFFERSPROC pglGenFramebuffers = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC pglFramebufferTexture2D = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC pglDeleteFramebuffers = nullptr;
PFNGLDRAWBUFFERSPROC pglDrawBuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC pglBindFramebuffer = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC pglCheckFramebufferStatus = nullptr;
PFNGLGETUNIFORMBLOCKINDEXPROC pglGetUniformBlockIndex = nullptr;
PFNGLUNIFORMBLOCKBINDINGPROC pglUniformBlockBinding = nullptr;
PFNGLMEMORYBARRIERPROC pglMemoryBarrier = nullptr;
PFNGLTEXBUFFERPROC pglTexBuffer = nullptr;
PFNGLBLITFRAMEBUFFERPROC pglBlitFramebuffer = nullptr;
PFNGLDISPATCHCOMPUTEPROC pglDispatchCompute = nullptr;
PFNGLGETPROGRAMRESOURCEINDEXPROC pglGetProgramResourceIndex = nullptr;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC pglShaderStorageBlockBinding = nullptr;
PFNGLFRAMEBUFFERTEXTURELAYERPROC pglFramebufferTextureLayer = nullptr;
PFNGLBLENDEQUATIONPROC pglBlendEquation = nullptr;
PFNGLTEXIMAGE3DPROC pglTexImage3D = nullptr;
PFNGLFRAMEBUFFERTEXTUREPROC pglFramebufferTexture = nullptr;
PFNGLBLENDEQUATIONSEPARATEPROC pglBlendEquationSeparate = nullptr;
PFNGLBLENDFUNCSEPARATEPROC pglBlendFuncSeparate = nullptr;
PFNGLFRAMEBUFFERPARAMETERIPROC pglFramebufferParameteri = nullptr;
PFNGLTEXSTORAGE3DPROC pglTexStorage3D = nullptr;
PFNGLBINDBUFFERBASEPROC pglBindBufferBase = nullptr;
PFNGLMAPBUFFERPROC pglMapBuffer = nullptr;
PFNGLUNMAPBUFFERPROC pglUnmapBuffer = nullptr;
PFNGLBINDIMAGETEXTUREPROC pglBindImageTexture = nullptr;
PFNGLBUFFERSTORAGEPROC pglBufferStorage = nullptr;
PFNGLMAPBUFFERRANGEPROC pglMapBufferRange = nullptr;
PFNGLCOMPRESSEDTEXIMAGE2DPROC pglCompressedTexImage2D = nullptr;
PFNGLDEBUGMESSAGECALLBACKPROC pglDebugMessageCallback = nullptr;
PFNGLGETTEXTUREHANDLEARBPROC pglGetTextureHandleARB = nullptr;
PFNGLUNIFORMHANDLEUI64ARBPROC pglUniformHandleui64ARB = nullptr;
PFNGLMAKETEXTUREHANDLERESIDENTARBPROC pglMakeTextureHandleResidentARB = nullptr;
PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC pglMakeTextureHandleNonResidentARB = nullptr;
PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC pglProgramUniformHandleui64ARB = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC pglFramebufferRenderbuffer = nullptr;
PFNGLCLIPCONTROLPROC pglClipControl = nullptr;
PFNGLDELETETEXTURESPROC pglDeleteTextures = nullptr;
PFNGLTEXIMAGE2DMULTISAMPLEPROC pglTexImage2DMultisample = nullptr;

#if NYA_WIN
    PFNWGLCHOOSEPIXELFORMATARBPROC pwglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC pwglCreateContextAttribsARB = nullptr;
    PFNWGLSWAPINTERVALEXTPROC pwglSwapIntervalEXT = nullptr;
#elif NYA_UNIX
    PFNGLXSWAPINTERVALEXTPROC pglXSwapIntervalEXT = nullptr;
#endif

PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC pglFlushMappedBufferRange = nullptr;
PFNGLFRAMEBUFFERTEXTURE1DPROC pglFramebufferTexture1D = nullptr;
PFNGLFRAMEBUFFERTEXTURE3DPROC pglFramebufferTexture3D = nullptr;

PFNGLSAMPLECOVERAGEPROC pglSampleCoverage = nullptr;
PFNGLSTENCILOPSEPARATEPROC pglStencilOpSeparate = nullptr;
PFNGLSTENCILFUNCSEPARATEPROC pglStencilFuncSeparate = nullptr;

PFNGLBINDVERTEXBUFFERPROC pglBindVertexBuffer = nullptr;
PFNGLOBJECTLABELPROC pglObjectLabel = nullptr;

PFNGLGENSAMPLERSPROC pglGenSamplers = nullptr;
PFNGLBINDSAMPLERPROC pglBindSampler = nullptr;
PFNGLDELETESAMPLERSPROC pglDeleteSamplers = nullptr;
PFNGLSAMPLERPARAMETERFPROC pglSamplerParameterf = nullptr;
PFNGLSAMPLERPARAMETERIPROC pglSamplerParameteri = nullptr;
PFNGLVERTEXARRAYATTRIBFORMATPROC pglVertexArrayAttribFormat = nullptr;
PFNGLENABLEVERTEXARRAYATTRIBPROC pglEnableVertexArrayAttrib = nullptr;
PFNGLVERTEXATTRIBBINDINGPROC pglVertexAttribBinding = nullptr;

PFNGLSHADERBINARYPROC pglShaderBinary = nullptr;
PFNGLSPECIALIZESHADERPROC pglSpecializeShader = nullptr;

PFNGLCLEARBUFFERFVPROC pglClearBufferfv = nullptr;
PFNGLNAMEDBUFFERSTORAGEPROC pglNamedBufferStorage = nullptr;
PFNGLCREATEBUFFERSPROC pglCreateBuffers = nullptr;
PFNGLVERTEXARRAYELEMENTBUFFERPROC pglVertexArrayElementBuffer = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERPROC pglVertexArrayVertexBuffer = nullptr;
PFNGLCREATEFRAMEBUFFERSPROC pglCreateFramebuffers = nullptr;

PFNGLNAMEDFRAMEBUFFERTEXTUREPROC pglNamedFramebufferTexture = nullptr;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC pglCheckNamedFramebufferStatus = nullptr;

PFNGLNAMEDBUFFERDATAPROC pglNamedBufferData = nullptr;
PFNGLCREATEVERTEXARRAYSPROC pglCreateVertexArrays = nullptr;
PFNGLNAMEDBUFFERSUBDATAPROC pglNamedBufferSubData = nullptr;

PFNGLPRIMITIVERESTARTINDEXPROC pglPrimitiveRestartIndex = nullptr;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC pglNamedFramebufferTextureLayer = nullptr;
PFNGLGETTEXTURESUBIMAGEPROC pglGetTextureSubImage = nullptr;
PFNGLGENQUERIESPROC pglGenQueries = nullptr;
PFNGLDELETEQUERIESPROC pglDeleteQueries = nullptr;
PFNGLBEGINQUERYPROC pglBeginQuery = nullptr;
PFNGLENDQUERYPROC pglEndQuery = nullptr;
PFNGLGETQUERYOBJECTUIVPROC pglGetQueryObjectuiv = nullptr;
PFNGLGETQUERYOBJECTUI64VPROC pglGetQueryObjectui64v = nullptr;
PFNGLQUERYCOUNTERPROC pglQueryCounter = nullptr;

PFNGLCREATETEXTURESPROC pglCreateTextures = nullptr;
PFNGLTEXTUREBUFFERPROC pglTextureBuffer = nullptr;
PFNGLTEXTURESTORAGE3DPROC pglTextureStorage3D = nullptr;
PFNGLCREATESAMPLERSPROC pglCreateSamplers = nullptr;
PFNGLCREATEQUERIESPROC pglCreateQueries = nullptr;

#if NYA_WIN
#define GL_GET_ADDRESS( x ) wglGetProcAddress( (const char*)x )
#elif NYA_UNIX
#define GL_GET_ADDRESS( x ) glXGetProcAddress( reinterpret_cast<const GLubyte*>( x ) )
#endif

#define LOAD_EXT( name, func )\
    ::name = reinterpret_cast<func>( GL_GET_ADDRESS( #name ) );\
    if ( !::name ) return false

bool nya::rendering::LoadOpenGLExtensions()
{
#if NYA_WIN
    LOAD_EXT( wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC );
    LOAD_EXT( wglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC );
    LOAD_EXT( wglSwapIntervalEXT, PFNWGLSWAPINTERVALEXTPROC );
#elif NYA_UNIX
    LOAD_EXT( glXSwapIntervalEXT, PFNGLXSWAPINTERVALEXTPROC );
#endif

    LOAD_EXT( glCreateTextures, PFNGLCREATETEXTURESPROC );
    LOAD_EXT( glTextureBuffer, PFNGLTEXTUREBUFFERPROC );
    LOAD_EXT( glTextureStorage3D, PFNGLTEXTURESTORAGE3DPROC );
    LOAD_EXT( glCreateSamplers, PFNGLCREATESAMPLERSPROC );
    LOAD_EXT( glCreateQueries, PFNGLCREATEQUERIESPROC );

    LOAD_EXT( glGetQueryObjectui64v, PFNGLGETQUERYOBJECTUI64VPROC );
    LOAD_EXT( glGetTextureSubImage, PFNGLGETTEXTURESUBIMAGEPROC );
    LOAD_EXT( glGenQueries, PFNGLGENQUERIESPROC );
    LOAD_EXT( glDeleteQueries, PFNGLDELETEQUERIESPROC );
    LOAD_EXT( glBeginQuery, PFNGLBEGINQUERYPROC );
    LOAD_EXT( glEndQuery, PFNGLENDQUERYPROC );
    LOAD_EXT( glGetQueryObjectuiv, PFNGLGETQUERYOBJECTUIVPROC );
    LOAD_EXT( glQueryCounter, PFNGLQUERYCOUNTERPROC );

    LOAD_EXT( glNamedFramebufferTextureLayer, PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC );
    LOAD_EXT( glPrimitiveRestartIndex, PFNGLPRIMITIVERESTARTINDEXPROC );
    LOAD_EXT( glNamedBufferSubData, PFNGLNAMEDBUFFERSUBDATAPROC );
    LOAD_EXT( glCreateVertexArrays, PFNGLCREATEVERTEXARRAYSPROC );
    LOAD_EXT( glNamedBufferData, PFNGLNAMEDBUFFERDATAPROC );
    LOAD_EXT( glCheckNamedFramebufferStatus, PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC );
    LOAD_EXT( glNamedFramebufferTexture, PFNGLNAMEDFRAMEBUFFERTEXTUREPROC );
    LOAD_EXT( glCreateFramebuffers, PFNGLCREATEFRAMEBUFFERSPROC );
    LOAD_EXT( glVertexArrayVertexBuffer, PFNGLVERTEXARRAYVERTEXBUFFERPROC );
    LOAD_EXT( glVertexArrayElementBuffer, PFNGLVERTEXARRAYELEMENTBUFFERPROC );
    LOAD_EXT( glCreateBuffers, PFNGLCREATEBUFFERSPROC );
    LOAD_EXT( glNamedBufferStorage, PFNGLNAMEDBUFFERSTORAGEPROC );
    LOAD_EXT( glVertexArrayAttribBinding, PFNGLVERTEXARRAYATTRIBBINDINGPROC );
    LOAD_EXT( glBlendColor, PFNGLBLENDCOLORPROC );
    LOAD_EXT( glClearBufferfv, PFNGLCLEARBUFFERFVPROC );
    LOAD_EXT( glShaderBinary, PFNGLSHADERBINARYPROC );
    LOAD_EXT( glSpecializeShader, PFNGLSPECIALIZESHADERPROC );
    LOAD_EXT( glVertexAttribBinding, PFNGLVERTEXATTRIBBINDINGPROC );
    LOAD_EXT( glEnableVertexArrayAttrib, PFNGLENABLEVERTEXARRAYATTRIBPROC );
    LOAD_EXT( glVertexArrayAttribFormat, PFNGLVERTEXARRAYATTRIBFORMATPROC );
    LOAD_EXT( glDrawElementsBaseVertex, PFNGLDRAWELEMENTSBASEVERTEXPROC );
    LOAD_EXT( glGenSamplers, PFNGLGENSAMPLERSPROC );
    LOAD_EXT( glBindSampler, PFNGLBINDSAMPLERPROC );
    LOAD_EXT( glDeleteSamplers, PFNGLDELETESAMPLERSPROC );
    LOAD_EXT( glSamplerParameterf, PFNGLSAMPLERPARAMETERFPROC );
    LOAD_EXT( glSamplerParameteri, PFNGLSAMPLERPARAMETERIPROC );
    LOAD_EXT( glSampleCoverage, PFNGLSAMPLECOVERAGEPROC );
    LOAD_EXT( glStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC );
    LOAD_EXT( glStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC );
    LOAD_EXT( glTexImage2DMultisample, PFNGLTEXIMAGE2DMULTISAMPLEPROC );
    LOAD_EXT( glFramebufferTexture1D, PFNGLFRAMEBUFFERTEXTURE1DPROC );
    LOAD_EXT( glFramebufferTexture3D, PFNGLFRAMEBUFFERTEXTURE3DPROC );
    LOAD_EXT( glActiveTexture, PFNGLACTIVETEXTUREPROC );
    LOAD_EXT( glCompressedTexImage2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC );
    LOAD_EXT( glBufferStorage, PFNGLBUFFERSTORAGEPROC );
    LOAD_EXT( glMapBufferRange, PFNGLMAPBUFFERRANGEPROC );
    LOAD_EXT( glBindBuffer, PFNGLBINDBUFFERPROC );
    LOAD_EXT( glBindVertexArray, PFNGLBINDVERTEXARRAYPROC );
    LOAD_EXT( glBindVertexBuffer, PFNGLBINDVERTEXBUFFERPROC );
    LOAD_EXT( glObjectLabel, PFNGLOBJECTLABELPROC );
    LOAD_EXT( glBufferData, PFNGLBUFFERDATAPROC );
    LOAD_EXT( glCompileShader, PFNGLCOMPILESHADERPROC );
    LOAD_EXT( glCreateProgram, PFNGLCREATEPROGRAMPROC );
    LOAD_EXT( glCreateShader, PFNGLCREATESHADERPROC );
    LOAD_EXT( glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
    LOAD_EXT( glDeleteProgram, PFNGLDELETEPROGRAMPROC );
    LOAD_EXT( glDeleteShader, PFNGLDELETESHADERPROC );
    LOAD_EXT( glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC );

    ::glDetachShader =
        ( PFNGLDETACHSHADERPROC )
        GL_GET_ADDRESS( "glDetachShader" );
    if ( !::glDetachShader ) {
        return false;
    }

    ::glEnableVertexAttribArray =
        ( PFNGLENABLEVERTEXATTRIBARRAYPROC )
        GL_GET_ADDRESS( "glEnableVertexAttribArray" );
    if ( !::glEnableVertexAttribArray ) {
        return false;
    }

    ::glGenBuffers =
        ( PFNGLGENBUFFERSPROC )
        GL_GET_ADDRESS( "glGenBuffers" );
    if ( !::glGenBuffers ) {
        return false;
    }

    ::glGenVertexArrays =
        ( PFNGLGENVERTEXARRAYSPROC )
        GL_GET_ADDRESS( "glGenVertexArrays" );
    if ( !::glGenVertexArrays ) {
        return false;
    }

    ::glGetAttribLocation =
        ( PFNGLGETATTRIBLOCATIONPROC )
        GL_GET_ADDRESS( "glGetAttribLocation" );
    if ( !::glGetAttribLocation ) {
        return false;
    }

    ::glGetProgramInfoLog =
        ( PFNGLGETPROGRAMINFOLOGPROC )
        GL_GET_ADDRESS( "glGetProgramInfoLog" );
    if ( !::glGetProgramInfoLog ) {
        return false;
    }

    ::glGetProgramiv =
        ( PFNGLGETPROGRAMIVPROC )
        GL_GET_ADDRESS( "glGetProgramiv" );
    if ( !::glGetProgramiv ) {
        return false;
    }

    ::glGetShaderInfoLog =
        ( PFNGLGETSHADERINFOLOGPROC )
        GL_GET_ADDRESS( "glGetShaderInfoLog" );
    if ( !::glGetShaderInfoLog ) {
        return false;
    }

    ::glGetShaderiv =
        ( PFNGLGETSHADERIVPROC )
        GL_GET_ADDRESS( "glGetShaderiv" );
    if ( !::glGetShaderiv ) {
        return false;
    }

    ::glLinkProgram =
        ( PFNGLLINKPROGRAMPROC )
        GL_GET_ADDRESS( "glLinkProgram" );
    if ( !::glLinkProgram ) {
        return false;
    }

    ::glShaderSource =
        ( PFNGLSHADERSOURCEPROC )
        GL_GET_ADDRESS( "glShaderSource" );
    if ( !::glShaderSource ) {
        return false;
    }

    ::glUseProgram =
        ( PFNGLUSEPROGRAMPROC )
        GL_GET_ADDRESS( "glUseProgram" );
    if ( !::glUseProgram ) {
        return false;
    }

    ::glVertexAttribPointer =
        ( PFNGLVERTEXATTRIBPOINTERPROC )
        GL_GET_ADDRESS( "glVertexAttribPointer" );
    if ( !::glVertexAttribPointer ) {
        return false;
    }

    ::glBindAttribLocation =
        ( PFNGLBINDATTRIBLOCATIONPROC )
        GL_GET_ADDRESS( "glBindAttribLocation" );
    if ( !::glBindAttribLocation ) {
        return false;
    }

    ::glGetUniformLocation =
        ( PFNGLGETUNIFORMLOCATIONPROC )
        GL_GET_ADDRESS( "glGetUniformLocation" );
    if ( !::glGetUniformLocation ) {
        return false;
    }

    ::glUniformMatrix2fv =
        ( PFNGLUNIFORMMATRIX2FVPROC )
        GL_GET_ADDRESS( "glUniformMatrix2fv" );
    if ( !::glUniformMatrix2fv ) {
        return false;
    }

    ::glUniformMatrix3fv =
        ( PFNGLUNIFORMMATRIX3FVPROC )
        GL_GET_ADDRESS( "glUniformMatrix3fv" );
    if ( !::glUniformMatrix3fv ) {
        return false;
    }

    ::glUniformMatrix4fv =
        ( PFNGLUNIFORMMATRIX4FVPROC )
        GL_GET_ADDRESS( "glUniformMatrix4fv" );
    if ( !::glUniformMatrix4fv ) {
        return false;
    }

    ::glUniform1i =
        ( PFNGLUNIFORM1IPROC )
        GL_GET_ADDRESS( "glUniform1i" );
    if ( !::glUniform1i ) {
        return false;
    }

    ::glUniform1f =
        ( PFNGLUNIFORM1FPROC )
        GL_GET_ADDRESS( "glUniform1f" );
    if ( !::glUniform1f ) {
        return false;
    }

    ::glUniform1iv =
        ( PFNGLUNIFORM1IVPROC )
        GL_GET_ADDRESS( "glUniform1iv" );
    if ( !::glUniform1iv ) {
        return false;
    }

    ::glUniform1fv =
        ( PFNGLUNIFORM1FVPROC )
        GL_GET_ADDRESS( "glUniform1fv" );
    if ( !::glUniform1fv ) {
        return false;
    }

    ::glGenerateMipmap =
        ( PFNGLGENERATEMIPMAPPROC )
        GL_GET_ADDRESS( "glGenerateMipmap" );
    if ( !::glGenerateMipmap ) {
        return false;
    }

    ::glDisableVertexAttribArray =
        ( PFNGLDISABLEVERTEXATTRIBARRAYPROC )
        GL_GET_ADDRESS( "glDisableVertexAttribArray" );
    if ( !::glDisableVertexAttribArray ) {
        return false;
    }

    glUniform2fv =
        ( PFNGLUNIFORM2FVPROC )
        GL_GET_ADDRESS( "glUniform2fv" );
    if ( !::glUniform2fv ) {
        return false;
    }

    glUniform2iv =
        ( PFNGLUNIFORM2IVPROC )
        GL_GET_ADDRESS( "glUniform2iv" );
    if ( !::glUniform2iv ) {
        return false;
    }

    glUniform2f =
        ( PFNGLUNIFORM2FPROC )
        GL_GET_ADDRESS( "glUniform2f" );
    if ( !::glUniform2f ) {
        return false;
    }

    glUniform2i =
        ( PFNGLUNIFORM2IPROC )
        GL_GET_ADDRESS( "glUniform2i" );
    if ( !::glUniform2i ) {
        return false;
    }

    glUniform3fv =
        ( PFNGLUNIFORM3FVPROC )
        GL_GET_ADDRESS( "glUniform3fv" );
    if ( !::glUniform3fv ) {
        return false;
    }

    glUniform3iv =
        ( PFNGLUNIFORM3IVPROC )
        GL_GET_ADDRESS( "glUniform3iv" );
    if ( !::glUniform3iv ) {
        return false;
    }

    glUniform3f =
        ( PFNGLUNIFORM3FPROC )
        GL_GET_ADDRESS( "glUniform3f" );
    if ( !::glUniform3f ) {
        return false;
    }

    glUniform3i =
        ( PFNGLUNIFORM3IPROC )
        GL_GET_ADDRESS( "glUniform3i" );
    if ( !::glUniform3i ) {
        return false;
    }

    ::glUniform4fv =
        ( PFNGLUNIFORM4FVPROC )
        GL_GET_ADDRESS( "glUniform4fv" );
    if ( !::glUniform4fv ) {
        return false;
    }

    ::glUniform4f =
        ( PFNGLUNIFORM4FPROC )
        GL_GET_ADDRESS( "glUniform4f" );
    if ( !::glUniform4f ) {
        return false;
    }

    ::glUniform4i =
        ( PFNGLUNIFORM4IPROC )
        GL_GET_ADDRESS( "glUniform4i" );
    if ( !::glUniform4i ) {
        return false;
    }

    ::glUniform4iv =
        ( PFNGLUNIFORM4IVPROC )
        GL_GET_ADDRESS( "glUniform4iv" );
    if ( !::glUniform4iv ) {
        return false;
    }

    ::glGenFramebuffers =
        ( PFNGLGENFRAMEBUFFERSPROC )
        GL_GET_ADDRESS( "glGenFramebuffers" );
    if ( !::glGenFramebuffers ) {
        return false;
    }

    ::glFramebufferTexture2D =
        ( PFNGLFRAMEBUFFERTEXTURE2DPROC )
        GL_GET_ADDRESS( "glFramebufferTexture2D" );
    if ( !::glFramebufferTexture2D ) {
        return false;
    }

    ::glDeleteFramebuffers =
        ( PFNGLDELETEFRAMEBUFFERSPROC )
        GL_GET_ADDRESS( "glDeleteFramebuffers" );
    if ( !::glDeleteFramebuffers ) {
        return false;
    }

    ::glCheckFramebufferStatus =
        ( PFNGLCHECKFRAMEBUFFERSTATUSPROC )
        GL_GET_ADDRESS( "glCheckFramebufferStatus" );
    if ( !::glCheckFramebufferStatus ) {
        return false;
    }

    ::glDrawBuffers =
        ( PFNGLDRAWBUFFERSPROC )
        GL_GET_ADDRESS( "glDrawBuffers" );
    if ( !::glDrawBuffers ) {
        return false;
    }

    ::glBindFramebuffer =
        ( PFNGLBINDFRAMEBUFFERPROC )
        GL_GET_ADDRESS( "glBindFramebuffer" );
    if ( !::glBindFramebuffer ) {
        return false;
    }

    ::glGetUniformBlockIndex =
        ( PFNGLGETUNIFORMBLOCKINDEXPROC )
        GL_GET_ADDRESS( "glGetUniformBlockIndex" );
    if ( !::glGetUniformBlockIndex ) {
        return false;
    }

    ::glUniformBlockBinding =
        ( PFNGLUNIFORMBLOCKBINDINGPROC )
        GL_GET_ADDRESS( "glUniformBlockBinding" );
    if ( !::glUniformBlockBinding ) {
        return false;
    }

    ::glTexBuffer =
        ( PFNGLTEXBUFFERPROC )
        GL_GET_ADDRESS( "glTexBuffer" );
    if ( !::glTexBuffer ) {
        return false;
    }

    ::glMemoryBarrier =
        ( PFNGLMEMORYBARRIERPROC )
        GL_GET_ADDRESS( "glMemoryBarrier" );
    if ( !::glMemoryBarrier ) {
        return false;
    }

    ::glBlitFramebuffer =
        ( PFNGLBLITFRAMEBUFFERPROC )
        GL_GET_ADDRESS( "glBlitFramebuffer" );
    if ( !::glBlitFramebuffer ) {
        return false;
    }

    ::glDispatchCompute =
        ( PFNGLDISPATCHCOMPUTEPROC )
        GL_GET_ADDRESS( "glDispatchCompute" );
    if ( !::glDispatchCompute ) {
        return false;
    }

    ::glGetProgramResourceIndex =
        ( PFNGLGETPROGRAMRESOURCEINDEXPROC )
        GL_GET_ADDRESS( "glGetProgramResourceIndex" );
    if ( !::glGetProgramResourceIndex ) {
        return false;
    }

    ::glShaderStorageBlockBinding =
        ( PFNGLSHADERSTORAGEBLOCKBINDINGPROC )
        GL_GET_ADDRESS( "glShaderStorageBlockBinding" );
    if ( !::glShaderStorageBlockBinding ) {
        return false;
    }

    ::glFramebufferTextureLayer =
        ( PFNGLFRAMEBUFFERTEXTURELAYERPROC )
        GL_GET_ADDRESS( "glFramebufferTextureLayer" );
    if ( !::glFramebufferTextureLayer ) {
        return false;
    }

    ::glBlendEquation =
        ( PFNGLBLENDEQUATIONPROC )
        GL_GET_ADDRESS( "glBlendEquation" );
    if ( !::glBlendEquation ) {
        return false;
    }

    ::glTexImage3D =
        ( PFNGLTEXIMAGE3DPROC )
        GL_GET_ADDRESS( "glTexImage3D" );
    if ( !::glTexImage3D ) {
        return false;
    }

    ::glFramebufferTexture =
        ( PFNGLFRAMEBUFFERTEXTUREPROC )
        GL_GET_ADDRESS( "glFramebufferTexture" );
    if ( !::glFramebufferTexture ) {
        return false;
    }

    ::glBlendEquationSeparate =
        ( PFNGLBLENDEQUATIONSEPARATEPROC )
        GL_GET_ADDRESS( "glBlendEquationSeparate" );
    if ( !::glBlendEquationSeparate ) {
        return false;
    }

    ::glBlendFuncSeparate =
        ( PFNGLBLENDFUNCSEPARATEPROC )
        GL_GET_ADDRESS( "glBlendFuncSeparate" );
    if ( !::glBlendFuncSeparate ) {
        return false;
    }

    ::glFramebufferParameteri = ( PFNGLFRAMEBUFFERPARAMETERIPROC )
        GL_GET_ADDRESS( "glFramebufferParameteri" );
    if ( !::glFramebufferParameteri ) {
        return false;
    }

    ::glTexStorage3D = ( PFNGLTEXSTORAGE3DPROC )
        GL_GET_ADDRESS( "glTexStorage3D" );
    if ( !::glTexStorage3D ) {
        return false;
    }

    ::glDebugMessageCallback = ( PFNGLDEBUGMESSAGECALLBACKPROC )
        GL_GET_ADDRESS( "glDebugMessageCallback" );
    if ( !::glDebugMessageCallback ) {
        return false;
    }

    ::glBindBufferBase = ( PFNGLBINDBUFFERBASEPROC )
        GL_GET_ADDRESS( "glBindBufferBase" );
    if ( !::glBindBufferBase ) {
        return false;
    }

    ::glMapBuffer = ( PFNGLMAPBUFFERPROC )
        GL_GET_ADDRESS( "glMapBuffer" );
    if ( !::glMapBuffer ) {
        return false;
    }

    ::glUnmapBuffer = ( PFNGLUNMAPBUFFERPROC )
        GL_GET_ADDRESS( "glUnmapBuffer" );
    if ( !::glUnmapBuffer ) {
        return false;
    }

    ::glAttachShader = ( PFNGLATTACHSHADERPROC )
        GL_GET_ADDRESS( "glAttachShader" );
    if ( !::glAttachShader ) {
        return false;
    }

    ::glGetTextureHandleARB = ( PFNGLGETTEXTUREHANDLEARBPROC )
        GL_GET_ADDRESS( "glGetTextureHandleARB" );
    if ( !::glGetTextureHandleARB ) {
        return false;
    }

    ::glUniformHandleui64ARB = ( PFNGLUNIFORMHANDLEUI64ARBPROC )
        GL_GET_ADDRESS( "glUniformHandleui64ARB" );
    if ( !::glUniformHandleui64ARB ) {
        return false;
    }

    ::glMakeTextureHandleResidentARB = ( PFNGLMAKETEXTUREHANDLERESIDENTARBPROC )
        GL_GET_ADDRESS( "glMakeTextureHandleResidentARB" );
    if ( !::glMakeTextureHandleResidentARB ) {
        return false;
    }

    ::glMakeTextureHandleNonResidentARB = ( PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC )
        GL_GET_ADDRESS( "glMakeTextureHandleNonResidentARB" );
    if ( !::glMakeTextureHandleNonResidentARB ) {
        return false;
    }

    ::glProgramUniformHandleui64ARB = ( PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC )
        GL_GET_ADDRESS( "glProgramUniformHandleui64ARB" );
    if ( !::glProgramUniformHandleui64ARB ) {
        return false;
    }

    ::glBindImageTexture = ( PFNGLBINDIMAGETEXTUREPROC )
        GL_GET_ADDRESS( "glBindImageTexture" );
    if ( !::glBindImageTexture ) {
        return false;
    }

    ::glClipControl = ( PFNGLCLIPCONTROLPROC )
        GL_GET_ADDRESS( "glClipControl" );
    if ( !::glClipControl ) {
        return false;
    }

    ::glFramebufferRenderbuffer = ( PFNGLFRAMEBUFFERRENDERBUFFERPROC )
        GL_GET_ADDRESS( "glFramebufferRenderbuffer" );
    if ( !::glFramebufferRenderbuffer ) {
        return false;
    }

    ::glFlushMappedBufferRange = ( PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC )
        GL_GET_ADDRESS( "glFlushMappedBufferRange" );
    if ( !::glFlushMappedBufferRange ) {
        return false;
    }

    return true;
}
#endif
