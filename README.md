# Project Motorway

## LICENSE
See COPYING.txt for the GNU GENERAL PUBLIC LICENSE
Some source code in this release is not covered by the GPL (see LICENCES.txt).

## FILE LAYOUT
| Path        | Description |
| ------------- |-------------|
CMakeModules/       | modules for CMake build scripts<br>
Flan/               | Flan Engine source code (renderer, low-level layer, core stuff, etc.)<br>
MotorwayClient/     | Motorway client (client logic specifics and OS entrypoint)<br>
MotorwayEditor/     | all-in-one editor for Motorway (material, level, game)<br>
MotorwayServer/     | Motorway server<br>
ThirdParty/         | third-party libs and software<br>
Tools/              | scripts for build environment and external softwares<br>
bin/             	| default output path for binaries (contains assets and runtime files too)<br>

(note that the layout might change)

## COMPILING
Clone and build third party libraries ('ThirdParty/' folder).
Build files should be generated using CMake.

Each binary has its own build script (see CMakeLists.txt), and can be generated
independantly (as long as the Engine library is compiled).

Shaders binaries should be generated before game launch (using 'Tools/CompileShaders.py' python script).

Assets files can be either be unzipped ('bin/Game.zip' archive to a 'bin/data/' folder) 
or can be left as is.

## TODO
See TODO.txt.
