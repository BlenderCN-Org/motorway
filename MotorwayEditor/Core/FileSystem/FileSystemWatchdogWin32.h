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

#if FLAN_DEVBUILD
#if FLAN_WIN

#include <atomic>
#include <queue>

#include <Rendering/ShaderStages.h>

class FileSystemWatchdog
{
public:
            FileSystemWatchdog();
            FileSystemWatchdog( FileSystemWatchdog& ) = delete;
            ~FileSystemWatchdog();

    void    Create();
    void    OnFrame();

private:
    struct ShaderStageToReload
    {
        fnString_t   Filename;
        eShaderStage StageType;
    };

private:
    HANDLE              watchdogHandle;
    std::atomic<bool>   shutdownSignal;

    std::queue<fnString_t> materialsToReload;
    std::queue<fnString_t> texturesToReload;
    std::queue<fnString_t> meshesToReload;
    std::queue<fnString_t> modelsToReload;
    
    std::queue<ShaderStageToReload> shadersToReload;
    
private:
    void    Monitor();
};
#endif
#endif
