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

#include <osgViewer/ViewerBase>

#include <osg/io_utils>

#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/TexMat>

#include <osgUtil/Optimizer>
#include <osgUtil/IntersectionVisitor>

using namespace osgViewer;

ViewerBase::ViewerBase():
    osg::Object(true)
{
    _done = false;
    _keyEventSetsDone = osgGA::GUIEventAdapter::KEY_Escape;
    _quitEventSetsDone = true;
    _threadingModel = AutomaticSelection;
    _threadsRunning = false;

}

ViewerBase::ViewerBase(const ViewerBase& base):
    osg::Object(true)
{
}

void ViewerBase::addUpdateOperation(osg::Operation* operation)
{
    if (!operation) return;

    if (!_updateOperations) _updateOperations = new osg::OperationQueue;
    
    _updateOperations->add(operation);
}

void ViewerBase::removeUpdateOperation(osg::Operation* operation)
{
    if (!operation) return;

    if (_updateOperations.valid())
    {
        _updateOperations->remove(operation);
    } 
}
