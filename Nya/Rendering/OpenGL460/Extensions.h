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
#pragma once

#if NYA_GL460
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glcorearb.h>

#if NYA_UNIX
#define GLX_GLXEXT_PROTOTYPES 1

// X11 typedef Window
#define Window WindowX11
#include <GL/glx.h>
#undef Window

#include <GL/glxext.h>
#elif NYA_WIN
#include <GL/wgl.h>
#endif

#define NYA_GL4_BUFFER_OFFSET( x ) ( char* )NULL + x

extern PFNGLDRAWELEMENTSBASEVERTEXPROC pglDrawElementsBaseVertex;
extern PFNGLATTACHSHADERPROC pglAttachShader;
extern PFNGLBINDBUFFERPROC pglBindBuffer;
extern PFNGLBINDVERTEXARRAYPROC pglBindVertexArray;
extern PFNGLBUFFERDATAPROC pglBufferData;
extern PFNGLCOMPILESHADERPROC pglCompileShader;
extern PFNGLCREATEPROGRAMPROC pglCreateProgram;
extern PFNGLCREATESHADERPROC pglCreateShader;
extern PFNGLDELETEBUFFERSPROC pglDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC pglDeleteProgram;
extern PFNGLDELETESHADERPROC pglDeleteShader;
extern PFNGLDELETEVERTEXARRAYSPROC pglDeleteVertexArrays;
extern PFNGLDETACHSHADERPROC pglDetachShader;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC pglEnableVertexAttribArray;
extern PFNGLGENBUFFERSPROC pglGenBuffers;
extern PFNGLGENVERTEXARRAYSPROC pglGenVertexArrays;
extern PFNGLGETATTRIBLOCATIONPROC pglGetAttribLocation;
extern PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC pglGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC pglGetShaderiv;
extern PFNGLLINKPROGRAMPROC pglLinkProgram;
extern PFNGLSHADERSOURCEPROC pglShaderSource;
extern PFNGLUSEPROGRAMPROC pglUseProgram;
extern PFNGLVERTEXATTRIBPOINTERPROC pglVertexAttribPointer;
extern PFNGLBINDATTRIBLOCATIONPROC pglBindAttribLocation;
extern PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation;
extern PFNGLUNIFORMMATRIX2FVPROC pglUniformMatrix2fv;
extern PFNGLUNIFORMMATRIX3FVPROC pglUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC pglUniformMatrix4fv;
extern PFNGLUNIFORM1IPROC pglUniform1i;
extern PFNGLUNIFORM1FPROC pglUniform1f;
extern PFNGLUNIFORM1IVPROC pglUniform1iv;
extern PFNGLUNIFORM1FVPROC pglUniform1fv;
extern PFNGLGENERATEMIPMAPPROC pglGenerateMipmap;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC pglDisableVertexAttribArray;
extern PFNGLUNIFORM2IPROC pglUniform2i;
extern PFNGLUNIFORM2FPROC pglUniform2f;
extern PFNGLUNIFORM2IVPROC pglUniform2iv;
extern PFNGLUNIFORM2FVPROC pglUniform2fv;
extern PFNGLUNIFORM3IPROC pglUniform3i;
extern PFNGLUNIFORM3FPROC pglUniform3f;
extern PFNGLUNIFORM3IVPROC pglUniform3iv;
extern PFNGLUNIFORM3FVPROC pglUniform3fv;
extern PFNGLUNIFORM4FPROC pglUniform4f;
extern PFNGLUNIFORM4IPROC pglUniform4i;
extern PFNGLUNIFORM4IVPROC pglUniform4iv;
extern PFNGLUNIFORM4FVPROC pglUniform4fv;
extern PFNGLGENFRAMEBUFFERSPROC pglGenFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC pglFramebufferTexture2D;
extern PFNGLDELETEFRAMEBUFFERSPROC pglDeleteFramebuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC pglCheckFramebufferStatus;
extern PFNGLDRAWBUFFERSPROC pglDrawBuffers;
extern PFNGLBINDFRAMEBUFFERPROC pglBindFramebuffer;
extern PFNGLGETUNIFORMBLOCKINDEXPROC pglGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC pglUniformBlockBinding;
extern PFNGLBINDIMAGETEXTUREPROC pglBindImageTexture;
extern PFNGLTEXBUFFERPROC pglTexBuffer;
extern PFNGLMEMORYBARRIERPROC pglMemoryBarrier;
extern PFNGLBLITFRAMEBUFFERPROC pglBlitFramebuffer;
extern PFNGLDISPATCHCOMPUTEPROC pglDispatchCompute;
extern PFNGLGETPROGRAMRESOURCEINDEXPROC pglGetProgramResourceIndex;
extern PFNGLSHADERSTORAGEBLOCKBINDINGPROC pglShaderStorageBlockBinding;
extern PFNGLFRAMEBUFFERTEXTURELAYERPROC pglFramebufferTextureLayer;
extern PFNGLBLENDEQUATIONPROC pglBlendEquation;
extern PFNGLTEXIMAGE3DPROC pglTexImage3D;
extern PFNGLFRAMEBUFFERTEXTUREPROC pglFramebufferTexture;
extern PFNGLBLENDEQUATIONSEPARATEPROC pglBlendEquationSeparate;
extern PFNGLBLENDFUNCSEPARATEPROC pglBlendFuncSeparate;
extern PFNGLFRAMEBUFFERPARAMETERIPROC pglFramebufferParameteri;
extern PFNGLTEXSTORAGE3DPROC pglTexStorage3D;
extern PFNGLACTIVETEXTUREPROC pglActiveTexture;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC pglCompressedTexImage2D;
extern PFNGLBINDBUFFERBASEPROC pglBindBufferBase;
extern PFNGLMAPBUFFERPROC pglMapBuffer;
extern PFNGLUNMAPBUFFERPROC pglUnmapBuffer;
extern PFNGLBUFFERSTORAGEPROC pglBufferStorage;
extern PFNGLMAPBUFFERRANGEPROC pglMapBufferRange;
extern PFNGLDEBUGMESSAGECALLBACKPROC pglDebugMessageCallback;

