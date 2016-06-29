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

#include <osg/ContextData>
#include <osg/FrameStamp>
#include <OpenThreads/ReentrantMutex>
#include <algorithm>

using namespace osg;


typedef std::map<unsigned int, osg::ref_ptr<ContextData> >  ContextIDMap;
static ContextIDMap s_contextIDMap;
static OpenThreads::ReentrantMutex s_contextIDMapMutex;
static ContextData::GraphicsContexts s_registeredContexts;

ContextData::ContextData(unsigned int contextID):
    GraphicsObjectManager("ContextData", contextID),
    _numContexts(0)
{
}

ContextData::~ContextData()
{
}

void ContextData::newFrame(osg::FrameStamp* frameStamp)
{
    // OSG_NOTICE<<"ContextData::newFrame("<<frameStamp->getFrameNumber()<<")"<<std::endl;
    for(ManagerMap::iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->newFrame(frameStamp);
    }
}

void ContextData::resetStats()
{
    for(ManagerMap::iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->resetStats();
    }
}

void ContextData::reportStats(std::ostream& out)
{
    for(ManagerMap::iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->reportStats(out);
    }
}

void ContextData::recomputeStats(std::ostream& out) const
{
    for(ManagerMap::const_iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->recomputeStats(out);
    }
}


void ContextData::flushDeletedGLObjects(double currentTime, double& availableTime)
{
    // OSG_NOTICE<<"ContextData::flushDeletedGLObjects("<<currentTime<<","<<availableTime<<")"<<std::endl;
    for(ManagerMap::iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->flushDeletedGLObjects(currentTime, availableTime);
    }
}

void ContextData::flushAllDeletedGLObjects()
{
    // OSG_NOTICE<<"ContextData::flushAllDeletedGLObjects()"<<std::endl;
    for(ManagerMap::iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->flushAllDeletedGLObjects();
    }
}

void ContextData::deleteAllGLObjects()
{
    // OSG_NOTICE<<"ContextData::deleteAllGLObjects()"<<std::endl;
    for(ManagerMap::iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->deleteAllGLObjects();
    }
}

void ContextData::discardAllGLObjects()
{
    // OSG_NOTICE<<"ContextData::discardAllGLObjects()"<<std::endl;
    for(ManagerMap::iterator itr = _managerMap.begin();
        itr != _managerMap.end();
        ++itr)
    {
        osg::GraphicsObjectManager* gom = dynamic_cast<osg::GraphicsObjectManager*>(itr->second.get());
        if (gom) gom->discardAllGLObjects();
    }
}

ContextData* osg::getContextData(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    ContextIDMap::iterator itr = s_contextIDMap.find(contextID);
    return (itr!=s_contextIDMap.end()) ? itr->second.get() : 0;
}

/** Get or create the ContextData for a specific contextID.*/
ContextData* osg::getOrCreateContextData(unsigned int contextID)
{
    //OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    osg::ref_ptr<ContextData>& cd = s_contextIDMap[contextID];
    if (!cd)
    {
        cd = new ContextData(contextID);
    }
    return cd.get();
}

unsigned int ContextData::createNewContextID()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    // first check to see if we can reuse contextID;
    for(ContextIDMap::iterator itr = s_contextIDMap.begin();
        itr != s_contextIDMap.end();
        ++itr)
    {
        if (!itr->second || itr->second->getNumContexts() == 0)
        {
            itr->second = new ContextData(itr->first);
            itr->second->setNumContexts(1);

            OSG_INFO<<"ContextData::createNewContextID() : reusing contextID="<<itr->first<<std::endl;

            return itr->first;
        }
    }

    unsigned int contextID = s_contextIDMap.size();
    s_contextIDMap[contextID] = new ContextData(contextID);
    s_contextIDMap[contextID]->setNumContexts(1);

    OSG_INFO<<"ContextData::createNewContextID() creating contextID="<<contextID<<std::endl;
    OSG_INFO<<"Updating the MaxNumberOfGraphicsContexts to "<<contextID+1<<std::endl;

    // update the maximum number of graphics contexts,
    // to ensure that texture objects and display buffers are configured to the correct size.
    osg::DisplaySettings::instance()->setMaxNumberOfGraphicsContexts( contextID + 1 );

    return contextID;
}

unsigned int ContextData::getMaxContextID()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    unsigned int maxContextID = 0;
    for(ContextIDMap::iterator itr = s_contextIDMap.begin();
        itr != s_contextIDMap.end();
        ++itr)
    {
        if (itr->first > maxContextID) maxContextID = itr->first;
    }
    return maxContextID;
}


