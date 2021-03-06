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
#pragma once

namespace nya
{
    namespace core
    {
        bool DisplayFileOpenPrompt( nyaString_t& selectedFilename, const nyaChar_t* filetypeFilter, const nyaString_t& initialDirectory, const nyaString_t& promptTitle );
        bool DisplayFileSavePrompt( nyaString_t& selectedFilename, const nyaChar_t* filetypeFilter, const nyaString_t& initialDirectory, const nyaString_t& promptTitle );
    }
}
