/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY
{
}
 without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgViewer/Viewer>

using namespace osgViewer;

Viewer::Viewer()
{
}

Viewer::~Viewer()
{
}

void Viewer::realize()
{
    osg::notify(osg::NOTICE)<<"Viewer::realize()"<<std::endl;

    if (_camera.valid() && _camera->getGraphicsContext())
    {
        _camera->getGraphicsContext()->realize();
    }
    
    for(unsigned int i=0; i<getNumSlaves(); ++i)
    {
        Slave& slave = getSlave(i);
        if (slave._camera.valid() && slave._camera->getGraphicsContext())
        {
            osg::notify(osg::NOTICE)<<"  slave realize()"<<std::endl;
            slave._camera->getGraphicsContext()->realize();
        }
    }
}


void Viewer::frame()
{
    osg::notify(osg::NOTICE)<<"Viewer::frame() not implemented yet."<<std::endl;
}

void Viewer::frameAdvance()
{
    osg::notify(osg::NOTICE)<<"Viewer::frameAdvance() not implemented yet."<<std::endl;
}

void Viewer::frameEventTraversal()
{
    osg::notify(osg::NOTICE)<<"Viewer::frameEventTraversal() not implemented yet."<<std::endl;
}

void Viewer::frameUpdateTraversal()
{
    osg::notify(osg::NOTICE)<<"Viewer::frameUpdateTraversal() not implemented yet."<<std::endl;
}

void Viewer::frameCullTraversal()
{
    osg::notify(osg::NOTICE)<<"Viewer::frameCullTraversal() not implemented yet."<<std::endl;
}

void Viewer::frameDrawTraversal()
{
    osg::notify(osg::NOTICE)<<"Viewer::frameDrawTraversal() not implemented yet."<<std::endl;
}

void Viewer::releaseAllGLObjects()
{
    osg::notify(osg::NOTICE)<<"Viewer::releaseAllGLObjects() not implemented yet."<<std::endl;
}

void Viewer::cleanup()
{
    osg::notify(osg::NOTICE)<<"Viewer::cleanup() not implemented yet."<<std::endl;
}

