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

import sys, string, os, argparse, platform, codecs, itertools, MurmurHash, multiprocessing

from pathlib import Path
from subprocess import Popen
from threading import Thread

# Script args (see description below)
isProd = False
shader_folder = "../Nya/Shaders/"
compiled_shader_folder = "../bin/data/shaders/"
compileD3D11 = False
compileSPIRV = False
compileWaitOutput = False
glslang_exe = 'glslangValidator'
spirvcross_exe = 'spirv-cross'
current_task_count = 0

if platform.system() == 'Windows':
    glslang_exe = glslang_exe + '.exe'
    spirvcross_exe = spirvcross_exe + '.exe'

# Helpers
def build_flag_list( flags ):
    flagset = ""
    
    for flag in flags:
        flagset = flagset + ( "/D " + flag + "=1 " )
        
    return flagset

def build_flag_list_GLSL( flags ):
    flagset = "-DFLAN_GLSL=1 "
    
    for flag in flags:
        flagset = flagset + ( "-D" + flag + "=1 " )
         
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
    
    file_timestamp = str( os.path.getmtime( shader_folder + filename_input + ".hlsl" ) )
    
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
   
def generate_combination_list( flags ):
    combs = [[[]]]
    for i in range( 1, len( flags ) + 1 ):
        els = [list( x ) for x in itertools.combinations( flags, i )]
        combs.append( els )
        
    return combs
   
def get_permutation_hashcode( name, flags ):
    filename = name
    
    for flag in flags:
        filename = filename + "+" + flag
        
    hash_object = MurmurHash.hash128( filename.encode( 'utf-8' ) ) #hashlib.md5( filename.encode( 'utf-8' ) )
    
    return hex( hash_object )[2:].zfill( 32 )
    
def get_entry_point( type ):
    return {
        'VS': 'EntryPointVS',
        'DS': 'EntryPointDS',
        'HS': 'EntryPointHS',
        'PS': 'EntryPointPS',
        'CS': 'EntryPointCS'
    }.get( type, 'main' )
    
def get_extension( type ):
    return {
        'VS': '.vso',
        'DS': '.dso',
        'HS': '.hso',
        'PS': '.pso',
        'CS': '.cso'
    }.get( type, '.unk' )
    
def get_d3d_sm( type ):
    return {
        'VS': 'vs_5_0',
        'HS': 'hs_5_0',
        'DS': 'ds_5_0',
        'PS': 'ps_5_0',
        'CS': 'cs_5_0'
    }.get( type, 'vs_5_0' )
    
def get_sprv_sm( type ):
    return {
        'VS': 'vert',
        'HS': 'tesc',
        'DS': 'tese',
        'PS': 'frag',
        'CS': 'comp'
    }.get( type, 'vs_5_0' )
    
def popenAndCall(popenArgs):
    global current_task_count
    cpu_thread_count = multiprocessing.cpu_count()
    
    def runInThread(popenArgs):
        global current_task_count
        proc = Popen(popenArgs)
        proc.wait()
        current_task_count = current_task_count - 1
        return
        
    while current_task_count > cpu_thread_count:
        pass
        
    thread = Thread(target=runInThread, args=(popenArgs,))
    current_task_count = current_task_count + 1
    thread.start()
    return thread
    
def compile_permutation_d3d11( shader_name, filename, entry_point, extension, shading_model, permutation = [] ):
    cmd_args_list = build_flag_list( permutation )
    
    cmdLine = "fxc.exe /nologo /E " + entry_point + " " + cmd_args_list + " /T " + shading_model + " " + shader_folder + shader_name + ".hlsl /I " + shader_folder + " /Fo " + compiled_shader_folder + filename + extension
    
    if isProd:
        cmdLine = cmdLine + " /O3 /Qstrip_debug /Qstrip_reflect /Qstrip_priv /Qstrip_rootsignature"
    else:
        cmdLine = cmdLine + " /O1"
        
    popenAndCall( ( cmdLine ) )
    
def compile_permutation_spirv( shader_name, filename, entry_point, extension, shading_model, permutation = [] ):  
    # GLSLang args are '-' delimited
    flag_list_glsl = build_flag_list_GLSL( permutation )

    # Resolve includes directives (since glslang don't support this feature for hlsl)
    resolved_file = resolve_includes( shader_folder + shader_name + ".hlsl", shader_folder )
    with open( "./tmp/" + filename + ".unroll", 'w') as outfile:
        outfile.write( resolved_file )

    # HLSL > SPIRV (VK)
    # SPIRV (VK) > GLSL
    # GLSL > SPIRV (GLSL)
    cmdLine = glslang_exe + " -S " + shading_model + " -e " + entry_point + " " + flag_list_glsl + " -V -o " + compiled_shader_folder + filename + extension + ".vk -D ./tmp/" + filename + ".unroll"
    popenAndCall( ( cmdLine ) )
    # subprocess.run( spirvcross_exe + " --combined-samplers-inherit-bindings --version 450 " + compiled_shader_folder + filename + extension + ".vk --output ./tmp/" + filename + extension + ".glsl" )
    # subprocess.run( glslang_exe + " -S " + shading_model + " -G -o " + compiled_shader_folder + filename + extension + ".gl ./tmp/" + filename + extension + ".glsl" )
    
