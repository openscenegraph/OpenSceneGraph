/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 * 
 * Cedric Pinson <mornifle@plopbyte.net>
 *
 */

#include "AnimtkViewerKeyHandler"

AnimtkKeyEventHandler::AnimtkKeyEventHandler()
{
    _actionKeys[List] = 'l';
    _actionKeys[Help] = 'h';
    _actionKeys[Play] = 'p';
    _actionKeys[Next] = ']';
    _actionKeys[Prev] = '[';
}

void AnimtkKeyEventHandler::printUsage() const 
{
    std::cout << (char) _actionKeys.find(Help)->second << " for Help" << std::endl;
    std::cout << (char) _actionKeys.find(List)->second << " for List" << std::endl;
    std::cout << (char) _actionKeys.find(Play)->second << " for Play" << std::endl;
    std::cout << (char) _actionKeys.find(Next)->second << " for select Next item" << std::endl;
    std::cout << (char) _actionKeys.find(Prev)->second << " for select Previous item" << std::endl;
}


bool AnimtkKeyEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&,
                                   osg::Object*, osg::NodeVisitor*)
{
    AnimtkViewerModelController& mc = AnimtkViewerModelController::instance();
    if(ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) 
    {
        if (ea.getKey() == _actionKeys[List]) return mc.list();
        else if (ea.getKey() == _actionKeys[Play]) return mc.play();
        else if (ea.getKey() == _actionKeys[Next]) return mc.next();
        else if (ea.getKey() == _actionKeys[Prev]) return mc.previous();
        else if (ea.getKey() == _actionKeys[Help]) 
        {
            printUsage();
            return true;
        }
    }

    return false;
}
