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

#include "TransactionCommand.h"

#include <Maths/Transform.h>

class LocalScaleCommand : public TransactionCommand
{
public:
    LocalScaleCommand( Transform* transformToEdit, const nyaVec3f& scaleValue )
        : transform( transformToEdit )
        , scale( scaleValue )
        , scaleBackup( 0, 0, 0 )
    {
        scaleBackup = transform->getLocalScale();

        actionInfos = "Apply Local Scale";
    }

    virtual void execute() override
    {
        transform->setLocalScale( scale );
    }

    virtual void undo() override
    {
        transform->setLocalScale( scaleBackup );
    }

private:
    Transform*  transform;
    nyaVec3f    scale;
    nyaVec3f    scaleBackup;
};

class WorldScaleCommand : public TransactionCommand
{
public:
    WorldScaleCommand( Transform* transformToEdit, const nyaVec3f& scaleValue )
        : transform( transformToEdit )
        , scale( scaleValue )
        , scaleBackup( 0, 0, 0 )
    {
        scaleBackup = transform->getWorldScale();

        actionInfos = "Apply World Scale";
    }

    virtual void execute() override
    {
        transform->setWorldScale( scale );
    }

    virtual void undo() override
    {
        transform->setWorldScale( scaleBackup );
    }

private:
    Transform* transform;
    nyaVec3f    scale;
    nyaVec3f    scaleBackup;
};
