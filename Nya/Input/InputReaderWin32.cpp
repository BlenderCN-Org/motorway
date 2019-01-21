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

#ifdef NYA_WIN
#include "InputReaderWin32.h"

#include "InputReader.h"

using namespace nya::input;

void nya::input::CreateInputReaderImpl( nya::input::eInputLayout& activeInputLayout )
{
    NYA_CLOG << "Creating InputReader (WINAPI)" << std::endl;

    // Check if there is at least one input device available on the system
    // Modern systems should support raw device querying
    UINT inputDeviceCount = 0;
    GetRawInputDeviceList( nullptr, &inputDeviceCount, sizeof( RAWINPUTDEVICELIST ) );

    if ( inputDeviceCount == 0 ) {
        NYA_CERR << "No input device available" << std::endl;
        return;
    }

    RAWINPUTDEVICELIST* deviceList = new RAWINPUTDEVICELIST[inputDeviceCount]{ 0 };
    GetRawInputDeviceList( deviceList, &inputDeviceCount, sizeof( RAWINPUTDEVICELIST ) );

    std::vector<RAWINPUTDEVICE> devicesToRegister;
    for ( UINT i = 0; i < inputDeviceCount; i++ ) {
        auto device = deviceList[i];

        if ( device.dwType == RIM_TYPEMOUSE ) {
            devicesToRegister.push_back( {} );

            // Register Raw Mouse (usage page and usage should always be 1 and 2 respectively)
            auto& deviceMouse = devicesToRegister.back();
            deviceMouse.usUsagePage = 0x01;
            deviceMouse.usUsage     = 0x02;
            deviceMouse.dwFlags     = 0x00;
            deviceMouse.hwndTarget  = NULL;
        } else if ( device.dwType == RIM_TYPEKEYBOARD ) {
            // Get Device infos
            UINT deviceInfosSize = sizeof( RID_DEVICE_INFO );
            RID_DEVICE_INFO deviceInfos = { 0 };
            deviceInfos.cbSize = sizeof( RID_DEVICE_INFO );
            GetRawInputDeviceInfo( device.hDevice, RIDI_DEVICEINFO, &deviceInfos, &deviceInfosSize );

            nyaChar_t inputLayoutName[KL_NAMELENGTH + 1] = { '\0' };
            GetKeyboardLayoutName( inputLayoutName );

            auto userInputLayout = std::stoul( inputLayoutName, NULL, 16 );

            switch ( userInputLayout ) {
            case 0x00020401:
            case 0x0001080c:
            case 0x0000080c:
            case 0x0000040c:
                activeInputLayout = INPUT_LAYOUT_AZERTY;
                break;

            case 0x0000041a:
            case 0x00000405:
            case 0x00000407:
            case 0x00010407:
            case 0x0000040e:
            case 0x0000046e:
            case 0x00010415:
            case 0x00000418:
            case 0x0000081a:
            case 0x0000041b:
            case 0x00000424:
            case 0x0001042e:
            case 0x0002042e:
            case 0x0000042e:
            case 0x0000100c:
            case 0x00000807:
                activeInputLayout = INPUT_LAYOUT_QWERTZ;
                break;

                /* case 0x00010409:
                 case 0x00030409:
                 case 0x00040409:
                     activeInputLayout = INPUT_LAYOUT_DVORAK;
                     break;*/

            default:
                activeInputLayout = INPUT_LAYOUT_QWERTY;
                break;
            }

            devicesToRegister.push_back( {} );

            // Register Raw Keyboard (usage page and usage should always be 1 and 6 respectively)
            auto& deviceKeyboard = devicesToRegister.back();
            deviceKeyboard.usUsagePage = 0x01;
            deviceKeyboard.usUsage = 0x06;
            deviceKeyboard.dwFlags = 0x00;
            deviceKeyboard.hwndTarget = NULL;
        }
        //} else if ( device.dwType == RIM_TYPEHID ) {
        //    // Get Device infos
        //    UINT deviceInfosSize = sizeof( RID_DEVICE_INFO );
        //    RID_DEVICE_INFO deviceInfos = { 0 };
        //    deviceInfos.cbSize = sizeof( RID_DEVICE_INFO );
        //    GetRawInputDeviceInfo( device.hDevice, RIDI_DEVICEINFO, &deviceInfos, &deviceInfosSize );

        //    NYA_CLOG << "Found unsupported input device with vendorid: " << std::hex << deviceInfos.hid.dwVendorId
        //        << " and productId: " << std::hex << deviceInfos.hid.dwProductId << " " << std::endl;
        //}
    }

    delete[] deviceList;

    BOOL isRegistered = RegisterRawInputDevices( &devicesToRegister[0], static_cast<UINT>( devicesToRegister.size() ), sizeof( RAWINPUTDEVICE ) );

    NYA_CLOG << "::RegisterRawInputDevices >> operation result : " << ( ( isRegistered == TRUE ) ? "OK" : "FAILED" ) << std::endl;
}

