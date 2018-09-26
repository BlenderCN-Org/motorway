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
#include "InputMapper.h"

#include "MappedInput.h"
#include "InputContext.h"

#include <Io/TextStreamHelpers.h>
#include <Core/StringHelpers.h>

using namespace flan::core;

InputMapper::InputMapper()
{

}

InputMapper::~InputMapper()
{
    clear();

    for ( auto iter = inputContexts.begin(); iter != inputContexts.end(); ++iter ) {
        delete iter->second;
    }

    activeContexts.clear();
    callbackTable.clear();
}

void InputMapper::update( const float frameTime )
{
    MappedInput input = currentMappedInput;

    for ( auto iter = callbackTable.begin(); iter != callbackTable.end(); ++iter ) {
        iter->second( input, frameTime );
    }
}

void InputMapper::clear()
{
    currentMappedInput.Actions.clear();
    currentMappedInput.Ranges.clear();
}

void InputMapper::setRawButtonState( eInputKey button, bool pressed, bool previouslypressed )
{
    fnStringHash_t action;
    fnStringHash_t state;

    bool isActionMapped = false,
         isStateMapped = false;

    if ( pressed && !previouslypressed ) {
        if ( isActionMapped = mapButtonToAction( button, action ) ) {
            currentMappedInput.Actions.insert( action );
        }
    }

    if ( pressed ) {
        if ( isStateMapped = mapButtonToState( button, state ) ) {
            currentMappedInput.States.insert( state );
        }
    }

    if ( !isStateMapped && !isActionMapped ) {
        mapAndEatButton( button );
    }
}

void InputMapper::setRawAxisValue( eInputAxis axis, double value )
{
    for ( auto iter = activeContexts.begin(); iter != activeContexts.end(); ++iter ) {
        const InputContext* context = *iter;

        fnStringHash_t range;
        if ( context->mapAxisToRange( axis, range ) ) {
            currentMappedInput.Ranges[range] = context->getConversions().convert( range, value * context->getSensitivity( range ) );
            break;
        }
    }
}

void InputMapper::addCallback( InputCallback_t callback, int priority )
{
    callbackTable.insert( std::make_pair( priority, callback ) );
}

void InputMapper::addContext( const fnStringHash_t name, InputContext* context )
{
    inputContexts.emplace( name, context );
}

void InputMapper::pushContext( const fnStringHash_t name )
{
    auto iter = inputContexts.find( name );
    if ( iter == inputContexts.end() ) {
        return;
    }

    activeContexts.push_front( iter->second );
}

void InputMapper::popContext()
{
    activeContexts.pop_front();
}

bool InputMapper::mapButtonToAction( eInputKey button, fnStringHash_t& action ) const
{
    for ( auto iter = activeContexts.begin(); iter != activeContexts.end(); ++iter ) {
        const InputContext* context = *iter;

        if ( context->mapButtonToAction( button, action ) )
            return true;
    }

    return false;
}

bool InputMapper::mapButtonToState( eInputKey button, fnStringHash_t& state ) const
{
    for ( auto iter = activeContexts.begin(); iter != activeContexts.end(); ++iter ) {
        const InputContext* context = *iter;

        if ( context->mapButtonToState( button, state ) )
            return true;
    }

    return false;
}

void InputMapper::mapAndEatButton( eInputKey button )
{
    fnStringHash_t action;
    fnStringHash_t state;

    if ( mapButtonToAction( button, action ) )
        currentMappedInput.eatAction( action );

    if ( mapButtonToState( button, state ) )
        currentMappedInput.eatState( state );
}

#define ButtonType( option ) option( NONE ) option( ACTION ) option( STATE ) option( AXIS )
FLAN_LAZY_ENUM( ButtonType )
#undef ButtonType

