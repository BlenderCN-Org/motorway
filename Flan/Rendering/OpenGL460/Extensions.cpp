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
#include "Extensions.h"

PFNGLVERTEXARRAYATTRIBBINDINGPROC pglVertexArrayAttribBinding = NULL;
PFNGLBLENDCOLORPROC pglBlendColor = NULL;
PFNGLDRAWELEMENTSBASEVERTEXPROC pglDrawElementsBaseVertex = NULL;
PFNGLATTACHSHADERPROC pglAttachShader = NULL;
PFNGLBINDBUFFERPROC pglBindBuffer = NULL;
PFNGLBINDVERTEXARRAYPROC pglBindVertexArray = NULL;
PFNGLBUFFERDATAPROC pglBufferData = NULL;
PFNGLCOMPILESHADERPROC pglCompileShader = NULL;
PFNGLCREATEPROGRAMPROC pglCreateProgram = NULL;
PFNGLCREATESHADERPROC pglCreateShader = NULL;
PFNGLDELETEBUFFERSPROC pglDeleteBuffers = NULL;
PFNGLDELETEPROGRAMPROC pglDeleteProgram = NULL;
PFNGLDELETESHADERPROC pglDeleteShader = NULL;
PFNGLDELETEVERTEXARRAYSPROC pglDeleteVertexArrays = NULL;
PFNGLDETACHSHADERPROC pglDetachShader = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC pglEnableVertexAttribArray = NULL;
PFNGLGENBUFFERSPROC pglGenBuffers = NULL;
PFNGLGENVERTEXARRAYSPROC pglGenVertexArrays = NULL;
PFNGLGETATTRIBLOCATIONPROC pglGetAttribLocation = NULL;
PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC pglGetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog = NULL;
PFNGLGETSHADERIVPROC pglGetShaderiv = NULL;
PFNGLLINKPROGRAMPROC pglLinkProgram = NULL;
PFNGLSHADERSOURCEPROC pglShaderSource = NULL;
PFNGLUSEPROGRAMPROC pglUseProgram = NULL;
PFNGLVERTEXATTRIBPOINTERPROC pglVertexAttribPointer = NULL;
PFNGLBINDATTRIBLOCATIONPROC pglBindAttribLocation = NULL;
PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation = NULL;
PFNGLUNIFORMMATRIX2FVPROC pglUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC pglUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC pglUniformMatrix4fv = NULL;
PFNGLACTIVETEXTUREPROC pglActiveTexture = NULL;
PFNGLUNIFORM1IPROC pglUniform1i = NULL;
PFNGLUNIFORM1FPROC pglUniform1f = NULL;
PFNGLUNIFORM1IVPROC pglUniform1iv = NULL;
PFNGLUNIFORM1FVPROC pglUniform1fv = NULL;
PFNGLUNIFORM2IPROC pglUniform2i = NULL;
PFNGLUNIFORM2FPROC pglUniform2f = NULL;
PFNGLUNIFORM2IVPROC pglUniform2iv = NULL;
PFNGLUNIFORM2FVPROC pglUniform2fv = NULL;
PFNGLGENERATEMIPMAPPROC pglGenerateMipmap = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC pglDisableVertexAttribArray = NULL;
PFNGLUNIFORM3IPROC pglUniform3i = NULL;
PFNGLUNIFORM3FPROC pglUniform3f = NULL;
PFNGLUNIFORM3IVPROC pglUniform3iv = NULL;
PFNGLUNIFORM3FVPROC pglUniform3fv = NULL;
PFNGLUNIFORM4IPROC pglUniform4i = NULL;
PFNGLUNIFORM4FPROC pglUniform4f = NULL;
PFNGLUNIFORM4IVPROC pglUniform4iv = NULL;
PFNGLUNIFORM4FVPROC pglUniform4fv = NULL;
PFNGLGENFRAMEBUFFERSPROC pglGenFramebuffers = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC pglFramebufferTexture2D = NULL;
PFNGLDELETEFRAMEBUFFERSPROC pglDeleteFramebuffers = NULL;
PFNGLDRAWBUFFERSPROC pglDrawBuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC pglBindFramebuffer = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC pglCheckFramebufferStatus = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC pglGetUniformBlockIndex = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC pglUniformBlockBinding = NULL;
PFNGLMEMORYBARRIERPROC pglMemoryBarrier = NULL;
PFNGLTEXBUFFERPROC pglTexBuffer = NULL;
PFNGLBLITFRAMEBUFFERPROC pglBlitFramebuffer = NULL;
PFNGLDISPATCHCOMPUTEPROC pglDispatchCompute = NULL;
PFNGLGETPROGRAMRESOURCEINDEXPROC pglGetProgramResourceIndex = NULL;
PFNGLSHADERSTORAGEBLOCKBINDINGPROC pglShaderStorageBlockBinding = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERPROC pglFramebufferTextureLayer = NULL;
PFNGLBLENDEQUATIONPROC pglBlendEquation = NULL;
PFNGLTEXIMAGE3DPROC pglTexImage3D = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC pglFramebufferTexture = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC pglBlendEquationSeparate = NULL;
PFNGLBLENDFUNCSEPARATEPROC pglBlendFuncSeparate = NULL;
PFNGLFRAMEBUFFERPARAMETERIPROC pglFramebufferParameteri = NULL;
PFNGLTEXSTORAGE3DPROC pglTexStorage3D = NULL;
PFNGLBINDBUFFERBASEPROC pglBindBufferBase = NULL;
PFNGLMAPBUFFERPROC pglMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC pglUnmapBuffer = NULL;
PFNGLBINDIMAGETEXTUREPROC pglBindImageTexture = NULL;
PFNGLBUFFERSTORAGEPROC pglBufferStorage = NULL;
PFNGLMAPBUFFERRANGEPROC pglMapBufferRange = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC pglCompressedTexImage2D = NULL;
PFNGLDEBUGMESSAGECALLBACKPROC pglDebugMessageCallback = NULL;
PFNGLGETTEXTUREHANDLEARBPROC pglGetTextureHandleARB = NULL;
PFNGLUNIFORMHANDLEUI64ARBPROC pglUniformHandleui64ARB = NULL;
PFNGLMAKETEXTUREHANDLERESIDENTARBPROC pglMakeTextureHandleResidentARB = NULL;
PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC pglMakeTextureHandleNonResidentARB = NULL;
PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC pglProgramUniformHandleui64ARB = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC pglFramebufferRenderbuffer = NULL;
PFNGLCLIPCONTROLPROC pglClipControl = NULL;
PFNGLDELETETEXTURESPROC pglDeleteTextures = NULL;
PFNGLTEXIMAGE2DMULTISAMPLEPROC pglTexImage2DMultisample = NULL;