void ContextData::incrementContextIDUsageCount(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    if (!s_contextIDMap[contextID]) s_contextIDMap[contextID] = new ContextData(contextID);

    s_contextIDMap[contextID]->incrementUsageCount();

    OSG_NOTICE<<"ContextData::incrementContextIDUsageCount("<<contextID<<") to "<<s_contextIDMap[contextID]->getNumContexts()<<std::endl;
}

void ContextData::decrementContextIDUsageCount(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    if (s_contextIDMap[contextID].valid())
    {
        if (s_contextIDMap[contextID]->getNumContexts()!=0)
        {
            s_contextIDMap[contextID]->decrementUsageCount();
        }

        if (s_contextIDMap[contextID]->getNumContexts()==0)
        {
            s_contextIDMap[contextID] = 0;
        }
    }
}


void ContextData::registerGraphicsContext(GraphicsContext* gc)
{
    OSG_INFO<<"ContextData::registerGraphicsContext "<<gc<<std::endl;

    if (!gc) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    GraphicsContexts::iterator itr = std::find(s_registeredContexts.begin(), s_registeredContexts.end(), gc);
    if (itr != s_registeredContexts.end()) s_registeredContexts.erase(itr);

    s_registeredContexts.push_back(gc);
}

void ContextData::unregisterGraphicsContext(GraphicsContext* gc)
{
    OSG_INFO<<"ContextData::unregisterGraphicsContext "<<gc<<std::endl;

    if (!gc) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);

    GraphicsContexts::iterator itr = std::find(s_registeredContexts.begin(), s_registeredContexts.end(), gc);
    if (itr != s_registeredContexts.end()) s_registeredContexts.erase(itr);
}

ContextData::GraphicsContexts ContextData::getAllRegisteredGraphicsContexts()
{
    OSG_INFO<<"ContextData::getAllRegisteredGraphicsContexts s_registeredContexts.size()="<<s_registeredContexts.size()<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    return s_registeredContexts;
}

ContextData::GraphicsContexts ContextData::getRegisteredGraphicsContexts(unsigned int contextID)
{
    GraphicsContexts contexts;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    for(GraphicsContexts::iterator itr = s_registeredContexts.begin();
        itr != s_registeredContexts.end();
        ++itr)
    {
        GraphicsContext* gc = *itr;
        if (gc->getState() && gc->getState()->getContextID()==contextID) contexts.push_back(gc);
    }

    OSG_INFO<<"ContextData::getRegisteredGraphicsContexts "<<contextID<<" contexts.size()="<<contexts.size()<<std::endl;

    return contexts;
}

GraphicsContext* ContextData::getOrCreateCompileContext(unsigned int contextID)
{
    OSG_NOTICE<<"ContextData::createCompileContext."<<std::endl;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
        if (s_contextIDMap[contextID]->getCompileContext()) return s_contextIDMap[contextID]->getCompileContext();
    }

    GraphicsContexts contexts = ContextData::getRegisteredGraphicsContexts(contextID);
    if (contexts.empty()) return 0;

    GraphicsContext* src_gc = contexts.front();
    const osg::GraphicsContext::Traits* src_traits = src_gc->getTraits();

    osg::GraphicsContext::Traits* traits = new osg::GraphicsContext::Traits;
    traits->screenNum = src_traits->screenNum;
    traits->displayNum = src_traits->displayNum;
    traits->hostName = src_traits->hostName;
    traits->width = 100;
    traits->height = 100;
    traits->red = src_traits->red;
    traits->green = src_traits->green;
    traits->blue = src_traits->blue;
    traits->alpha = src_traits->alpha;
    traits->depth = src_traits->depth;
    traits->sharedContext = src_gc;
    traits->pbuffer = true;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);
    if (gc.valid() && gc->realize())
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
        s_contextIDMap[contextID]->setCompileContext(gc.get());
        OSG_NOTICE<<"   succeeded ContextData::createCompileContext."<<std::endl;
        return gc.release();
    }
    else
    {
        return 0;
    }

}

void ContextData::setCompileContext(unsigned int contextID, GraphicsContext* gc)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    s_contextIDMap[contextID]->setCompileContext(gc);
}

GraphicsContext* ContextData::getCompileContext(unsigned int contextID)
{
    // OSG_NOTICE<<"ContextData::getCompileContext "<<contextID<<std::endl;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_contextIDMapMutex);
    ContextIDMap::iterator itr = s_contextIDMap.find(contextID);
    if (itr != s_contextIDMap.end()) return itr->second->getCompileContext();
    else return 0;
}
