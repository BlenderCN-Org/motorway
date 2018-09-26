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
#include "RenderContext.h"

#include <Core/Display/DisplaySurface.h>

#include <Rendering/RenderTarget.h>
#include <Rendering/PrimitiveTopologies.h>

#include "Extensions.h"
#include "Texture.h"
#include "PipelineState.h"

// GLX/WGL Implementations
#if FLAN_UNIX
#include <Core/Display/DisplaySurfaceXcb.h>

NativeRenderContext* CreateRenderContextSysImpl( DisplaySurface* surface )
{
    FLAN_CLOG << "Creating render context (GLX)" << std::endl;

    Display* tmpDisp = XOpenDisplay( nullptr );

    if ( tmpDisp == nullptr ) {
        FLAN_CERR << "Failed to open Xorg root display" << std::endl;
        return nullptr;
    }

    auto defaultScreen = DefaultScreen( tmpDisp );

    constexpr int32_t backbufferAttributes[] = {
       GLX_X_RENDERABLE, True,
       GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
       GLX_RENDER_TYPE, GLX_RGBA_BIT,
       GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
       GLX_RED_SIZE, 8,
       GLX_GREEN_SIZE, 8,
       GLX_BLUE_SIZE, 8,
       GLX_ALPHA_SIZE, 8,
       GLX_DEPTH_SIZE, 24,
       GLX_STENCIL_SIZE, 8,
       GLX_DOUBLEBUFFER, True,
       None
    };

    int glxMajor = -1, glxMinor = -1;
    if( !glXQueryVersion( tmpDisp, &glxMajor, &glxMinor ) ) {
        FLAN_CERR << "Failed to query glX version (missing glX packet(s)?)" << std::endl;
        return nullptr;
    }

    FLAN_CLOG << "glX version: " << glxMajor << "." << glxMinor << std::endl;

    int32_t backbufferConfigCount = 0;
    GLXFBConfig* backbufferConfigs = glXChooseFBConfig( tmpDisp, defaultScreen, backbufferAttributes, &backbufferConfigCount );

    if( backbufferConfigs == nullptr || backbufferConfigCount <= 0 ) {
        FLAN_CERR << "Could not find a suitable backbuffer configuration" << std::endl;
        return nullptr;
    }

    FLAN_CLOG << "glXChooseFBConfig: " << backbufferConfigCount << " configuration(s) available" << std::endl;
    int32_t bestConfigId = -1, worstConfigId = -1, bestSampNum = -1, worstSampNum = std::numeric_limits<int>::max();

    VisualID visualID = 0;
    for( int32_t i = 0; i < backbufferConfigCount; ++i ) {
        XVisualInfo* visualInfos = glXGetVisualFromFBConfig( tmpDisp, backbufferConfigs[i] );

        if ( visualInfos != nullptr ) {
            int32_t sampleBuffers = 0, samples = 0;

            glXGetFBConfigAttrib( tmpDisp, backbufferConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers );
            glXGetFBConfigAttrib( tmpDisp, backbufferConfigs[i], GLX_SAMPLES, &samples );

            FLAN_CLOG << "glXGetVisualFromFBConfig [" << i << "] => " << sampleBuffers << " | " << samples << std::endl;

            if ( bestConfigId < 0 || ( sampleBuffers != 0 && samples > bestSampNum ) ) {
                visualID = visualInfos->visualid;
                bestConfigId = i;
                bestSampNum = samples;
            }

            if ( worstConfigId < 0 || sampleBuffers <= 0 || samples < worstSampNum ) {
                worstConfigId = i;
                worstSampNum = samples;
            }
        }

        XFree( visualInfos );
    }

    FLAN_CLOG << "Selected [" << bestConfigId << "]" << std::endl;

    GLXFBConfig backbufferChoosenConfig = backbufferConfigs[bestConfigId];
    XFree( backbufferConfigs );

    glXGetFBConfigAttrib( tmpDisp, backbufferChoosenConfig, GLX_VISUAL_ID , reinterpret_cast<int*>( &visualID ) );

    constexpr int32_t ATTRIBUTES[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB,
        4,
        GLX_CONTEXT_MINOR_VERSION_ARB,
        6,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
    };

    // Retrieve function pointer before loading extension list
    const GLubyte* procname = reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB");

    auto glXCreateContextAttribs = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>( glXGetProcAddress( procname ) );
    if ( glXCreateContextAttribs == nullptr ) {
        FLAN_CERR << "glX context creation failed! (failed to retrieve glXCreateContextAttribs)" << std::endl;
        return nullptr;
    }

    auto context = glXCreateContextAttribs( tmpDisp, backbufferChoosenConfig, nullptr, True, ATTRIBUTES );
    if ( context == nullptr ) {
        FLAN_CERR << "glX context creation failed! (backbuffer configuration is probably incorrect)" << std::endl;
        return nullptr;
    }

    auto nativeDisplaySurface = surface->getNativeDisplaySurface();
    GLXWindow tmpWin = glXCreateWindow( tmpDisp, backbufferChoosenConfig, nativeDisplaySurface->WindowInstance, nullptr );

    if ( tmpWin <= 0 ) {
        FLAN_CERR << "glX window creation failed! (bad colormap provided)" << std::endl;
        return nullptr;
    }

    glXMakeContextCurrent( tmpDisp, tmpWin, tmpWin, context );

    if ( !flan::rendering::LoadOpenGLExtensions() ) {
        FLAN_CERR << "An error happened during OpenGL extension loading (one or several extension might be missing on your system)" << std::endl;
        return nullptr;
    }

    FLAN_CLOG << "OpenGL Version : " << glGetString( GL_VERSION ) << std::endl;
    FLAN_CLOG << "Context creation done! Moving on..." << std::endl;

    // Use [0..1] depth range
    glClipControl( GL_LOWER_LEFT, GL_ZERO_TO_ONE );

    NativeRenderContext* nativeRenderContext = new NativeRenderContext();
    nativeRenderContext->X11Display = tmpDisp;
    nativeRenderContext->DrawContext = tmpWin;

    return nativeRenderContext;
}

