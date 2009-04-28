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
                
        virtual void accept(osgGA::GUIEventHandlerVisitor& v);
        
        virtual void getUsage(osg::ApplicationUsage& usage) const;
        
};

}

#endif
