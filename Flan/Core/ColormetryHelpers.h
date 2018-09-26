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

#pragma once

#include <glm/glm.hpp>

glm::vec3 Core_TemperatureToRGB( const float temperatureInKelvin );
float Core_RGBToTemperature( const float colorRedChannel, const float colorBlueChannel );

glm::vec3 Core_SRGBToLinear( const glm::vec3& srgbColor );

#if FLAN_DEVBUILD
namespace flan
{
    namespace editor
    {
        enum eColorMode : int
        {
            COLOR_MODE_SRGB = 0,
            COLOR_MODE_TEMPERATURE
        };

        bool PanelLuminousIntensity( float& luminousIntensity );
        bool PanelColor( eColorMode& activeColorMode, glm::vec3& colorRGB );
    }
}
#endif
