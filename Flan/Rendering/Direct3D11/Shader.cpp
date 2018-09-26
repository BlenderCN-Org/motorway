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

#if FLAN_D3D11
#include "Shader.h"

#include "RenderContext.h"

#include <d3dcompiler.h>
#include <fstream>
#include <vector>

uint32_t GetD3D11ShaderStage( const eShaderStage stage )
{
    auto stageShift = static_cast<uint32_t>( stage );
    uint32_t index = 0;
    while ( stageShift >>= 1 ) {
        index++;
    }

    return index;
}
// Interface which must be implemented by the application so that the shader compiler retrieve the include paths correctly
// This shouldn't be used in a release client build
class ShaderInclude : public ID3DInclude
{
public:
    ShaderInclude( const char* shaderDir, const char* systemDir );
    ShaderInclude( ShaderInclude& ) = default;
    ~ShaderInclude() = default;

    HRESULT __stdcall Open( D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes );
    HRESULT __stdcall Close( LPCVOID pData );

private:
    std::string workingDirectory;
    std::string absoluteDirectory;
};

ShaderInclude::ShaderInclude( const char* includePathWorkingDirectory, const char* includePathAbsolute )
    : workingDirectory( includePathWorkingDirectory )
    , absoluteDirectory( includePathAbsolute )
{

}

HRESULT __stdcall ShaderInclude::Open( D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes )
{
    std::string finalPath = {};

    switch ( IncludeType ) {
    case D3D_INCLUDE_LOCAL:
        finalPath.append( workingDirectory );
        break;

    case D3D_INCLUDE_SYSTEM:
        finalPath.append( absoluteDirectory );
        break;

    default:
        return E_FAIL;
    }

    finalPath += "\\";
    finalPath += pFileName;

    std::ifstream includeFile( finalPath.c_str(), std::ios::in | std::ios::binary | std::ios::ate );

    if ( includeFile.is_open() ) {
        auto fileSize = includeFile.tellg();

        char* buf = new char[fileSize];
        includeFile.seekg( 0, std::ios::beg );
        includeFile.read( buf, fileSize );
        includeFile.close();

        *ppData = buf;
        *pBytes = static_cast<UINT>( fileSize );
    } else {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT __stdcall ShaderInclude::Close( LPCVOID pData )
{
    char* buf = ( char* )pData;
    delete[] buf;

    return S_OK;
}

NativeShaderObject* flan::rendering::CreateAndCompileShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage, const char* shaderToBeCompiled, const std::size_t shaderToBeCompiledLength )
{
    static constexpr char* DX11_SE[SHADER_STAGE_COUNT] = {
        "vs_5_0",
        "ps_5_0",
        "ds_5_0",
        "hs_5_0",
        "cs_5_0",
    };

    static constexpr char* DX11_EP[SHADER_STAGE_COUNT] = {
        "EntryPointVS",
        "EntryPointPS",
        "EntryPointDS",
        "EntryPointHS",
        "EntryPointCS",
    };

    auto pipelineStageIndex = GetD3D11ShaderStage( shaderStage );

    ID3D10Blob* stageBlob = nullptr;
    ID3D10Blob*	compilationErrorBlob = nullptr;

    ShaderInclude include( ".\\dev\\Shaders\\", "..\\Flan\\Graphics\\Shaders\\" );

    HRESULT operationResult = D3DCompile(
        shaderToBeCompiled,
        shaderToBeCompiledLength,
        nullptr,
        nullptr,
        &include,
        DX11_EP[pipelineStageIndex],
        DX11_SE[pipelineStageIndex],
        D3DCOMPILE_OPTIMIZATION_LEVEL3,
        0,
        &stageBlob,
        &compilationErrorBlob
    );
    
    if ( FAILED( operationResult ) ) {
        FLAN_CERR << "Failed to compile shader (error code: " << operationResult << ")" << std::endl;
        FLAN_CERR << "Infos: " << ( char* )compilationErrorBlob->GetBufferPointer() << std::endl;

        compilationErrorBlob->Release();
        return nullptr;
    }

    NativeShaderObject* nativeShaderObject = CreatePrecompiledShaderImpl( nativeRenderContext, shaderStage, (uint8_t*)stageBlob->GetBufferPointer(), stageBlob->GetBufferSize() );
    
    stageBlob->Release();

    return nativeShaderObject;
}

NativeShaderObject* flan::rendering::CreatePrecompiledShaderImpl( NativeRenderContext* nativeRenderContext, const eShaderStage shaderStage,  const uint8_t* precompiledShader, const std::size_t precompiledShaderLength )
{
    auto nativeDevice = nativeRenderContext->nativeDevice;
    HRESULT operationResult = S_OK;

    NativeShaderObject* nativeShaderObject = new NativeShaderObject();

    switch ( shaderStage ) {
    case SHADER_STAGE_VERTEX:
        operationResult = nativeDevice->CreateVertexShader( precompiledShader, precompiledShaderLength, NULL, &nativeShaderObject->vertexShader );
        break;

    case SHADER_STAGE_PIXEL:
        operationResult = nativeDevice->CreatePixelShader( precompiledShader, precompiledShaderLength, NULL, &nativeShaderObject->pixelShader );
        break;

    case SHADER_STAGE_TESSELATION_CONTROL:
        operationResult = nativeDevice->CreateHullShader( precompiledShader, precompiledShaderLength, NULL, &nativeShaderObject->tesselationControlShader );
        break;

    case SHADER_STAGE_TESSELATION_EVALUATION:
        operationResult = nativeDevice->CreateDomainShader( precompiledShader, precompiledShaderLength, NULL, &nativeShaderObject->tesselationEvalShader );
        break;

    case SHADER_STAGE_COMPUTE:
        operationResult = nativeDevice->CreateComputeShader( precompiledShader, precompiledShaderLength, NULL, &nativeShaderObject->computeShader );
        break;
    }

    if ( operationResult != S_OK ) {
        FLAN_CERR << "Failed to load precompiled shader (error code: " << operationResult << ")" << std::endl;
        return nullptr;
    }

    return nativeShaderObject;
}

void flan::rendering::DestroyShaderImpl( NativeRenderContext* nativeRenderContext, NativeShaderObject* shaderObject )
{
#define D3D11_RELEASE( obj ) if ( obj != nullptr ) { obj->Release(); obj = nullptr; }
    D3D11_RELEASE( shaderObject->resource );
}
#endif
