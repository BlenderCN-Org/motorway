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

template<typename Precision>
const Vector<Precision, 1> Vector<Precision, 1>::Zero = Vector( ( Precision )0 );

template<typename Precision>
const Vector<Precision, 2> Vector<Precision, 2>::Zero = Vector( ( Precision )0, ( Precision )0 );

template<typename Precision>
const Vector<Precision, 3> Vector<Precision, 3>::Zero = Vector( ( Precision )0, ( Precision )0, ( Precision )0 );

template<typename Precision>
const Vector<Precision, 4> Vector<Precision, 4>::Zero = Vector( ( Precision )0, ( Precision )0, ( Precision )0, ( Precision )0 );

template<typename Precision>
const Vector<Precision, 1> Vector<Precision, 1>::Max = Vector( std::numeric_limits<Precision>::max() );

template<typename Precision>
const Vector<Precision, 2> Vector<Precision, 2>::Max = Vector( std::numeric_limits<Precision>::max(), std::numeric_limits<Precision>::max() );

template<typename Precision>
const Vector<Precision, 3> Vector<Precision, 3>::Max = Vector( std::numeric_limits<Precision>::max(), std::numeric_limits<Precision>::max(), std::numeric_limits<Precision>::max() );

template<typename Precision>
const Vector<Precision, 4> Vector<Precision, 4>::Max = Vector( std::numeric_limits<Precision>::max(), std::numeric_limits<Precision>::max(), std::numeric_limits<Precision>::max(), std::numeric_limits<Precision>::max() );
