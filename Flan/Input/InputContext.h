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
    using ConversionMap_t = std::map<fnStringHash_t, Converter>;

    // Construction
public:
    RangeConverter() {}
    ~RangeConverter() { ConversionMap.clear(); }

    void addRangeToConverter( fnStringHash_t range, double inputMin, double inputMax, double outputMin, double outputMax )
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
    RangeType convert( fnStringHash_t rangeid, RangeType invalue ) const
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
    const inline RangeConverter& getConversions() const { return *rangeConverter.get(); }

public:
            InputContext();
            InputContext( InputContext& ) = default;
            ~InputContext();

public:
    void    bindActionToButton( flan::core::eInputKey button, fnStringHash_t action );
    void    bindStateToButton( flan::core::eInputKey button, fnStringHash_t state );
    void    bindRangeToAxis( flan::core::eInputAxis axis, fnStringHash_t range );
    void    bindSensitivityToRange( double sensitivity, fnStringHash_t range );
    void    setRangeDataRange( double inputMin, double inputMax, double outputMin, double outputMax, fnStringHash_t range );

    bool    mapButtonToAction( flan::core::eInputKey button, fnStringHash_t& out ) const;
    bool    mapButtonToState( flan::core::eInputKey button, fnStringHash_t& out ) const;
    bool    mapAxisToRange( flan::core::eInputAxis axis, fnStringHash_t& out ) const;
    double  getSensitivity( fnStringHash_t range ) const;

private:
    std::map<flan::core::eInputKey, fnStringHash_t>     actionMap;
    std::map<flan::core::eInputKey, fnStringHash_t>     stateMap;
    std::map<flan::core::eInputAxis, fnStringHash_t>    rangeMap;
    std::map<fnStringHash_t, double>                    sensitivityMap;

    std::unique_ptr<RangeConverter>         rangeConverter;
};
