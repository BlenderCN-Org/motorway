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

#include "InputKeys.h"
#include "InputAxis.h"
#include <map>

class RangeConverter
{
private:
    struct Converter
    {
        double MinimumInput;
        double MaximumInput;

        double MinimumOutput;
        double MaximumOutput;

        template <typename RangeType>
        RangeType convert( RangeType invalue ) const
        {
            double v = static_cast<double>( invalue );
            if ( v < MinimumInput )
                v = MinimumInput;
            else if ( v > MaximumInput )
                v = MaximumInput;

            double interpolationfactor = ( v - MinimumInput ) / ( MaximumInput - MinimumInput );
            return static_cast<RangeType>( ( interpolationfactor * ( MaximumOutput - MinimumOutput ) ) + MinimumOutput );
        }
    };

private:
    using ConversionMap_t = std::map<nyaStringHash_t, Converter>;

    // Construction
public:
    RangeConverter() {}
    ~RangeConverter() { ConversionMap.clear(); }

    void addRangeToConverter( nyaStringHash_t range, double inputMin, double inputMax, double outputMin, double outputMax )
    {
        Converter converter;
        converter.MinimumInput = inputMin;
        converter.MaximumInput = inputMax;
        converter.MinimumOutput = outputMin;
        converter.MaximumOutput = outputMax;

        ConversionMap.insert( std::make_pair( range, converter ) );
    }

    // Conversion interface
public:
    template <typename RangeType>
    RangeType convert( nyaStringHash_t rangeid, RangeType invalue ) const
    {
        ConversionMap_t::const_iterator iter = ConversionMap.find( rangeid );
        if ( iter == ConversionMap.end() )
            return invalue;

        return iter->second.convert<RangeType>( invalue );
    }

    // Internal tracking
private:
    ConversionMap_t ConversionMap;
};

class InputContext
{
public:
    const inline RangeConverter& getConversions() const { return rangeConverter; }

public:
            InputContext();
            InputContext( InputContext& ) = default;
            ~InputContext();

public:
    void    bindActionToButton( nya::input::eInputKey button, nyaStringHash_t action );
    void    bindStateToButton( nya::input::eInputKey button, nyaStringHash_t state );
    void    bindRangeToAxis( nya::input::eInputAxis axis, nyaStringHash_t range );
    void    bindSensitivityToRange( double sensitivity, nyaStringHash_t range );
    void    setRangeDataRange( double inputMin, double inputMax, double outputMin, double outputMax, nyaStringHash_t range );

    bool    mapButtonToAction( nya::input::eInputKey button, nyaStringHash_t& out ) const;
    bool    mapButtonToState( nya::input::eInputKey button, nyaStringHash_t& out ) const;
    bool    mapAxisToRange( nya::input::eInputAxis axis, nyaStringHash_t& out ) const;
    double  getSensitivity( nyaStringHash_t range ) const;

private:
    std::map<nya::input::eInputKey, nyaStringHash_t>     actionMap;
    std::map<nya::input::eInputKey, nyaStringHash_t>     stateMap;
    std::map<nya::input::eInputAxis, nyaStringHash_t>    rangeMap;
    std::map<nyaStringHash_t, double>                    sensitivityMap;

    RangeConverter  rangeConverter;
};
