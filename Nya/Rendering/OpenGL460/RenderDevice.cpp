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

#if NYA_GL460
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include <Display/DisplaySurface.h>

#include "Extensions.h"

#include <limits>

RenderDevice::~RenderDevice()
{
}

// GLX/WGL Implementations
#if NYA_UNIX
#include <Display/DisplaySurfaceXcb.h>

struct RenderContext
{
    GLXDrawable DrawContext;
    Display*    X11Display;
};

RenderContext* CreateRenderContextSysImpl( BaseAllocator* memoryAllocator, DisplaySurface* surface )
{
    NYA_CLOG << "Creating render context (GLX)" << std::endl;

    Display* tmpDisp = XOpenDisplay( nullptr );

    if ( tmpDisp == nullptr ) {
        NYA_CERR << "Failed to open Xorg root display" << std::endl;
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
        NYA_CERR << "Failed to query glX version (missing glX packet(s)?)" << std::endl;
        return nullptr;
    }

    NYA_CLOG << "glX version: " << glxMajor << "." << glxMinor << std::endl;

    int32_t backbufferConfigCount = 0;
    GLXFBConfig* backbufferConfigs = glXChooseFBConfig( tmpDisp, defaultScreen, backbufferAttributes, &backbufferConfigCount );

    if( backbufferConfigs == nullptr || backbufferConfigCount <= 0 ) {
        NYA_CERR << "Could not find a suitable backbuffer configuration" << std::endl;
        return nullptr;
    }

    NYA_CLOG << "glXChooseFBConfig: " << backbufferConfigCount << " configuration(s) available" << std::endl;
    int32_t bestConfigId = -1, worstConfigId = -1, bestSampNum = -1, worstSampNum = std::numeric_limits<int>::max();

    VisualID visualID = 0;
    for( int32_t i = 0; i < backbufferConfigCount; ++i ) {
        XVisualInfo* visualInfos = glXGetVisualFromFBConfig( tmpDisp, backbufferConfigs[i] );

        if ( visualInfos != nullptr ) {
            int32_t sampleBuffers = 0, samples = 0;

            glXGetFBConfigAttrib( tmpDisp, backbufferConfigs[i], GLX_SAMPLE_BUFFERS, &sampleBuffers );
            glXGetFBConfigAttrib( tmpDisp, backbufferConfigs[i], GLX_SAMPLES, &samples );

            NYA_CLOG << "glXGetVisualFromFBConfig [" << i << "] => " << sampleBuffers << " | " << samples << std::endl;

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

    NYA_CLOG << "Selected [" << bestConfigId << "]" << std::endl;

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
        NYA_CERR << "glX context creation failed! (failed to retrieve glXCreateContextAttribs)" << std::endl;
        return nullptr;
    }

    auto context = glXCreateContextAttribs( tmpDisp, backbufferChoosenConfig, nullptr, True, ATTRIBUTES );
    if ( context == nullptr ) {
        NYA_CERR << "glX context creation failed! (backbuffer configuration is probably incorrect)" << std::endl;
        return nullptr;
    }

    auto nativeSurface = surface->nativeDisplaySurface;
    GLXWindow tmpWin = glXCreateWindow( tmpDisp, backbufferChoosenConfig, nativeSurface->WindowInstance, nullptr );

    if ( tmpWin <= 0 ) {
        NYA_CERR << "glX window creation failed! (bad colormap provided)" << std::endl;
        return nullptr;
    }

    glXMakeContextCurrent( tmpDisp, tmpWin, tmpWin, context );

    if ( !nya::rendering::LoadOpenGLExtensions() ) {
        NYA_CERR << "An error happened during OpenGL extension loading (one or several extension might be missing on your system)" << std::endl;
        return nullptr;
    }

    NYA_CLOG << "OpenGL Version : " << glGetString( GL_VERSION ) << std::endl;
    NYA_CLOG << "Context creation done! Moving on..." << std::endl;

    // Use [0..1] depth range
    glClipControl( GL_LOWER_LEFT, GL_ZERO_TO_ONE );

    RenderContext* renderContext = nya::core::allocate<RenderContext>( memoryAllocator );
    renderContext->X11Display = tmpDisp;
    renderContext->DrawContext = tmpWin;

    return renderContext;
}

void RenderDevice::present()
{
    glXSwapBuffers( renderContext->X11Display, renderContext->DrawContext );
}

void RenderDevice::enableVerticalSynchronisation( const bool enabled )
{
    glXSwapIntervalEXT( renderContext->X11Display, renderContext->DrawContext, ( enabled ) ? 1 : 0 );
}

#elif NYA_WIN
#include <Display/DisplaySurfaceWin32.h>

struct RenderContext
{
    HDC         DrawContext;
    HGLRC       RenderContext;
};

LRESULT CALLBACK FakeWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc( hwnd, msg, wParam, lParam );
}

