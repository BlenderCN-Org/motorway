# -*- coding: utf-8 -*-
#    
#    Copyright (C) 2018 Team Motorway
#    
#    This file is part of Project Motorway source code.
#    
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#    
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#    
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#    

import sys, string, os, argparse, platform, codecs
from pathlib import Path
import subprocess


# Script args (see description below)
isProd = False
shader_folder = "../Nya/Shaders/"
compiled_shader_folder = "../bin/data/shaders/"
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
                    #print( filename_output + " : skipped (no changes detected)" )
                    return False
        
    with open( cache_file, 'w' ) as stream:
        stream.write( str( file_timestamp ) )
    
    return True
    
def compile_shader_VS( filename_input, filename_output, entrypoint = 'EntryPointVS', flags = {}, flip_y = False ):
    if need_to_recompile( filename_input, filename_output, ".vso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )

        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /T vs_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".vso"
            if isProd:
                cmdLine = cmdLine + " /O3 /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"
            else:
                cmdLine = cmdLine + " /O1"
                
            subprocess.run( cmdLine )
            
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
            subprocess.run( glslang_exe + " -S vert -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvv -D ./tmp/" + filename_output + ".unroll"  )    
            
            if flip_y:
                subprocess.run( spirvcross_exe + " --combined-samplers-inherit-bindings --flip-vert-y --version 450 " + compiled_shader_folder + filename_output + ".vk.spvv --output ./tmp/" + filename_output + ".vert.glsl" )
            else:
                subprocess.run( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvv --output ./tmp/" + filename_output + ".vert.glsl" )
          
            subprocess.run( glslang_exe + " -S vert -G -o " + compiled_shader_folder + filename_output + ".gl.spvv ./tmp/" + filename_output + ".vert.glsl"  )    

def compile_shader_PS( filename_input, filename_output, entrypoint = 'EntryPointPS', flags = {} ):
    if need_to_recompile( filename_input, filename_output, ".pso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )
            
        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /T ps_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".pso"
            if isProd:
                cmdLine = cmdLine + " /O3 /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"
            else:
                cmdLine = cmdLine + " /O1"     
            subprocess.run( cmdLine )
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
            subprocess.run( glslang_exe + " -S frag -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvp -D ./tmp/" + filename_output + ".unroll"  )    
            subprocess.run( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvp --output ./tmp/" + filename_output + ".frag.glsl" )
            subprocess.run( glslang_exe + " -S frag -G -o " + compiled_shader_folder + filename_output + ".gl.spvp ./tmp/" + filename_output + ".frag.glsl"  )    

def compile_shader_CS( filename_input, filename_output, entrypoint = 'EntryPointCS', flags = {} ):
    if need_to_recompile( filename_input, filename_output, ".cso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )
            
        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /T cs_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".cso"
            if isProd:
                cmdLine = cmdLine + " /O3 /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"
            else:
                cmdLine = cmdLine + " /O1"
            subprocess.run( cmdLine )
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
            subprocess.run( glslang_exe + " -S comp -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvc -D ./tmp/" + filename_output + ".unroll"  )    
            subprocess.run( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvc --output ./tmp/" + filename_output + ".comp.glsl" )
            subprocess.run( glslang_exe + " -S comp -G -o " + compiled_shader_folder + filename_output + ".gl.spvc ./tmp/" + filename_output + ".comp.glsl"  )    
 
def compile_shader_HS( filename_input, filename_output, entrypoint = 'EntryPointHS', flags = {} ):
    if need_to_recompile( filename_input, filename_output, ".hso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )
            
        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /T hs_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".hso"
            if isProd:
                cmdLine = cmdLine + " /O3 /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"
            else:
                cmdLine = cmdLine + " /O1"          
            subprocess.run( cmdLine )
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
            subprocess.run( glslang_exe + " -S tesc -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvh -D ./tmp/" + filename_output + ".unroll"  )    
            subprocess.run( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvh --output ./tmp/" + filename_output + ".hull.glsl" )
            subprocess.run( glslang_exe + " -S tesc -G -o " + compiled_shader_folder + filename_output + ".gl.spvh ./tmp/" + filename_output + ".hull.glsl"  )    
    
def compile_shader_DS( filename_input, filename_output, entrypoint = 'EntryPointDS', flags = {} ):
    if need_to_recompile( filename_input, filename_output, ".dso" ):
        if flags is not None:
            flag_list = build_flag_list( flags )
            
        if compileD3D11:
            cmdLine = "fxc.exe /nologo /E " + entrypoint + " " + flag_list + " /T ds_5_0 " + shader_folder + filename_input + " /I " + shader_folder + " /Fo " + compiled_shader_folder + filename_output + ".dso"
            if isProd:
                cmdLine = cmdLine + " /O3 /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"
            else:
                cmdLine = cmdLine + " /O1"
            subprocess.run( cmdLine )
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
            subprocess.run( glslang_exe + " -S tese -e " + entrypoint + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename_output + ".vk.spvd -D ./tmp/" + filename_output + ".unroll"  )    
            subprocess.run( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename_output + ".vk.spvd --output ./tmp/" + filename_output + ".domain.glsl" )
            subprocess.run( glslang_exe + " -S tese -G -o " + compiled_shader_folder + filename_output + ".gl.spvd ./tmp/" + filename_output + ".domain.glsl"  )    
            
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
compile_shader_VS( "Atmosphere/HosekSky.hlsl", "HosekSky" )
compile_shader_PS( "Atmosphere/HosekSky.hlsl", "HosekSky" )

# Atmosphere
compile_shader_VS( "Atmosphere/BrunetonSky.hlsl", "BrunetonSky", "EntryPointVS", {}, False )
compile_shader_PS( "Atmosphere/BrunetonSky.hlsl", "BrunetonSky", "EntryPointPS", { "NYA_RENDER_SUN_DISC" : "1" } )
compile_shader_PS( "Atmosphere/BrunetonSky.hlsl", "BrunetonSkyNoSunDisc" )

# Shared
compile_shader_VS( "FullscreenTriangle.hlsl", "FullscreenTriangle" )
compile_shader_PS( "CopyTexture.hlsl", "CopyTexture" )

# UI
compile_shader_PS( "UI/SDFTextRendering.hlsl", "SDFTextRendering" )
compile_shader_VS( "UI/SDFTextRendering.hlsl", "SDFTextRendering" )