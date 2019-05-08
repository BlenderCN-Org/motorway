/*
    Copyright (C) 2018 Team Horsepower
    https://github.com/ProjectHorsepower

    This file is part of Project Horsepower source code.

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

#if NYA_WIN
#include "FileSystemIOHelpers.h"

#include <commdlg.h>

bool nya::core::DisplayFileOpenPrompt( nyaString_t& selectedFilename, const nyaChar_t* filetypeFilter, const nyaString_t& initialDirectory, const nyaString_t& promptTitle )
{
    selectedFilename.resize( MAX_PATH );

    OPENFILENAMEW ofn = {};
    HANDLE hf = {};

    ZeroMemory( &ofn, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = &selectedFilename[0];

    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filetypeFilter; 
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = initialDirectory.c_str();
    ofn.lpstrTitle = promptTitle.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    // Display the Open dialog box.
    return GetOpenFileName( &ofn ) == TRUE;
}

bool nya::core::DisplayFileSavePrompt( nyaString_t& selectedFilename, const nyaChar_t* filetypeFilter, const nyaString_t& initialDirectory, const nyaString_t& promptTitle )
{
    selectedFilename.resize( MAX_PATH );

    OPENFILENAMEW ofn = {};
    HANDLE hf = {};

    ZeroMemory( &ofn, sizeof( ofn ) );
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = &selectedFilename[0];

    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filetypeFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = initialDirectory.c_str();
    ofn.lpstrTitle = promptTitle.c_str();
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

    // Display the Open dialog box.
    return GetSaveFileName( &ofn ) == TRUE;
}
#endif