extern PFNGLGETTEXTUREHANDLEARBPROC pglGetTextureHandleARB;
extern PFNGLUNIFORMHANDLEUI64ARBPROC pglUniformHandleui64ARB;
extern PFNGLMAKETEXTUREHANDLERESIDENTARBPROC pglMakeTextureHandleResidentARB;
extern PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC pglMakeTextureHandleNonResidentARB;
extern PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC pglProgramUniformHandleui64ARB;
extern PFNGLCLIPCONTROLPROC pglClipControl;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC pglFramebufferRenderbuffer;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC pglFramebufferRenderbuffer;

extern PFNGLCREATETEXTURESPROC pglCreateTextures;
extern PFNGLTEXTUREBUFFERPROC pglTextureBuffer;
extern PFNGLTEXTURESTORAGE3DPROC pglTextureStorage3D;
extern PFNGLCREATESAMPLERSPROC pglCreateSamplers;
extern PFNGLCREATEQUERIESPROC pglCreateQueries;

#if NYA_WIN
    extern PFNWGLCHOOSEPIXELFORMATARBPROC pwglChoosePixelFormatARB;
    extern PFNWGLCREATECONTEXTATTRIBSARBPROC pwglCreateContextAttribsARB;
    extern PFNWGLSWAPINTERVALEXTPROC pwglSwapIntervalEXT;
#elif NYA_UNIX
    extern PFNGLXSWAPINTERVALEXTPROC pglXSwapIntervalEXT;
#endif

extern PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC pglFlushMappedBufferRange;
extern PFNGLTEXIMAGE2DMULTISAMPLEPROC pglTexImage2DMultisample;
extern PFNGLFRAMEBUFFERTEXTURE1DPROC pglFramebufferTexture1D;
extern PFNGLFRAMEBUFFERTEXTURE3DPROC pglFramebufferTexture3D;
extern PFNGLSAMPLECOVERAGEPROC pglSampleCoverage;
extern PFNGLSTENCILOPSEPARATEPROC pglStencilOpSeparate;
extern PFNGLSTENCILFUNCSEPARATEPROC pglStencilFuncSeparate;

extern PFNGLBINDVERTEXBUFFERPROC pglBindVertexBuffer;
extern PFNGLOBJECTLABELPROC pglObjectLabel;

extern PFNGLGENSAMPLERSPROC pglGenSamplers;
extern PFNGLBINDSAMPLERPROC pglBindSampler;
extern PFNGLDELETESAMPLERSPROC pglDeleteSamplers;
extern PFNGLSAMPLERPARAMETERFPROC pglSamplerParameterf;
extern PFNGLSAMPLERPARAMETERIPROC pglSamplerParameteri;
extern PFNGLVERTEXARRAYATTRIBFORMATPROC pglVertexArrayAttribFormat;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC pglEnableVertexArrayAttrib;
extern PFNGLVERTEXATTRIBBINDINGPROC pglVertexAttribBinding;
extern PFNGLSHADERBINARYPROC pglShaderBinary;
extern PFNGLSPECIALIZESHADERPROC pglSpecializeShader;
extern PFNGLCLEARBUFFERFVPROC pglClearBufferfv;