#if FLAN_WIN
    PFNWGLCHOOSEPIXELFORMATARBPROC pwglChoosePixelFormatARB = NULL;
    PFNWGLCREATECONTEXTATTRIBSARBPROC pwglCreateContextAttribsARB = NULL;
    PFNWGLSWAPINTERVALEXTPROC pwglSwapIntervalEXT = NULL;
#elif FLAN_UNIX
    PFNGLXSWAPINTERVALEXTPROC pglXSwapIntervalEXT = NULL;
#endif

PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC pglFlushMappedBufferRange = NULL;
PFNGLFRAMEBUFFERTEXTURE1DPROC pglFramebufferTexture1D = NULL;
PFNGLFRAMEBUFFERTEXTURE3DPROC pglFramebufferTexture3D = NULL;

PFNGLSAMPLECOVERAGEPROC pglSampleCoverage = NULL;
PFNGLSTENCILOPSEPARATEPROC pglStencilOpSeparate = NULL;
PFNGLSTENCILFUNCSEPARATEPROC pglStencilFuncSeparate = NULL;

PFNGLBINDVERTEXBUFFERPROC pglBindVertexBuffer = NULL;
PFNGLOBJECTLABELPROC pglObjectLabel = NULL;

PFNGLGENSAMPLERSPROC pglGenSamplers = NULL;
PFNGLBINDSAMPLERPROC pglBindSampler = NULL;
PFNGLDELETESAMPLERSPROC pglDeleteSamplers = NULL;
PFNGLSAMPLERPARAMETERFPROC pglSamplerParameterf = NULL;
PFNGLSAMPLERPARAMETERIPROC pglSamplerParameteri = NULL;
PFNGLVERTEXARRAYATTRIBFORMATPROC pglVertexArrayAttribFormat = NULL;
PFNGLENABLEVERTEXARRAYATTRIBPROC pglEnableVertexArrayAttrib = NULL;
PFNGLVERTEXATTRIBBINDINGPROC pglVertexAttribBinding = NULL;

