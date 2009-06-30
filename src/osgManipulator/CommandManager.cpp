/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/CommandManager>
#include <osgManipulator/Command>

using namespace osgManipulator;

CommandManager::CommandManager()
{
}

CommandManager::~CommandManager()
{
}

bool CommandManager::connect(Dragger& dragger, Selection& selection)
{
    dragger.addSelection(&selection);

    return true;
}

bool CommandManager::connect(Dragger& dragger, Constraint& constraint)
{
    dragger.addConstraint(&constraint);

    return true;
}

bool CommandManager::disconnect(Dragger& dragger)
{
    dragger.getConstraints().clear();
    dragger.getSelections().clear();

    return true;
}

void CommandManager::dispatch(MotionCommand& command)
{
    command.execute();
}

void CommandManager::addSelectionsToCommand(MotionCommand& command, Dragger& dragger)
{
    for(Dragger::Constraints::iterator itr = dragger.getConstraints().begin();
        itr != dragger.getConstraints().end();
        ++itr)
    {
        command.applyConstraint(itr->get());
    }

    // Add the dragger to the selection list first.
    command.addSelection(&dragger);

    for(Dragger::Selections::iterator itr = dragger.getSelections().begin();
        itr != dragger.getSelections().end();
        ++itr)
    {
        command.addSelection(*itr);
    }
}


CommandManager::Selections CommandManager::getConnectedSelections(Dragger& dragger)
{
    Selections selections;

    for(Dragger::Selections::iterator itr = dragger.getSelections().begin();
        itr != dragger.getSelections().end();
        ++itr)
    {
        selections.push_back(*itr);
    }

    return selections;
}
