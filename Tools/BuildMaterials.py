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
shader_folder = "../Flan/Graphics/Shaders/"
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

def compile_material( filename_input, filename_output, entrypoint = 'EntryPointPS', flags = {} ):
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
         
def read_value_string( value ):
	str = value.replace( "\r", "" ).replace( '"', '' ).replace( '\n', '' ).replace( '<', '' ).replace( '>', '' )
	print( str )
	return str[1:] if str.startswith( ' ' ) else str
	
def read_value_integer( value ):
	return int( value )
	
def read_value_float( value ):
	return float( value )
	
def read_value_boolean( value ):
	return True if value is 'True' else False
	
def read_value_vector( value ):
	return value[1:-1].replace( ' ', '' ).split( ',' )

def read_value_rgb( value ):
	spaceless_value = value.replace( ' ', '' )
	
	return [float( spaceless_value[offset:( offset + 2)] ) / 255.0 for offset in range( 1, 7, 2 )] 

def read_value_input( value ):
	if value.startswith( '{' ) and value.endswith( '}' ):
		return read_value_vector( value )
	elif value.startswith( '"' ) and value.endswith( '"' ):
		return read_value_string( value )
	elif value.startswith( '#' ):
		return read_value_rgb( value )
	elif value is '':
		return None
	else:
		return read_value_float( value )

# Parse script args
parser = argparse.ArgumentParser(description='Flan Game Engine. Compile materials shaders permutations for Graphics backends.')
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
parser.add_argument('material', nargs='?', help='path to the material to compile')
				   
args = parser.parse_args()

isProd = args.isProd
compileD3D11 = args.compileD3D11
compileSPIRV = args.compileSPIRV
forceRecompile = args.forceRecompile
material = args.material

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

if material is '':
	print( "error: missing material name!" )
	exit()
	
material_file = codecs.open( material, "r", encoding = 'utf-8', errors = 'ignore' )

material_name = ' '
material_version = 0
material_shading_model = ''
material_write_velocity = False
material_enable_alpha_test = False
material_enable_alpha_blend = False
material_enable_alpha_to_coverage = False
material_is_double_face = False
material_cast_shadow = False
material_receive_shadow = False
material_scale_uv_by_model_scale = False

material_base_color = None
material_alpha_mask = None
material_reflectance = None
material_refraction = None
material_refraction_ior = None
material_metalness = None
material_ambient_occlusion = None
material_normal = None
material_roughness = None
material_emissivity = None
material_scale = None

for line in material_file:
	try:
		key, value = line[:line.find( ';' )].split( ':' )
		value = value[1:] if value.startswith( ' ' ) else value
		value = value.replace( "\r", "" ).replace( '\n', '' )
		#print( key, value )
		
		if 'Name' in key:
			material_name = read_value_string( value )
		elif 'Version' in key:
			material_version = read_value_integer( value )
		elif 'ShadingModel' in key:
			material_shading_model = read_value_string( value )
		elif 'WriteVelocity' in key:
			material_write_velocity = read_value_boolean( value )
		elif 'EnableAlphaTest' in key:
			material_enable_alpha_test = read_value_boolean( value )
		elif 'EnableAlphaBlend' in key:
			material_enable_alpha_blend = read_value_boolean( value )
		elif 'EnableAlphaToCoverage' in key:
			material_enable_alpha_to_coverage = read_value_boolean( value )
		elif 'IsDoubleFace' in key:
			material_is_double_face = read_value_boolean( value )
		elif 'CastShadow' in key:
			material_cast_shadow = read_value_boolean( value )
		elif 'ReceiveShadow' in key:
			material_receive_shadow = read_value_boolean( value )
		elif 'ScaleUVByModelScale' in key:
			material_scale_uv_by_model_scale = read_value_boolean( value )
		elif 'BaseColor' in key:
			material_base_color = read_value_input( value )
		elif 'AlphaMask' in key:
			material_alpha_mask = read_value_input( value )
		elif 'Reflectance' in key:
			material_reflectance = read_value_input( value )
		elif 'Refraction' in key:
			material_refraction = read_value_input( value )
		elif 'RefractionIor' in key:
			material_refraction_ior = read_value_input( value )
		elif 'Metalness' in key:
			material_metalness = read_value_input( value )
		elif 'AmbientOcclusion' in key:
			material_ambient_occlusion = read_value_input( value )
		elif 'Normal' in key:
			material_normal = read_value_input( value )
		elif 'Roughness' in key:
			material_roughness = read_value_input( value )
		elif 'Emissivity' in key:
			material_emissivity = read_value_input( value )
		elif 'Scale' in key:
			material_scale = read_value_vector( value )
	except ValueError:
		print( "warning: line skipped" )
	
define_list = {}

def add_input_define( value, name, swizzle_mask ):
	if type( value ) is float:
		return str( value ) + "f"
	elif type( value ) is bool:
		return "true" if value is True else "false"
	elif type( value ) is str:
		return "g_Tex" + name + ".Sample( g_" + name + "Sampler, uvCoord * g_LayerScale )." + swizzle_mask
	elif type( value ) is list:
		if len( value ) is 2:
			return "float2( " + value[0] + ", " + value[1] + " )"
		elif len( value ) is 3:
			return "float3( " + value[0] + ", " + value[1] + ", " + value[2] + " )"
		elif len( value ) is 4:
			return "float4( " + value[0] + ", " + value[1] + ", " + value[2] + ", " + value[3] + " )"
	
define_list["PA_SHADING_MODEL"] = material_shading_model

if material_write_velocity:
	define_list["PA_WRITE_VELOCITY"] = 1

if material_enable_alpha_test:
	define_list["PA_ENABLE_ALPHA_TEST"] = 1

if material_enable_alpha_blend:
	define_list["PA_ENABLE_ALPHA_BLEND"] = 1
	
if material_enable_alpha_to_coverage:
	define_list["PA_ENABLE_ALPHA_TO_COVERAGE"] = 1

if material_is_double_face:
	define_list["PA_IS_DOUBLE_FACE"] = 1
	
#if not material_receive_shadow:
#	define_list["PA_NO_SHADOW"] = 1
	
define_list["PA_BASE_COLOR"] = add_input_define( material_base_color, "BaseColor", "rgb" )
define_list["PA_ALPHA_MASK"] = add_input_define( material_alpha_mask, "AlphaMask", "r" )
define_list["PA_REFLECTANCE"] = add_input_define( material_reflectance, "Reflectance", "r" )
define_list["PA_REFRACTION"] = add_input_define( material_refraction, "Refraction", "r" )
define_list["PA_REFRACTION_IOR"] = add_input_define( material_refraction_ior, "RefractionIor", "r" )
define_list["PA_METALNESS"] = add_input_define( material_metalness, "Metalness", "r" )
define_list["PA_AMBIENT_OCCLUSION"] = add_input_define( material_ambient_occlusion, "AmbientOcclusion", "r" )
define_list["PA_NORMAL"] = add_input_define( material_normal, "Normal", "rgb" )
define_list["PA_ROUGHNESS"] = add_input_define( material_roughness, "Roughness", "r" )
define_list["PA_EMISSIVITY"] = add_input_define( material_emissivity, "Emissivity", "rgb" )
define_list["PA_SCALE"] = add_input_define( material_scale, "Scale", "rg" )

print( define_list )