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
#ifndef __SHARED_H__
#define __SHARED_H__ 1

#define MAX_POINT_LIGHT_COUNT               256
#define MAX_SPOT_LIGHT_COUNT                256
#define MAX_DIRECTIONAL_LIGHT_COUNT         1

#define MAX_SPHERE_LIGHT_COUNT              8
#define MAX_DISC_LIGHT_COUNT                8
#define MAX_RECTANGLE_LIGHT_COUNT           16

#define MAX_LOCAL_ENVIRONMENT_PROBE_COUNT   31
#define MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT  1

#define MAX_ENVIRONMENT_PROBE_COUNT         ( MAX_LOCAL_ENVIRONMENT_PROBE_COUNT + MAX_GLOBAL_ENVIRONMENT_PROBE_COUNT )

#endif
