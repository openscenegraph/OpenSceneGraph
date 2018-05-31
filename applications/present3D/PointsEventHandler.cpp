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

#include "PointsEventHandler.h"

#include <osg/Notify>

PointsEventHandler::PointsEventHandler()
{
    _point = new osg::Point;
    //_point->setDistanceAttenuation(osg::Vec3(0.0,0.0000,0.05f));
}

bool PointsEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='+' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Add)
            {
               changePointSize(1.0f);
               return true;
            }
            else if (ea.getKey()=='-' || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Subtract)
            {
               changePointSize(-1.0f);
               return true;
            }
            else if (ea.getKey()=='<')
            {
               changePointAttenuation(1.1f);
               return true;
            }
            else if (ea.getKey()=='>')
            {
               changePointAttenuation(1.0f/1.1f);
               return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

void PointsEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("+","Increase point size");
    usage.addKeyboardMouseBinding("-","Reduce point size");
    usage.addKeyboardMouseBinding(">","Increase point size");
    usage.addKeyboardMouseBinding("<","Reduce point size");
}

float PointsEventHandler::getPointSize() const
{
    return _point->getSize();
}

void PointsEventHandler::setPointSize(float psize)
{
    if (psize>0.0)
    {
        _point->setSize(psize);
        _stateset->setAttribute(_point.get());
    }
    else
    {
        _stateset->setAttribute(_point.get(),osg::StateAttribute::INHERIT);
    }
    osg::notify(osg::INFO)<<"Point size "<<psize<<std::endl;
}

void PointsEventHandler::changePointSize(float delta)
{
    setPointSize(getPointSize()+delta);
}

void PointsEventHandler::changePointAttenuation(float scale)
{
    if (_point.valid())
    {
        _point->setDistanceAttenuation(_point->getDistanceAttenuation()*scale);
        if (_stateset.valid()) _stateset->setAttribute(_point.get());
    }
}