void ReadKeyboard( InputReader* inputReader, const RAWINPUT& input, const uint64_t timestamp )
{
    bool isKeyDown = ( input.data.keyboard.Message == WM_KEYDOWN );
    inputReader->pushKeyEvent( { WindowsKeys[input.data.keyboard.VKey], isKeyDown } );
}

void ReadMouse( InputReader* inputReader, const RAWINPUT& input, const uint64_t timestamp )
{
    // Update Mouse Axis
    inputReader->pushAxisEvent( { eInputAxis::MOUSE_X, 0, static_cast<double>( input.data.mouse.lLastX ) } );
    inputReader->pushAxisEvent( { eInputAxis::MOUSE_Y, 0, static_cast<double>( input.data.mouse.lLastY ) } );
  
    // Update mouse buttons state
    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_1_DOWN )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_LEFT_BUTTON, true } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_1_UP )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_LEFT_BUTTON, false } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_2_DOWN )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_RIGHT_BUTTON, true } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_2_UP )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_RIGHT_BUTTON, false } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_3_DOWN )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_MIDDLE_BUTTON, true } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_3_UP )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_MIDDLE_BUTTON, false } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_4_DOWN )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_EXTRA_1_BUTTON, true } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_4_UP )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_EXTRA_1_BUTTON, false } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_5_DOWN )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_EXTRA_2_BUTTON, true } );

    if ( input.data.mouse.ulButtons & RI_MOUSE_BUTTON_5_UP )
        inputReader->pushKeyEvent( { eInputKey::MOUSE_EXTRA_2_BUTTON, false } );

    // Update Mouse Wheel
    if ( input.data.mouse.usButtonFlags & RI_MOUSE_WHEEL )
        inputReader->pushAxisEvent( { eInputAxis::MOUSE_SCROLL_WHEEL, 0, static_cast<double>( input.data.mouse.usButtonData ) } );
}

void nya::input::ProcessInputEventImpl( InputReader* inputReader, const HWND win, const WPARAM infos, const uint64_t timestamp )
{
    RAWINPUT rawInput = {};
    UINT szData = sizeof( RAWINPUT );

    GetRawInputData( ( HRAWINPUT )infos, RID_INPUT, &rawInput, &szData, sizeof( RAWINPUTHEADER ) );

    if ( rawInput.header.dwType == RIM_TYPEMOUSE ) {
        ReadMouse( inputReader, rawInput, timestamp );

        // Get absolute mouse coordinates
        POINT p;
        GetCursorPos( &p );
        ScreenToClient( win, &p );

        inputReader->setAbsoluteAxisValue( eInputAxis::MOUSE_X, p.x );
        inputReader->setAbsoluteAxisValue( eInputAxis::MOUSE_Y, p.y );
    } else if ( rawInput.header.dwType == RIM_TYPEKEYBOARD ) {
        ReadKeyboard( inputReader, rawInput, timestamp );
    }
}
#endif