def compile_shader( shader_name, type, flags = [] ):
    flagset = generate_combination_list( flags )
    
    entry_point = get_entry_point( type )
    extension = get_extension( type )
    shading_model_d3d = get_d3d_sm( type )
    shading_model_spirv = get_sprv_sm( type )
    
    for combination_group in flagset:
        for combination in combination_group:
            filename = get_permutation_hashcode( shader_name, combination )
            if need_to_recompile( shader_name, filename, extension ):
                if compileD3D11:
                    compile_permutation_d3d11( shader_name, filename, entry_point, extension, shading_model_d3d, combination )
                if compileSPIRV:
                    compile_permutation_spirv( shader_name, filename, entry_point, extension, shading_model_spirv, combination )

def compile_shader_VS( shader_name, flags = [] ):
    compile_shader( shader_name, 'VS', flags )
    
def compile_shader_DS( shader_name, flags = [] ):
    compile_shader( shader_name, 'DS', flags )
    
def compile_shader_HS( shader_name, flags = [] ):
    compile_shader( shader_name, 'HS', flags )
    
def compile_shader_PS( shader_name, flags = [] ):
    compile_shader( shader_name, 'PS', flags )
    
def compile_shader_CS( shader_name, flags = [] ):
    compile_shader( shader_name, 'CS', flags )
     
# Parse script args
parser = argparse.ArgumentParser(description='Nya Game Engine. Compile shaders permutations for Graphics backends.')
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
    
# Error
compile_shader_VS( "Error" )
compile_shader_PS( "Error" )
compile_shader_CS( "Error" )

# Atmosphere
compile_shader_VS( "Atmosphere/BrunetonSky" )
compile_shader_PS( "Atmosphere/BrunetonSky", [ "NYA_RENDER_SUN_DISC", "NYA_FIXED_EXPOSURE" ] )

# AutoExposure
compile_shader_CS( "AutoExposure/BinCompute" )
compile_shader_CS( "AutoExposure/HistogramMerge" )
compile_shader_CS( "AutoExposure/TileHistogramCompute" )

# PostFX
compile_shader_CS( "PostFX/FinalPost" )
compile_shader_PS( "PostFX/Downsample", [ "NYA_USE_KARIS_AVERAGE" ] )
compile_shader_PS( "PostFX/Upsample", [ "NYA_NO_ACCUMULATION" ] )
compile_shader_PS( "PostFX/BrightPass" )

# Editor
compile_shader_VS( "Editor/IBLProbeConvolution" )
compile_shader_PS( "Editor/IBLProbeConvolution" )

# Shared
compile_shader_VS( "FullscreenTriangle" )
compile_shader_PS( "CopyTexture" )
compile_shader_VS( "LineRendering" )
compile_shader_PS( "LineRendering" )

compile_shader_PS( "MSAAResolve", [ "NYA_MSAA_X2", "NYA_USE_TAA" ] )
compile_shader_PS( "MSAAResolve", [ "NYA_MSAA_X4", "NYA_USE_TAA" ] )
compile_shader_PS( "MSAAResolve", [ "NYA_MSAA_X8", "NYA_USE_TAA" ] )

# HUD
compile_shader_VS( "HUD/Primitive" )
compile_shader_PS( "HUD/Primitive" )

# UI
compile_shader_VS( "UI/SDFTextRendering" )
compile_shader_PS( "UI/SDFTextRendering" )

# # Lighting
# compile_shader_CS( "Lighting/LightCulling" )
# compile_shader_VS( "Lighting/Ubersurface", [ "NYA_SCALE_UV_BY_MODEL_SCALE" ] )
# compile_shader_VS( "Lighting/UberDepthOnly", [ "NYA_SCALE_UV_BY_MODEL_SCALE" ] )
# compile_shader_PS( "Lighting/UberDepthOnly" )
# compile_shader_PS( "Lighting/Ubersurface", [ "NYA_EDITOR", "NYA_TERRAIN", "NYA_BRDF_STANDARD", "NYA_PROBE_CAPTURE", "NYA_ENCODE_RGBD", "NYA_USE_LOD_ALPHA_BLENDING", "NYA_USE_NORMAL_MAPPING", "NYA_RECEIVE_SHADOW", "NYA_CAST_SHADOW", "NYA_DEBUG_CSM_CASCADE" ] )
# compile_shader_PS( "Lighting/Ubersurface", [ "NYA_EDITOR", "NYA_TERRAIN", "NYA_BRDF_CLEAR_COAT", "NYA_PROBE_CAPTURE", "NYA_ENCODE_RGBD", "NYA_USE_LOD_ALPHA_BLENDING", "NYA_USE_NORMAL_MAPPING", "NYA_RECEIVE_SHADOW", "NYA_CAST_SHADOW", "NYA_DEBUG_CSM_CASCADE" ] )
# compile_shader_PS( "Lighting/Ubersurface", [ "NYA_EDITOR", "NYA_TERRAIN", "NYA_BRDF_EMISSIVE", "NYA_PROBE_CAPTURE", "NYA_ENCODE_RGBD", "NYA_USE_LOD_ALPHA_BLENDING", "NYA_USE_NORMAL_MAPPING", "NYA_RECEIVE_SHADOW", "NYA_CAST_SHADOW", "NYA_DEBUG_CSM_CASCADE" ] )
# compile_shader_PS( "Lighting/Ubersurface", [ "NYA_DEBUG_IBL_PROBE", "NYA_ENCODE_RGBD" ] )
