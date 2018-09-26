# -*- coding: utf-8 -*-
#	
#	Copyright (C) 2018 Team Motorway
#	
#	This file is part of Project Motorway source code.
#	
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#	
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <https://www.gnu.org/licenses/>.
#	

import sys, string, os, argparse, platform, codecs
from pathlib import Path

# Script args (see description below)
isProd = False
shader_folder = "../Flan/Shaders/"
compiled_shader_folder = "../bin/data/CompiledShaders/"
compileD3D11 = False
compileSPIRV = False

glslang_exe = 'glslangValidator'
spirvcross_exe = 'spirv-cross'

if platform.system() == 'Windows':
    glslang_exe = glslang_exe + '.exe'
    spirvcross_exe = spirvcross_exe + '.exe'

# Helpers
def build_flag_list( flags ):
    flagset = ""
    
    for (id, text) in flags.items():
        flagset = flagset + ( "/D " + id + "=" + text + " " )
        
    return flagset

def build_flag_list_GLSL( flags ):
    flagset = "-DFLAN_GLSL=1 "
    
    for (id, text) in flags.items():
        flagset = flagset + ( "-D" + id + "=" + text + " " )
         
    return flagset
    
def resolve_includes( filename, include_path ):
    datafile = codecs.open(filename, "r",encoding='utf-8', errors='ignore')
        
    output_content = ""
    
    for line in datafile:
        if '#include' in line:
            token, file = line.split( ' ' )
            
            is_absolute_include = False
            
            if ( ( '<' or '>' ) in file ):
                is_absolute_include = True
            
            file_to_include = file.replace("\r","").replace( '"', '' ).replace('\n', '').replace('<', '').replace('>', '')
            path = os.path.dirname( filename ) + "/"
            
            if is_absolute_include:      
                path = os.path.abspath( include_path ) + "/"
            
            my_path = os.path.abspath(path)
            file_to_include_path = os.path.join(my_path, file_to_include )
            fileContent = resolve_includes( file_to_include_path, include_path )
            output_content = output_content + fileContent
        else:
            output_content = output_content + line
            
    return output_content

def need_to_recompile( filename_input, filename_output, ext ):
    if forceRecompile:
        return True
    
    file_timestamp = str( os.path.getmtime( shader_folder + filename_input ) )
    
    cache_file = "./cache/" + filename_output + ext + ".stamp"
    if os.path.isfile( cache_file ):
        with open( cache_file, "r" ) as stream:
            for line in stream:
                if line == file_timestamp:
                    print( filename_output + " : skipped (no changes detected)" )
                    return False
        
    with open( cache_file, 'w' ) as stream:
        stream.write( str( file_timestamp ) )
    
    return True
    