NativeRenderContext* CreateRenderContextSysImpl( BaseAllocator* memoryAllocator, DisplaySurface* surface )
{
    NYA_CLOG << "Creating render context (WGL)" << std::endl;

    auto fakeFrameName = NYA_STRING( "FlanGLFakeFrame" );

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

    HWND fakeWindow = CreateWindow( fakeFrameName, NYA_STRING( "FlanGLFakeWindow" ), WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL );
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
        NYA_CERR << "An error happened during OpenGL extension loading (one or several extension might be missing on your system)" << std::endl;
        return nullptr;
    }

    NYA_CLOG << "Loaded extension list! Now destroying fake GL context..." << std::endl;

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
        NYA_CERR << "FATAL: wglCreateContextAttribsARB FAILED!" << std::endl;
        return nullptr;
    }

    wglMakeCurrent( nativeDisplay->DrawContext, renderContext );

    NYA_CLOG << "OpenGL Version : " << (const char*)glGetString( GL_VERSION ) << std::endl;
    NYA_CLOG << "Context creation done! Moving on..." << std::endl;

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

#if NYA_DEVBUILD
void APIENTRY OpenGLDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
{
    // NOTE If you need a stack trace, you can set a breakpoint here
    if ( severity == GL_DEBUG_SEVERITY_NOTIFICATION )
        NYA_COUT << "[LOG] ";
    else if ( type == GL_DEBUG_SEVERITY_LOW )
        NYA_COUT << "[INFO] ";
    else if ( type == GL_DEBUG_SEVERITY_MEDIUM )
        NYA_COUT << "[WARN] ";
    else if ( type == GL_DEBUG_SEVERITY_HIGH )
        NYA_COUT << "[ERROR] ";

    NYA_COUT << NYA_FILENAME << ":" << __LINE__ << " >> " << source << " : " << message << std::endl;
}
#endif

void RenderDevice::create( DisplaySurface* surface )
{
    NYA_CLOG << "Creating RenderDevice (OpenGL 4.6)" << std::endl;

    // Create system specific context (wGL or GLX)
    renderContext = CreateRenderContextSysImpl( memoryAllocator, surface );

    NYA_ASSERT( ( renderContext == nullptr ), "(FATAL) OpenGL context creation failed! Either your GPU does not support OpenGL 4.6, or something went wrong during context creation. (0x%x)", glGetError() );

#if NYA_DEVBUILD
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( OpenGLDebugCallback, nullptr );
    NYA_CWARN << "Debug Layer is enabled; performances might be impacted!" << std::endl;
#else
    NYA_CLOG << "Debug Layer is disabled (build was compiled with NYA_NO_DEBUG_DEVICE)" << std::endl;
#endif

    // Use [0..1] depth range
    glClipControl( GL_UPPER_LEFT, GL_ZERO_TO_ONE );

    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
}

RenderTarget* RenderDevice::getSwapchainBuffer()
{
    return nullptr;
}

CommandList& RenderDevice::allocateGraphicsCommandList() const
{

}

CommandList& RenderDevice::allocateComputeCommandList() const
{

}

void RenderDevice::submitCommandList( CommandList* commandList )
{

}

const nyaChar_t* RenderDevice::getBackendName() const
{
    return NYA_STRING( "OpenGL 4.6" );
}
#endif
