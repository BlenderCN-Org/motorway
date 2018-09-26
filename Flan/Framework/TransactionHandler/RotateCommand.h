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

#include <Core/Maths/Transform.h>

class LocalRotateCommand : public TransactionCommand
{
public:
    LocalRotateCommand( Transform* transformToEdit, const glm::quat& rotationValue )
        : transform( transformToEdit )
        , rotate( rotationValue )
        , rotateBackup( 1, 0, 0, 0 )
    {
        rotateBackup = transform->getLocalRotation();

        actionInfos = "Apply Local Rotation";
    }

    virtual void execute() override
    {
        transform->setLocalRotation( rotate );
    }

    virtual void undo() override
    {
        transform->setLocalRotation( rotateBackup );
    }

private:
    Transform*  transform;
    glm::quat   rotate;
    glm::quat   rotateBackup;
};

class WorldRotateCommand : public TransactionCommand
{
public:
    WorldRotateCommand( Transform* transformToEdit, const glm::quat& rotationValue )
        : transform( transformToEdit )
        , rotate( rotationValue )
        , rotateBackup( 1, 0, 0, 0 )
    {
        rotateBackup = transform->getWorldRotation();

        actionInfos = "Apply World Rotation";
    }

    virtual void execute() override
    {
        transform->setWorldRotation( rotate );
    }

    virtual void undo() override
    {
        transform->setWorldRotation( rotateBackup );
    }

private:
    Transform*  transform;
    glm::quat   rotate;
    glm::quat   rotateBackup;
};
