/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRA;NTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/View>
#include <osg/Notify>

using namespace osg;

View::View()
{
}

View::~View()
{
    // detatch the cameras from this View to prevent dangling pointers
    for(Slaves::iterator itr = _slaves.begin();
        itr != _slaves.end();
        ++itr)
    {
        Slave& cd = *itr;
        cd._camera->setView(0);
        cd._camera->setCullCallback(0);
    }
}


void View::updateSlaves()
{
    if (!_camera) return;
    
    for(Slaves::iterator itr = _slaves.begin();
        itr != _slaves.end();
        ++itr)
    {
        Slave& cd = *itr;
        cd._camera->setProjectionMatrix(cd._projectionOffset * _camera->getProjectionMatrix());
        cd._camera->setViewMatrix(cd._viewOffset * _camera->getViewMatrix());
        cd._camera->inheritCullSettings(*_camera);
    }


}

bool View::addSlave(osg::Camera* camera, const osg::Matrix& projectionOffset, const osg::Matrix& viewOffset)
{
    if (!camera) return false;

    camera->setView(this);
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    if (_camera.valid())
    {
        camera->setProjectionMatrix(projectionOffset * _camera->getProjectionMatrix());
        camera->setViewMatrix(viewOffset * _camera->getViewMatrix());
        camera->inheritCullSettings(*_camera);
    }
           
    _slaves.push_back(Slave(camera, projectionOffset, viewOffset));

    // osg::notify(osg::NOTICE)<<"Added camera"<<std::endl;

    return true;
}

bool View::removeSlave(unsigned int pos)
{
    if (pos >= _slaves.size()) return false;

    _slaves[pos]._camera->setView(0);
    _slaves[pos]._camera->setCullCallback(0);
    
    _slaves.erase(_slaves.begin()+pos);
    
    osg::notify(osg::NOTICE)<<"Removed camera"<<std::endl;

    return true;
}


