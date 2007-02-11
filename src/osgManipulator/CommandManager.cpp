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
    dragger.setCommandManager(this);

    // Check to see if the selection is already associated with the dragger.
    if (_draggerSelectionMap.count(&dragger) > 0)
    {
        std::pair<DraggerSelectionMap::iterator,DraggerSelectionMap::iterator> s;
        s = _draggerSelectionMap.equal_range(&dragger);

        for (DraggerSelectionMap::iterator iter = s.first; iter != s.second; ++iter)
        {
            if (iter->second == &selection)
                return false;
        }            
    }

    // Associate selection with dragger
    _draggerSelectionMap.insert(DraggerSelectionMap::value_type(&dragger,&selection));

    return true;
}

bool CommandManager::connect(Dragger& dragger, Constraint& constraint)
{
    dragger.setCommandManager(this);

    // Check to see if the selection is already associated with the dragger.
    if (_draggerConstraintMap.count(&dragger) > 0)
    {
        std::pair<DraggerConstraintMap::iterator,DraggerConstraintMap::iterator> s;
        s = _draggerConstraintMap.equal_range(&dragger);

        for (DraggerConstraintMap::iterator iter = s.first; iter != s.second; ++iter)
        {
            if (iter->second == &constraint)
                return false;
        }            
    }

    // Associate selection with dragger
    _draggerConstraintMap.insert(DraggerConstraintMap::value_type(&dragger,&constraint));

    return true;
}

bool CommandManager::disconnect(Dragger& dragger)
{
    _draggerSelectionMap.erase(&dragger);
    _draggerConstraintMap.erase(&dragger);
    return true;
}

void CommandManager::dispatch(MotionCommand& command)
{
    command.execute();
}

void CommandManager::addSelectionsToCommand(MotionCommand& command, Dragger& dragger)
{
    // Apply constraints to the command.
    if (_draggerConstraintMap.count(&dragger) > 0)
    {
        // Get all the selections assoicated with this dragger.
        std::pair<DraggerConstraintMap::iterator,DraggerConstraintMap::iterator> s;
        s = _draggerConstraintMap.equal_range(&dragger);

        for (DraggerConstraintMap::iterator iter = s.first; iter != s.second; ++iter)
        {
            // Add the selection to the command.
            if (iter->second.valid())
            {
                command.applyConstraint(iter->second.get());
            }
        }
    }
    
    // Add the dragger to the selection list first.
    command.addSelection(&dragger);
    
    // Add the remaining selections.
    if (_draggerSelectionMap.count(&dragger) > 0)
    {
        // Get all the selections assoicated with this dragger.
        std::pair<DraggerSelectionMap::iterator,DraggerSelectionMap::iterator> s;
        s = _draggerSelectionMap.equal_range(&dragger);

        for (DraggerSelectionMap::iterator iter = s.first; iter != s.second; ++iter)
        {
            // Add the selection to the command.
            if (iter->second.valid())
            {
                command.addSelection(iter->second.get());
            }
        }
    }
}
