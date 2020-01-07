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

#ifndef SHOWEVENTHANDLER
#define SHOWEVENTHANDLER 1

#include <osg/StateSet>
#include <osg/Point>

#include <osgGA/GUIEventHandler>

namespace p3d
{

class ShowEventHandler : public osgGA::GUIEventHandler
{
    public:

        ShowEventHandler();

        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv);

        virtual void getUsage(osg::ApplicationUsage& usage) const;

};

}

#endif
