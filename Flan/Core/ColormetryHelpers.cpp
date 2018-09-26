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
#include "Shared.h"
#include "ColormetryHelpers.h"

glm::vec3 Core_TemperatureToRGB( const float temperatureInKelvin )
{
    float x = temperatureInKelvin / 1000.0f;
    float x2 = x * x;
    float x3 = x2 * x;
    float x4 = x3 * x;
    float x5 = x4 * x;

    float R = 0.0f, G = 0.0f, B = 0.0f;

    // Red Channel
    if ( temperatureInKelvin <= 6600.0f ) {
        R = 1.0f;
    } else {
        R = 0.0002889f * x5 - 0.01258f * x4 + 0.2148f * x3 - 1.776f * x2 + 6.907f * x - 8.723f;
    }

    // Green Channel
    if ( temperatureInKelvin <= 6600.0f ) {
        G = -4.593e-05f * x5 + 0.001424f * x4 - 0.01489f * x3 + 0.0498f * x2 + 0.1669f * x - 0.1653f;
    } else {
        G = -1.308e-07f * x5 + 1.745e-05f * x4 - 0.0009116f * x3 + 0.02348f * x2 - 0.3048f * x + 2.159f;
    }

    // Blue Channel
    if ( temperatureInKelvin <= 2000.0f ) {
        B = 0.0f;
    } else if ( temperatureInKelvin < 6600.0f ) {
        B = 1.764e-05f * x5 + 0.0003575f * x4 - 0.01554f * x3 + 0.1549f * x2 - 0.3682f * x + 0.2386f;
    } else {
        B = 1.0f;
    }

    return glm::vec3( R, G, B );
}

float Core_RGBToTemperature( const float colorRedChannel, const float colorBlueChannel )
{
    // NOTE This is actually an approximation which might not be accurate for extreme values
    float tMin = 2000.0f, tMax = 23000.0f;
    float colorTemperature = 0.0f;

    for ( colorTemperature = ( tMax + tMin ) / 2.0f; tMax - tMin > 0.1f; colorTemperature = ( tMax + tMin ) / 2.0f ) {
        glm::vec3 newColor = Core_TemperatureToRGB( colorTemperature );

        if ( newColor.z / newColor.x > colorBlueChannel / colorRedChannel ) {
            tMax = colorTemperature;
        } else {
            tMin = colorTemperature;
        }
    }

    return colorTemperature;
}

glm::vec3 Core_SRGBToLinear( const glm::vec3& srgbColor )
{
    glm::vec3 linearRGBLo = srgbColor / 12.92f;
    glm::vec3 linearRGBHi = glm::pow( ( srgbColor + 0.055f ) / 1.055f, glm::vec3( 2.4f ) );

    return ( ( srgbColor.r ) <= 0.04045f && ( srgbColor.g ) <= 0.04045f && ( srgbColor.b ) <= 0.04045f ) ? linearRGBLo : linearRGBHi;
}

#if FLAN_DEVBUILD
#include <imgui/imgui.h>

static constexpr int intensityPresetCount = 12;
static constexpr char* intensityPresetLabels[intensityPresetCount] = { "LED 37 mW", "Candle flame", "LED 1W", "Incandescent lamp 40W", "Fluorescent lamp", "Tungsten Bulb 100W", "Fluorescent light", "HID Car headlight", "Induction Lamp 200W", "Low pressure sodium vapor lamp 127W", "Metal-halide lamp 400W", "Direct sunlight" };
static constexpr float intensityPresetValues[intensityPresetCount] = { 0.20f, 15.0f, 75.0f, 325.0f, 1200.0f, 1600.0f, 2600.0f, 3000.0f, 16000.0f, 25000.0f, 40000.0f, 64000.0f };

static constexpr int temperaturePresetCount = 13;
// presets from https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf and some google search
static constexpr char* temperaturePresetLabels[temperaturePresetCount] = { "Candle flame", "Tungsten bulb", "Tungsten lamp 500W-1K", "Quartz light", "Tungsten lamp 2K",
"Tungsten lamp 5K/10K", "Mecury vapor bulb", "Daylight", "RGB Monitor", "Fluorescent lights", "Sky overcast", "Outdoor shade areas", "Sky flanrtly cloudy" };
static constexpr float temperaturePresetValues[temperaturePresetCount] = { 1930.0f, 2900.0f, 3000.0f, 3500.0f, 3275.0f, 3380.0f, 5700.0f, 6000.0f, 6280.0f, 6500.0f, 7500.0f, 8000.0f, 9000.0f };

static constexpr char* colorMode[2] = { "RGB", "Temperature" };

static flan::editor::eColorMode activeColorMode = flan::editor::COLOR_MODE_SRGB;

bool flan::editor::PanelLuminousIntensity( float& luminousIntensity )
{
    bool hasChanged = false;

    hasChanged = ImGui::SliderFloat( "Luminous Intensity (lm)", &luminousIntensity, 0.0f, 100000.0f );

    int intensityPresetIndex = -1;
    if ( ImGui::Combo( "Luminous Intensity Preset (lm)", &intensityPresetIndex, intensityPresetLabels, intensityPresetCount ) 
        && intensityPresetIndex != -1 ) {
        if ( !hasChanged ) {
            hasChanged = true;
        }

        luminousIntensity = intensityPresetValues[intensityPresetIndex];
    }

    return hasChanged;
}

bool flan::editor::PanelColor( eColorMode& activeColorMode, glm::vec3& colorRGB )
{
    bool hasChanged = false;

    ImGui::Combo( "Color Mode", (int*)&activeColorMode, colorMode, 2 );

    if ( activeColorMode == 0 ) {
        hasChanged = ImGui::ColorEdit3( "Color (sRGB)", ( float* )&colorRGB[0] );
    } else {
        float colorTemperature = Core_RGBToTemperature( colorRGB[0], colorRGB[2] );

        hasChanged = ImGui::SliderFloat( "Temperature (K)", &colorTemperature, 0.0f, 9000.0f );

        int temperaturePresetIndex = -1;
        if ( ImGui::Combo( "Temperature Preset (K)", &temperaturePresetIndex, temperaturePresetLabels, temperaturePresetCount ) && temperaturePresetIndex != -1 ) {
            colorTemperature = temperaturePresetValues[temperaturePresetIndex];
            hasChanged = true;
        }

        colorRGB = Core_TemperatureToRGB( colorTemperature );
    }

    return hasChanged;
}
#endif
