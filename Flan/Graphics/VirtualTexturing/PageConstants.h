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

static constexpr int PAGE_TABLE_MIP_COUNT = 11;
static constexpr int PAGE_SIZE_IN_PIXELS = 128;
static constexpr int PAGE_TABLE_PER_LINE_PAGE_COUNT = 128;
static constexpr int PAGE_TABLE_PAGE_COUNT = PAGE_TABLE_PER_LINE_PAGE_COUNT * PAGE_TABLE_PER_LINE_PAGE_COUNT;