void InputMapper::deserialize( FileSystemObject* file )
{
    fnString_t streamLine, dictionaryKey, dictionaryValue;

    bool isReadingContext = false;
    InputContext* currentContext = nullptr;
    while ( file->isGood() ) {
        flan::core::ReadString( file, streamLine );

        // Find seperator character offset in the line (if any)
        const auto commentSeparator = streamLine.find_first_of( FLAN_STRING( "#" ) );

        // Remove user comments before reading the keypair value
        if ( commentSeparator != fnString_t::npos ) {
            streamLine.erase( streamLine.begin() + commentSeparator, streamLine.end() );
        }

        // Skip commented out and empty lines
        if ( streamLine.empty() ) {
            continue;
        }

        if ( streamLine[0] == '{' ) {
            continue;
        } else if ( streamLine[0] == '}' ) {
            isReadingContext = false;
            currentContext = nullptr;
            continue;
        }

        const auto keyValueSeparator = streamLine.find_first_of( ':' );

        // Check if this is a key value line
        if ( keyValueSeparator != fnString_t::npos ) {
            dictionaryKey = streamLine.substr( 0, keyValueSeparator );
            dictionaryValue = streamLine.substr( keyValueSeparator + 1 );

            // Trim both key and values (useful if a file has inconsistent spacing, ...)
            flan::core::TrimString( dictionaryKey );
            flan::core::TrimString( dictionaryValue );

            // Do the check after triming, since the value might be a space or a tab character
            if ( !dictionaryValue.empty() ) {
                auto keyHashcode = CRC32( dictionaryKey.c_str() );

                switch ( keyHashcode ) {
                case FLAN_STRING_HASH( "Context" ): {
                    auto contextHashcode = CRC32( flan::core::WrappedStringToString( dictionaryValue ) );

                    auto iterator = inputContexts.find( contextHashcode );
                    currentContext = ( iterator != inputContexts.end() ) ? iterator->second : new InputContext();

                    addContext( contextHashcode, currentContext );
                } break;

                default:
                    if ( currentContext == nullptr ) {
                        FLAN_CERR << "State " << dictionaryValue << " does not belong to any input context (if this is intended to be a global state, use one of the global context)" << std::endl;
                        break;
                    }

                    if ( dictionaryValue.front() != '{' || dictionaryValue.back() != '}' ) {
                        FLAN_CERR << "State " << dictionaryValue << " : invalid syntax (expected opening and closing brackets)" << std::endl;
                        break;
                    }

                    std::size_t buttonTypeEndOffset = dictionaryValue.find_first_of( ',' );
                    fnString_t buttonTypeString = dictionaryValue.substr( 1, buttonTypeEndOffset - 1 );
                    flan::core::TrimString( buttonTypeString );

                    auto buttonTypeHashcode = CRC32( buttonTypeString );
                    auto buttonType = StringToButtonType( buttonTypeHashcode );

                    switch ( buttonType ) {
                    case eButtonType::STATE:
                    case eButtonType::ACTION: {
                        std::size_t actionKeyEndOffset = dictionaryValue.find_last_of( ',' );
                        std::size_t vecEnd = dictionaryValue.find_last_of( '}' );

                        fnString_t keyString = dictionaryValue.substr( buttonTypeEndOffset + 1, vecEnd - actionKeyEndOffset - 1 );
                        flan::core::TrimString( keyString );
                        auto inputKeyHashcode = CRC32( keyString );
                        auto key = StringToInputKey( inputKeyHashcode );

                        if ( buttonType == eButtonType::ACTION )
                            currentContext->bindActionToButton( key, keyHashcode );
                        else
                            currentContext->bindStateToButton( key, keyHashcode );
                    } break;

                    case eButtonType::AXIS: {
                        std::size_t vecEnd = dictionaryValue.find_last_of( '}' );

                        std::size_t axisInputEndOffset = dictionaryValue.find_first_of( ',', buttonTypeEndOffset + 1 );
                        fnString_t axisString = dictionaryValue.substr( buttonTypeEndOffset + 1, axisInputEndOffset - buttonTypeEndOffset - 1 );
                        flan::core::TrimString( axisString );
                        auto axisHashcode = CRC32( axisString );
                        auto axis = StringToInputAxis( axisHashcode );

                        std::size_t sensitivityRangeOffset = dictionaryValue.find_first_of( ',', axisInputEndOffset + 1 );
                        fnString_t sensitivityRangeString = dictionaryValue.substr( axisInputEndOffset + 1, sensitivityRangeOffset - axisInputEndOffset - 1 );
                        double axisSensitivity = std::stod( sensitivityRangeString );

                        // Input min, max, output min, output max
                        std::size_t inputMinOffset = dictionaryValue.find_first_of( ',', sensitivityRangeOffset + 1 );
                        fnString_t inputMinString = dictionaryValue.substr( sensitivityRangeOffset + 1, inputMinOffset - sensitivityRangeOffset - 1 );

                        std::size_t inputMaxOffset = dictionaryValue.find_first_of( ',', inputMinOffset + 1 );
                        fnString_t inputMaxString = dictionaryValue.substr( inputMinOffset + 1, inputMaxOffset - inputMinOffset - 1 );

                        std::size_t outputMinOffset = dictionaryValue.find_first_of( ',', inputMaxOffset + 1 );
                        fnString_t outputMinString = dictionaryValue.substr( inputMaxOffset + 1, outputMinOffset - inputMaxOffset - 1 );

                        std::size_t outputMaxOffset = dictionaryValue.find_first_of( ',', outputMinOffset + 1 );
                        fnString_t outputMaxString = dictionaryValue.substr( outputMinOffset + 1, vecEnd - outputMaxOffset - 1 );

                        double inputMin = std::stod( inputMinString );
                        double inputMax = std::stod( inputMaxString );
                        double outputMin = std::stod( outputMinString );
                        double outputMax = std::stod( outputMaxString );

                        currentContext->bindRangeToAxis( axis, keyHashcode );
                        currentContext->bindSensitivityToRange( axisSensitivity, keyHashcode );
                        currentContext->setRangeDataRange( inputMin, inputMax, outputMin, outputMax, keyHashcode );
                    } break;

                    case eButtonType::NONE:
                    default:
                        break;
                    }

                    break;
                }
            }
        }
    }
}