PFNGLSHADERBINARYPROC pglShaderBinary = NULL;
PFNGLSPECIALIZESHADERPROC pglSpecializeShader = NULL;

PFNGLCLEARBUFFERFVPROC pglClearBufferfv = NULL;
PFNGLNAMEDBUFFERSTORAGEPROC pglNamedBufferStorage = NULL;
PFNGLCREATEBUFFERSPROC pglCreateBuffers = NULL;
PFNGLVERTEXARRAYELEMENTBUFFERPROC pglVertexArrayElementBuffer = NULL;
PFNGLVERTEXARRAYVERTEXBUFFERPROC pglVertexArrayVertexBuffer = NULL;
PFNGLCREATEFRAMEBUFFERSPROC pglCreateFramebuffers = NULL;

PFNGLNAMEDFRAMEBUFFERTEXTUREPROC pglNamedFramebufferTexture = NULL;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC pglCheckNamedFramebufferStatus = NULL;

PFNGLNAMEDBUFFERDATAPROC pglNamedBufferData = NULL;
PFNGLCREATEVERTEXARRAYSPROC pglCreateVertexArrays = NULL;
PFNGLNAMEDBUFFERSUBDATAPROC pglNamedBufferSubData = NULL;

PFNGLPRIMITIVERESTARTINDEXPROC pglPrimitiveRestartIndex = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC pglNamedFramebufferTextureLayer = NULL;
PFNGLGETTEXTURESUBIMAGEPROC pglGetTextureSubImage = NULL;
PFNGLGENQUERIESPROC pglGenQueries = NULL;
PFNGLDELETEQUERIESPROC pglDeleteQueries = NULL;
PFNGLBEGINQUERYPROC pglBeginQuery = NULL;
PFNGLENDQUERYPROC pglEndQuery = NULL;
PFNGLGETQUERYOBJECTUIVPROC pglGetQueryObjectuiv = NULL;
PFNGLGETQUERYOBJECTUI64VPROC pglGetQueryObjectui64v = NULL;
PFNGLQUERYCOUNTERPROC pglQueryCounter = NULL;

PFNGLCREATETEXTURESPROC pglCreateTextures = NULL;
PFNGLTEXTUREBUFFERPROC pglTextureBuffer = NULL;
PFNGLTEXTURESTORAGE3DPROC pglTextureStorage3D = NULL;
PFNGLCREATESAMPLERSPROC pglCreateSamplers = NULL;
PFNGLCREATEQUERIESPROC pglCreateQueries = NULL;

#if FLAN_WIN
#define GL_GET_ADDRESS( x ) wglGetProcAddress( (const char*)x )
#elif FLAN_UNIX
#define GL_GET_ADDRESS( x ) glXGetProcAddress( (const GLubyte*)x )
#endif

#define LOAD_EXT( name, func )\
    ::name = ( func )GL_GET_ADDRESS( #name );\
    if ( !::name ) {\
        return false;\
    }

bool flan::rendering::LoadOpenGLExtensions()
{
#if FLAN_WIN
    LOAD_EXT( wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC );
    LOAD_EXT( wglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC );
    LOAD_EXT( wglSwapIntervalEXT, PFNWGLSWAPINTERVALEXTPROC );
#elif FLAN_UNIX
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
