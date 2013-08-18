/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield 
 *
 * This software is open source and may be redistributed and/or modified under  
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * include LICENSE.txt for more details.
*/

#include "ShowEventHandler.h"

#include <osgViewer/Viewer>
#include <osg/Notify>

using namespace p3d;

ShowEventHandler::ShowEventHandler()
{
}

bool ShowEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& /*aa*/, osg::Object* object, osg::NodeVisitor* /*nv*/)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            osg::notify(osg::INFO)<<"ShowEventHandler KEYUP "<<(int)ea.getKey()<<std::endl;
            if (ea.getKey()>=osgGA::GUIEventAdapter::KEY_F1 && 
                ea.getKey()<=osgGA::GUIEventAdapter::KEY_F8)
            {
                unsigned int child = ea.getKey()-osgGA::GUIEventAdapter::KEY_F1;
                osg::notify(osg::INFO)<<"   Select "<<child<<std::endl;
                osg::Switch* showSwitch = dynamic_cast<osg::Switch*>(object);
                if (showSwitch) 
                {
                    if (child<showSwitch->getNumChildren())
                    {
                        osg::notify(osg::INFO)<<"   Switched "<<child<<std::endl;
                        showSwitch->setSingleChildOn(child);
                        return true;
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return false;
}


void ShowEventHandler::accept(osgGA::GUIEventHandlerVisitor& v)
{
    v.visit(*this);
}

void ShowEventHandler::getUsage(osg::ApplicationUsage& /*usage*/) const
{
}

