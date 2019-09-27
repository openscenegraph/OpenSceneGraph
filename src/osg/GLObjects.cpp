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
#include <osg/GLObjects>

#include <osg/VertexProgram>
#include <osg/FragmentProgram>
#include <osg/FrameBufferObject>
#include <osg/ContextData>

using namespace osg;

void osg::flushDeletedGLObjects(unsigned int contextID, double currentTime, double& availableTime)
{
    osg::getContextData(contextID)->flushDeletedGLObjects(currentTime, availableTime);
}

void osg::flushAllDeletedGLObjects(unsigned int contextID)
{
    osg::getContextData(contextID)->flushAllDeletedGLObjects();
}

void osg::deleteAllGLObjects(unsigned int contextID)
{
    osg::getContextData(contextID)->deleteAllGLObjects();
}

void osg::discardAllGLObjects(unsigned int contextID)
{
    osg::getContextData(contextID)->discardAllGLObjects();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GraphicsObject
//
GraphicsObject::GraphicsObject()
{
//    OSG_NOTICE<<"GraphicsObject::GraphicsObject() "<<this<<std::endl;
}

GraphicsObject::~GraphicsObject()
{
//    OSG_NOTICE<<"GraphicsObject::~GraphicsObject() "<<this<<std::endl;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GraphicsObjectManager
//
GraphicsObjectManager::GraphicsObjectManager(const std::string& name, unsigned int contextID):
    _name(name),
    _contextID(contextID)
{
}

GraphicsObjectManager::~GraphicsObjectManager()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GLObjectManager
//
GLObjectManager::GLObjectManager(const std::string& name, unsigned int contextID):
    GraphicsObjectManager(name, contextID)
{
}

GLObjectManager::~GLObjectManager()
{
}

void GLObjectManager::flushDeletedGLObjects(double, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(GLObjectHandleList::iterator itr = _deleteGLObjectHandles.begin();
        itr != _deleteGLObjectHandles.end() && elapsedTime<availableTime;
        )
    {
        deleteGLObject( *itr );
        itr = _deleteGLObjectHandles.erase( itr );
        elapsedTime = timer.delta_s(start_tick,timer.tick());
    }

    availableTime -= elapsedTime;
}

void GLObjectManager::flushAllDeletedGLObjects()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    for(GLObjectHandleList::iterator itr = _deleteGLObjectHandles.begin();
        itr != _deleteGLObjectHandles.end();
        ++itr)
    {
        deleteGLObject( *itr );
    }
    _deleteGLObjectHandles.clear();
}

void GLObjectManager::deleteAllGLObjects()
{
    OSG_INFO<<"void "<<_name<<"::deleteAllGLObjects() : Not Implemented"<<std::endl;
}

void GLObjectManager::discardAllGLObjects()
{
   // OSG_NOTICE<<"void "<<_name<<"::discardAllGLObjects()"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    _deleteGLObjectHandles.clear();
}

void GLObjectManager::scheduleGLObjectForDeletion(GLuint globj)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    // add glProgram to the cache for the appropriate context.
    _deleteGLObjectHandles.push_back(globj);
}


GLuint GLObjectManager::createGLObject()
{
    OSG_INFO<<"void "<<_name<<"::createGLObject() : Not Implemented"<<std::endl;
    return 0;
}