extern PFNGLBLENDCOLORPROC pglBlendColor;
extern PFNGLVERTEXARRAYATTRIBBINDINGPROC pglVertexArrayAttribBinding;
extern PFNGLNAMEDBUFFERSTORAGEPROC pglNamedBufferStorage;
extern PFNGLCREATEBUFFERSPROC pglCreateBuffers;
extern PFNGLVERTEXARRAYELEMENTBUFFERPROC pglVertexArrayElementBuffer;
extern PFNGLVERTEXARRAYVERTEXBUFFERPROC pglVertexArrayVertexBuffer;
extern PFNGLCREATEFRAMEBUFFERSPROC pglCreateFramebuffers;

extern PFNGLNAMEDFRAMEBUFFERTEXTUREPROC pglNamedFramebufferTexture;
extern PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC pglCheckNamedFramebufferStatus;
extern PFNGLNAMEDBUFFERDATAPROC pglNamedBufferData;
extern PFNGLCREATEVERTEXARRAYSPROC pglCreateVertexArrays;
extern PFNGLNAMEDBUFFERSUBDATAPROC pglNamedBufferSubData;
extern PFNGLPRIMITIVERESTARTINDEXPROC pglPrimitiveRestartIndex;
extern PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC pglNamedFramebufferTextureLayer;
extern PFNGLGETTEXTURESUBIMAGEPROC pglGetTextureSubImage;
extern PFNGLGENQUERIESPROC pglGenQueries;
extern PFNGLDELETEQUERIESPROC pglDeleteQueries;
extern PFNGLBEGINQUERYPROC pglBeginQuery;
extern PFNGLENDQUERYPROC pglEndQuery;
extern PFNGLGETQUERYOBJECTUIVPROC pglGetQueryObjectuiv;
extern PFNGLQUERYCOUNTERPROC pglQueryCounter;
extern PFNGLGETQUERYOBJECTUI64VPROC pglGetQueryObjectui64v;

#define glGetTextureSubImage pglGetTextureSubImage
#define glGenQueries pglGenQueries
#define glDeleteQueries pglDeleteQueries
#define glBeginQuery pglBeginQuery
#define glEndQuery pglEndQuery
#define glGetQueryObjectuiv pglGetQueryObjectuiv
#define glQueryCounter pglQueryCounter

#define glNamedFramebufferTextureLayer pglNamedFramebufferTextureLayer
#define glPrimitiveRestartIndex pglPrimitiveRestartIndex

#define glNamedBufferSubData pglNamedBufferSubData
#define glCreateVertexArrays pglCreateVertexArrays
#define glNamedBufferData pglNamedBufferData

#define glNamedFramebufferTexture pglNamedFramebufferTexture
#define glCheckNamedFramebufferStatus pglCheckNamedFramebufferStatus

