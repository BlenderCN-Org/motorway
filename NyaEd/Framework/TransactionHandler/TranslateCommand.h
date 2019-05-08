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

class LocalTranslateCommand : public TransactionCommand
{
public:
    LocalTranslateCommand( Transform* transformToEdit, const nyaVec3f& translationValue )
        : transform( transformToEdit )
        , translation( translationValue )
        , translationBackup( 0, 0, 0 )
    {
        translationBackup = transform->getLocalTranslation();

        actionInfos = "Apply Local Translation";
    }

    virtual void execute() override
    {
        transform->setLocalTranslation( translation );
    }

    virtual void undo() override
    {
        transform->setLocalTranslation( translationBackup );
    }

private:
    Transform*  transform;
    nyaVec3f   translation;
    nyaVec3f   translationBackup;
};

class WorldTranslateCommand : public TransactionCommand
{
public:
    WorldTranslateCommand( Transform* transformToEdit, const nyaVec3f& translationValue )
        : transform( transformToEdit )
        , translation( translationValue )
        , translationBackup( 0, 0, 0 )
    {
        translationBackup = transform->getWorldTranslation();

        actionInfos = "Apply World Translation";
    }

    virtual void execute() override
    {
        transform->setWorldTranslation( translation );
    }

    virtual void undo() override
    {
        transform->setWorldTranslation( translationBackup );
    }

private:
    Transform*  transform;
    nyaVec3f   translation;
    nyaVec3f   translationBackup;
};