def compile_shader_VS( filename_input, filename_output, entrypoint = 'EntryPointVS', flags = {}, flip_y = False ):
    if need_to_recompile( filename_input, filename_output, ".vso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )

        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /O3 /T vs_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".vso"
            if isProd:
                cmdLine = cmdLine + " /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"          
            os.system( cmdLine )
            
        if compileSPIRV:
            # GLSLang args are '-' delimited
            flag_list_glsl = build_flag_list_GLSL( flags )
            
            # Resolve includes directives (since glslang don't support this feature for hlsl)
            resolved_file = resolve_includes( shader_folder + filename_input, shader_folder )
            with open( "./tmp/" + filename_output + ".unroll", 'w') as outfile:
                outfile.write( resolved_file )

            # HLSL > SPIRV (VK)
            # SPIRV (VK) > GLSL
            # GLSL > SPIRV (GLSL)
            os.system( glslang_exe + " -S vert -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvv -D ./tmp/" + filename_output + ".unroll"  )	
            
            if flip_y:
                os.system( spirvcross_exe + " --combined-samplers-inherit-bindings --flip-vert-y --version 450 " + compiled_shader_folder + filename_output + ".vk.spvv --output ./tmp/" + filename_output + ".vert.glsl" )
            else:
                os.system( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvv --output ./tmp/" + filename_output + ".vert.glsl" )
          
            os.system( glslang_exe + " -S vert -G -o " + compiled_shader_folder + filename_output + ".gl.spvv ./tmp/" + filename_output + ".vert.glsl"  )	

def compile_shader_PS( filename_input, filename_output, entrypoint = 'EntryPointPS', flags = {} ):
    if need_to_recompile( filename_input, filename_output, ".pso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )
            
        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /O3 /T ps_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".pso"
            if isProd:
                cmdLine = cmdLine + " /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"          
            os.system( cmdLine )
        if compileSPIRV:
            # GLSLang args are '-' delimited
            flag_list_glsl = build_flag_list_GLSL( flags )
            
            # Resolve includes directives (since glslang don't support this feature for hlsl)
            resolved_file = resolve_includes( shader_folder + filename_input, shader_folder )
            with open( "./tmp/" + filename_output + ".unroll", 'w') as outfile:
                outfile.write( resolved_file )
                
            # HLSL > SPIRV (VK)
            # SPIRV (VK) > GLSL
            # GLSL > SPIRV (GLSL)
            os.system( glslang_exe + " -S frag -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvp -D ./tmp/" + filename_output + ".unroll"  )	
            os.system( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvp --output ./tmp/" + filename_output + ".frag.glsl" )
            os.system( glslang_exe + " -S frag -G -o " + compiled_shader_folder + filename_output + ".gl.spvp ./tmp/" + filename_output + ".frag.glsl"  )	

def compile_shader_CS( filename_input, filename_output, entrypoint = 'EntryPointCS', flags = {} ):
    if need_to_recompile( filename_input, filename_output, ".cso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )
            
        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /O3 /T cs_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".cso"
            if isProd:
                cmdLine = cmdLine + " /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"          
            os.system( cmdLine )
        if compileSPIRV:
            # GLSLang args are '-' delimited
            flag_list_glsl = build_flag_list_GLSL( flags )
            
            # Resolve includes directives (since glslang don't support this feature for hlsl)
            resolved_file = resolve_includes( shader_folder + filename_input, shader_folder )
            with open( "./tmp/" + filename_output + ".unroll", 'w') as outfile:
                outfile.write( resolved_file )
                
            # HLSL > SPIRV (VK)
            # SPIRV (VK) > GLSL
            # GLSL > SPIRV (GLSL)
            os.system( glslang_exe + " -S comp -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvc -D ./tmp/" + filename_output + ".unroll"  )	
            os.system( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvc --output ./tmp/" + filename_output + ".comp.glsl" )
            os.system( glslang_exe + " -S comp -G -o " + compiled_shader_folder + filename_output + ".gl.spvc ./tmp/" + filename_output + ".comp.glsl"  )	
            
# Parse script args
parser = argparse.ArgumentParser(description='Flan Game Engine. Compile shaders permutations for Graphics backends.')
parser.add_argument('--d3d11', dest='compileD3D11', action='store_const',
                   const=True, default=False,
                   help='compile shaders for Direct3D11')
parser.add_argument('--spirv', dest='compileSPIRV', action='store_const',
                   const=True, default=False,
                   help='compile shaders for OpenGL/Vulkan (using SPIR-V bytecode)')
parser.add_argument('--prod', dest='isProd', action='store_const',
                   const=True, default=False,
                   help='use aggressive compilation settings (WARNING: this will strip reflection/debug informations aswell)')
parser.add_argument('--force-recompile', dest='forceRecompile', action='store_const',
                   const=True, default=False,
                   help='force shaders recompilation (skip timestamp cache lookup)')
				   
args = parser.parse_args()

isProd = args.isProd
compileD3D11 = args.compileD3D11
compileSPIRV = args.compileSPIRV
forceRecompile = args.forceRecompile

# Check if the path exist first
if not os.path.exists( compiled_shader_folder ):
    print( "%s does not exist yet" % compiled_shader_folder )
    os.makedirs( compiled_shader_folder )

if not os.path.exists( "./tmp" ):
    print( "./tmp does not exist yet" )
    os.makedirs( "./tmp" )
    
if not os.path.exists( "./cache" ):
    print( "./cache does not exist yet" )
    os.makedirs( "./cache" )
    
# Atmosphere
compile_shader_VS( "Atmosphere/SkyRendering.hlsl", "SkyRendering", "EntryPointVS", {}, False )
compile_shader_PS( "Atmosphere/SkyRendering.hlsl", "SkyRendering", "EntryPointPS", { "PA_RENDER_SUN_DISC" : "1" } )
compile_shader_PS( "Atmosphere/SkyRendering.hlsl", "SkyRenderingNoSunDisc" )

# Editor
compile_shader_CS( "Editor/DFGLUTGeneration.hlsl", "DFGLUTGeneration" )
compile_shader_VS( "Editor/IBLConvolution.hlsl", "IBLConvolution", "EntryPointVS", {}, True )
compile_shader_PS( "Editor/IBLConvolution.hlsl", "IBLConvolution" )
compile_shader_CS( "Editor/VMFSolver.hlsl", "VMFSolver" )
compile_shader_CS( "Editor/VMFSolver.hlsl", "VMFSolverTextureMapInput", "EntryPointCS", { "FLAN_TEX_INPUT" : "1" } )

# Debug
compile_shader_PS( "Debug/Wireframe.hlsl", "Wireframe" )
compile_shader_VS( "Debug/Primitive.hlsl", "Primitive" )
compile_shader_VS( "Debug/Line.hlsl", "Line" )
compile_shader_PS( "Debug/Line.hlsl", "Line" )

# Common
compile_shader_PS( "Common/DownsampleFast.hlsl", "DownsampleFast" )
compile_shader_PS( "Common/UpsampleWeighted.hlsl", "UpsampleWeighted" )
compile_shader_PS( "Common/DownsampleWeighted.hlsl", "DownsampleWeighted" )
compile_shader_PS( "Common/DownsampleWeighted.hlsl", "DownsampleWeightedStabilized", "EntryPointPS_Karis" )
compile_shader_VS( "Common/FullscreenQuad.hlsl", "FullscreenQuad", "EntryPointVS", {}, False )
compile_shader_VS( "Common/FullscreenTriangle.hlsl", "FullscreenTrianglePresent", "EntryPointVS", {}, True )
compile_shader_VS( "Common/FullscreenTriangle.hlsl", "FullscreenTriangle", "EntryPointVS", {}, False )
compile_shader_VS( "Common/DepthWrite.hlsl", "DepthWrite" )
compile_shader_PS( "Common/CopyTexture.hlsl", "CopyTexture" )

compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve1", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "1", "PH_USE_TAA": "0" } )
compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve2", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "2", "PH_USE_TAA": "0" } )
compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve4", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "4", "PH_USE_TAA": "0" } )
compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve8", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "8", "PH_USE_TAA": "0" } )

compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve1TAA", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "1", "PH_USE_TAA": "1" } )
compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve2TAA", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "2", "PH_USE_TAA": "1" } )
compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve4TAA", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "4", "PH_USE_TAA": "1" } )
compile_shader_PS( "Common/MSAAResolve.hlsl", "MSAAResolve8TAA", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "8", "PH_USE_TAA": "1" } )

compile_shader_PS( "Common/MSAADepthResolve.hlsl", "MSAADepthResolve1", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "1" } )
compile_shader_PS( "Common/MSAADepthResolve.hlsl", "MSAADepthResolve2", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "2" } )
compile_shader_PS( "Common/MSAADepthResolve.hlsl", "MSAADepthResolve4", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "4" } )
compile_shader_PS( "Common/MSAADepthResolve.hlsl", "MSAADepthResolve8", "EntryPointPS", { "PH_MSAA_SAMPLE_COUNT" : "8" } )

# UI
compile_shader_VS( "UI/Primitive2D.hlsl", "Primitive2D" )
compile_shader_PS( "UI/Primitive2D.hlsl", "Primitive2D", "EntryPointPS", { "PA_EDITOR": "1" } )
compile_shader_PS( "UI/SDFTextRendering.hlsl", "SDFTextRendering" )
compile_shader_VS( "UI/SDFTextRendering.hlsl", "SDFTextRendering" )
compile_shader_VS( "UI/UIGeometry.hlsl", "UIGeometry", "EntryPointVS", {}, False )

# ImageEffects
compile_shader_CS( "ImageEffects/AutoExposureCompute.hlsl", "AutoExposureCompute" )
compile_shader_PS( "ImageEffects/Compositing.hlsl", "FrameComposition" )
compile_shader_PS( "ImageEffects/GaussianBlur.hlsl", "GaussianBlur" )
compile_shader_CS( "ImageEffects/MergeHistogram.hlsl", "MergeHistogram" )
compile_shader_CS( "ImageEffects/TileHistogramCompute.hlsl", "TileHistogramCompute" )
compile_shader_PS( "ImageEffects/FXAA.hlsl", "FXAA" )

# Lighting
compile_shader_CS( "Lighting/LightCulling.hlsl", "LightCulling" )
compile_shader_CS( "Lighting/LightCulling.hlsl", "LightCullingMSAA", "EntryPointCS", { "PH_USE_MSAA": "1" } )

# compile_shader_VS( "Lighting/Opaque.hlsl", "Opaque" )
# compile_shader_VS( "Lighting/Opaque.hlsl", "OpaqueScaledUV", "EntryPointVS", { "PH_SCALE_UV_BY_MODEL_SCALE": "1" } )
# compile_shader_VS( "Lighting/Opaque.hlsl", "OpaqueNormalMapping", "EntryPointVS", { "PH_USE_NORMAL_MAPPING": "1" } )
# compile_shader_VS( "Lighting/Opaque.hlsl", "OpaqueScaledUVNormalMapping", "EntryPointVS", { "PH_SCALE_UV_BY_MODEL_SCALE": "1", "PH_USE_NORMAL_MAPPING": "1" } )

# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueDebugTileHeat", "EntryPointHeatMapPS", { "PA_EDITOR": "1" } )
# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueStandard", "EntryPointPS", { "PA_USE_STANDARD_SM": "1", "PA_EDITOR": "1" } )
# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueEmissive", "EntryPointPS", { "PA_USE_EMISSIVE_SM": "1", "PA_DONT_RECEIVE_SHADOWS": "1", "PA_EDITOR": "1" } )
# compile_shader_PS( "Lighting/UberOpaque.hlsl", "Opaque", "EntryPointPS", { "PA_EDITOR": "1" } )
# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueDepth", "EntryPointDepthPS", { "PA_EDITOR": "1" } )

# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueStandardProbeCapture", "EntryPointPS", { "PA_USE_STANDARD_SM": "1", "PA_EDITOR": "1", "PA_PROBE_CAPTURE" : "1" } )
# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueClearCoatProbeCapture", "EntryPointPS", { "PA_USE_CLEARCOAT_SM": "1", "PA_EDITOR": "1", "PA_PROBE_CAPTURE" : "1" } )
# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueEmissiveProbeCapture", "EntryPointPS", { "PA_USE_EMISSIVE_SM": "1", "PA_EDITOR": "1", "PA_DONT_RECEIVE_SHADOWS": "1", "PA_PROBE_CAPTURE" : "1" } )
# compile_shader_PS( "Lighting/UberOpaque.hlsl", "OpaqueProbeCapture", "EntryPointPS", { "PA_EDITOR": "1", "PA_PROBE_CAPTURE" : "1" } )

compile_shader_VS( "Lighting/Surface.hlsl", "Surface" )
compile_shader_VS( "Lighting/Surface.hlsl", "SurfaceScaledUV", "EntryPointVS", { "PH_SCALE_UV_BY_MODEL_SCALE": "1" } )
compile_shader_VS( "Lighting/Surface.hlsl", "SurfaceNormalMapping", "EntryPointVS", { "PH_USE_NORMAL_MAPPING": "1" } )
compile_shader_VS( "Lighting/Surface.hlsl", "SurfaceScaledUVNormalMapping", "EntryPointVS", { "PH_SCALE_UV_BY_MODEL_SCALE": "1", "PH_USE_NORMAL_MAPPING": "1" } )

compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceDebugTileHeat", "EntryPointHeatMapPS", { "PA_EDITOR": "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceStandard", "EntryPointPS", { "PA_USE_STANDARD_SM": "1", "PA_EDITOR": "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceClearCoat", "EntryPointPS", { "PA_USE_CLEARCOAT_SM": "1", "PA_EDITOR": "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceEmissive", "EntryPointPS", { "PA_USE_EMISSIVE_SM": "1", "PA_DONT_RECEIVE_SHADOWS": "1", "PA_EDITOR": "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "Surface", "EntryPointPS", { "PA_EDITOR": "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceDepth", "EntryPointDepthPS", { "PA_EDITOR": "1" } )

compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceStandardProbeCapture", "EntryPointPS", { "PA_USE_STANDARD_SM": "1", "PA_EDITOR": "1", "PA_PROBE_CAPTURE" : "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceClearCoatProbeCapture", "EntryPointPS", { "PA_USE_CLEARCOAT_SM": "1", "PA_EDITOR": "1", "PA_PROBE_CAPTURE" : "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceEmissiveProbeCapture", "EntryPointPS", { "PA_USE_EMISSIVE_SM": "1", "PA_EDITOR": "1", "PA_DONT_RECEIVE_SHADOWS": "1", "PA_PROBE_CAPTURE" : "1" } )
compile_shader_PS( "Lighting/UberSurface.hlsl", "SurfaceProbeCapture", "EntryPointPS", { "PA_EDITOR": "1", "PA_PROBE_CAPTURE" : "1" } )