#define glCreateFramebuffers pglCreateFramebuffers
#define glVertexArrayVertexBuffer pglVertexArrayVertexBuffer
#define glVertexArrayElementBuffer pglVertexArrayElementBuffer
#define glCreateBuffers pglCreateBuffers
#define glNamedBufferStorage pglNamedBufferStorage
#define glVertexArrayAttribBinding pglVertexArrayAttribBinding
#define glBlendColor pglBlendColor
#define glClearBufferfv pglClearBufferfv
#define glShaderBinary pglShaderBinary
#define glSpecializeShader pglSpecializeShader
#define glVertexAttribBinding pglVertexAttribBinding
#define glEnableVertexArrayAttrib pglEnableVertexArrayAttrib
#define glVertexArrayAttribFormat pglVertexArrayAttribFormat
#define glDrawElementsBaseVertex pglDrawElementsBaseVertex
#define glGenSamplers pglGenSamplers
#define glBindSampler pglBindSampler
#define glDeleteSamplers pglDeleteSamplers
#define glSamplerParameterf pglSamplerParameterf
#define glSamplerParameteri pglSamplerParameteri
#define glObjectLabel pglObjectLabel
#define glBindVertexBuffer pglBindVertexBuffer
#define glStencilOpSeparate pglStencilOpSeparate
#define glStencilFuncSeparate pglStencilFuncSeparate
#define glSampleCoverage pglSampleCoverage
#define glAttachShader pglAttachShader
#define glBindBuffer pglBindBuffer
#define glBindVertexArray pglBindVertexArray
#define glBufferData pglBufferData
#define glCompileShader pglCompileShader
#define glCreateProgram pglCreateProgram
#define glCreateShader pglCreateShader
#define glDeleteBuffers pglDeleteBuffers
#define glDeleteProgram pglDeleteProgram
#define glDeleteShader pglDeleteShader
#define glDeleteVertexArrays pglDeleteVertexArrays
#define glDetachShader pglDetachShader
#define glEnableVertexAttribArray pglEnableVertexAttribArray
#define glGenBuffers pglGenBuffers
#define glGenVertexArrays pglGenVertexArrays
#define glGetAttribLocation pglGetAttribLocation
#define glGetProgramInfoLog pglGetProgramInfoLog
#define glGetProgramiv pglGetProgramiv
#define glGetShaderInfoLog pglGetShaderInfoLog
#define glGetShaderiv pglGetShaderiv
#define glLinkProgram pglLinkProgram
#define glShaderSource pglShaderSource
#define glUseProgram pglUseProgram
#define glVertexAttribPointer pglVertexAttribPointer
#define glBindAttribLocation pglBindAttribLocation
#define glGetUniformLocation pglGetUniformLocation
#define glUniformMatrix2fv pglUniformMatrix2fv
#define glUniformMatrix3fv pglUniformMatrix3fv
#define glUniformMatrix4fv pglUniformMatrix4fv
#define glUniform1i pglUniform1i
#define glUniform1f pglUniform1f
#define glUniform1iv pglUniform1iv
#define glUniform1fv pglUniform1fv
#define glGenerateMipmap pglGenerateMipmap
#define glDisableVertexAttribArray pglDisableVertexAttribArray
#define glUniform2i pglUniform2i
#define glUniform2f pglUniform2f
#define glUniform2iv pglUniform2iv
#define glUniform2fv pglUniform2fv
#define glUniform3i pglUniform3i
#define glUniform3f pglUniform3f
#define glUniform3iv pglUniform3iv
#define glUniform3fv pglUniform3fv
#define glUniform4f pglUniform4f
#define glUniform4i pglUniform4i
#define glUniform4iv pglUniform4iv
#define glUniform4fv pglUniform4fv
#define glGenFramebuffers pglGenFramebuffers
#define glFramebufferTexture2D pglFramebufferTexture2D
#define glDeleteFramebuffers pglDeleteFramebuffers
#define glCheckFramebufferStatus pglCheckFramebufferStatus
#define glDrawBuffers pglDrawBuffers
#define glBindFramebuffer pglBindFramebuffer
#define glMapBuffer pglMapBuffer
#define glUnmapBuffer pglUnmapBuffer
#define glBindBufferBase pglBindBufferBase
#define glGetUniformBlockIndex pglGetUniformBlockIndex
#define glUniformBlockBinding pglUniformBlockBinding
#define glBindImageTexture pglBindImageTexture
#define glTexBuffer pglTexBuffer
#define glMemoryBarrier pglMemoryBarrier
#define glGetBufferSubData pglGetBufferSubData
#define glBlitFramebuffer pglBlitFramebuffer
#define glDispatchCompute pglDispatchCompute
#define glGetProgramResourceIndex pglGetProgramResourceIndex
#define glShaderStorageBlockBinding pglShaderStorageBlockBinding
#define glFramebufferTextureLayer pglFramebufferTextureLayer
#define glBlendEquation pglBlendEquation
#define glTexImage3D pglTexImage3D
#define glFramebufferTexture pglFramebufferTexture
#define glBlendEquationSeparate pglBlendEquationSeparate
#define glBlendFuncSeparate pglBlendFuncSeparate
#define glFramebufferParameteri pglFramebufferParameteri
#define glTexStorage3D pglTexStorage3D
#define glActiveTexture pglActiveTexture
#define glCompressedTexImage2D pglCompressedTexImage2D
#define glBufferStorage pglBufferStorage
#define glMapBufferRange pglMapBufferRange
#define glDebugMessageCallback pglDebugMessageCallback
#define glGetTextureHandleARB pglGetTextureHandleARB
#define glUniformHandleui64ARB pglUniformHandleui64ARB
#define glMakeTextureHandleResidentARB pglMakeTextureHandleResidentARB
#define glMakeTextureHandleNonResidentARB pglMakeTextureHandleNonResidentARB
#define glProgramUniformHandleui64ARB pglProgramUniformHandleui64ARB
#define glClipControl pglClipControl
#define glTexImage2DMultisample pglTexImage2DMultisample
#define glFramebufferRenderbuffer pglFramebufferRenderbuffer
#define glGetQueryObjectui64v pglGetQueryObjectui64v

#define glFlushMappedBufferRange pglFlushMappedBufferRange
#define glFramebufferTexture1D pglFramebufferTexture1D
#define glFramebufferTexture3D pglFramebufferTexture3D

#define glCreateTextures pglCreateTextures
#define glTextureBuffer pglTextureBuffer
#define glTextureStorage3D pglTextureStorage3D
#define glCreateSamplers pglCreateSamplers
#define glCreateQueries pglCreateQueries

#if NYA_WIN
    #define wglChoosePixelFormatARB pwglChoosePixelFormatARB
    #define wglCreateContextAttribsARB pwglCreateContextAttribsARB
    #define wglSwapIntervalEXT pwglSwapIntervalEXT
#elif NYA_UNIX
    #define glXSwapIntervalEXT pglXSwapIntervalEXT
#endif

namespace nya
{
    namespace rendering
    {
        bool LoadOpenGLExtensions();
    }
}
#endif