void flan::rendering::PresentImpl( NativeRenderContext* nativeRenderContext )
{
    glXSwapBuffers( nativeRenderContext->X11Display, nativeRenderContext->DrawContext );
}

void flan::rendering::SetVSyncStateImpl( NativeRenderContext* nativeRenderContext, const bool enabled )
{
    glXSwapIntervalEXT( nativeRenderContext->X11Display, nativeRenderContext->DrawContext, ( enabled ) ? 1 : 0 );
}
#elif FLAN_WIN
#include <Core/Display/DisplaySurfaceWin32.h>

LRESULT CALLBACK FakeWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc( hwnd, msg, wParam, lParam );
}

NativeRenderContext* CreateRenderContextSysImpl( DisplaySurface* surface )
{
    FLAN_CLOG << "Creating render context (WGL)" << std::endl;

    auto fakeFrameName = FLAN_STRING( "FlanGLFakeFrame" );

    // fake window creation to test the environement
    WNDCLASSEX wc = {};
    HINSTANCE hInstance = GetModuleHandle( NULL );

    wc.cbSize = sizeof( WNDCLASSEX );
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = FakeWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( hInstance, IDI_APPLICATION );
    wc.hIconSm = LoadIcon( hInstance, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = ( HBRUSH )( COLOR_MENUBAR + 1 );
    wc.lpszMenuName = NULL;
    wc.lpszClassName = fakeFrameName;

    if ( !RegisterClassEx( &wc ) ) {
        return nullptr;
    }

    HWND fakeWindow = CreateWindow( fakeFrameName, FLAN_STRING( "FlanGLFakeWindow" ), WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL );
    HDC fakeHdc = GetDC( fakeWindow );

    PIXELFORMATDESCRIPTOR pixelFormat =
    {
        sizeof( PIXELFORMATDESCRIPTOR ),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixelFormatResult = ChoosePixelFormat( fakeHdc, &pixelFormat );
    if ( !SetPixelFormat( fakeHdc, pixelFormatResult, &pixelFormat ) ) {
        return nullptr;
    }

    HGLRC fakeHglrc = wglCreateContext( fakeHdc );
    wglMakeCurrent( fakeHdc, fakeHglrc );

    if ( !flan::rendering::LoadOpenGLExtensions() ) {
        FLAN_CERR << "An error happened during OpenGL extension loading (one or several extension might be missing on your system)" << std::endl;
        return nullptr;
    }

    FLAN_CLOG << "Loaded extension list! Now destroying fake GL context..." << std::endl;

    wglMakeCurrent( NULL, NULL );
    UnregisterClass( fakeFrameName, hInstance );
    wglDeleteContext( fakeHglrc );
    ReleaseDC( fakeWindow, fakeHdc );
    DestroyWindow( fakeWindow );

    int32_t backbufferAttributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 24,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0
    };

    int32_t context[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        4,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        6,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,
        WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB,
        WGL_CONTEXT_FLAGS_ARB,
        WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0
    };

    int piFormats = 0;
    UINT nNumFormats = 0;

    auto nativeDisplay = surface->getNativeDisplaySurface();

    if ( !wglChoosePixelFormatARB( nativeDisplay->DrawContext, backbufferAttributes, NULL, 1, &piFormats, &nNumFormats ) ) {
        return nullptr;
    }

    if ( !SetPixelFormat( nativeDisplay->DrawContext, piFormats, &pixelFormat ) ) {
        return nullptr;
    }

    auto renderContext = wglCreateContextAttribsARB( nativeDisplay->DrawContext, nullptr, context );
    if ( !renderContext ) {
        FLAN_CERR << "FATAL: wglCreateContextAttribsARB FAILED!" << std::endl;
        return nullptr;
    }

    wglMakeCurrent( nativeDisplay->DrawContext, renderContext );

    FLAN_CLOG << "OpenGL Version : " << (const char*)glGetString( GL_VERSION ) << std::endl;
    FLAN_CLOG << "Context creation done! Moving on..." << std::endl;

    NativeRenderContext* nativeRenderContext = new NativeRenderContext();
    nativeRenderContext->DrawContext = nativeDisplay->DrawContext;
    nativeRenderContext->RenderContext= renderContext;

    return nativeRenderContext;
}

void flan::rendering::PresentImpl( NativeRenderContext* nativeRenderContext )
{
    SwapBuffers( nativeRenderContext->DrawContext );
}

void flan::rendering::SetVSyncStateImpl( NativeRenderContext* nativeRenderContext, const bool enabled )
{
    wglSwapIntervalEXT( ( enabled ) ? 1 : 0 );
}
#endif

#if FLAN_DEVBUILD
void APIENTRY OpenGLDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
{
    // NOTE If you need a stack trace, you can set a breakpoint here
    if ( type == GL_DEBUG_TYPE_ERROR )
        FLAN_COUT << ( ( type == GL_DEBUG_TYPE_ERROR ) ? "[ERROR]" : "[LOG]" ) << "[GL] " << message << std::endl;
}
#endif

NativeRenderContext* flan::rendering::CreateRenderContextImpl( DisplaySurface* surface )
{
    // Create system specific context (wGL or GLX)
    auto nativeRenderContext = CreateRenderContextSysImpl( surface );

    if ( nativeRenderContext == nullptr ) {
        FLAN_CERR << "(FATAL) OpenGL context creation failed! Either your GPU does not support OpenGL4.6, or something went wrong during context creation." << std::endl;
        return nullptr;
    }
    
#if FLAN_DEVBUILD
#ifndef FLAN_NO_DEBUG_DEVICE
    FLAN_IMPORT_VAR_PTR( EnableDebugRenderDevice, bool )

    if ( EnableDebugRenderDevice ) {
        glEnable( GL_DEBUG_OUTPUT );
        glDebugMessageCallback( OpenGLDebugCallback, nullptr );
        FLAN_CWARN << "Debug Layer is enabled; performances might be impacted!" << std::endl;
    }
#else
    FLAN_CLOG << "Debug Layer is disabled (build was compiled with FLAN_NO_DEBUG_DEVICE)" << std::endl;
#endif
#endif

    // Use [0..1] depth range
    glClipControl( GL_UPPER_LEFT, GL_ZERO_TO_ONE );

    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );

    return nativeRenderContext;
}
#endif
