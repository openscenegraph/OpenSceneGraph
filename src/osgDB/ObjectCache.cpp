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

#include <osg/Texture>
#include <osgDB/ObjectCache>
#include <osgDB/Options>

using namespace osgDB;

bool ObjectCache::ClassComp::operator() (const ObjectCache::FileNameOptionsPair& lhs, const ObjectCache::FileNameOptionsPair& rhs) const
{
    // check if filename are the same
    if (lhs.first < rhs.first) return true;
    if (rhs.first < lhs.first) return false;

    // check if Options pointers are the same.
    if (lhs.second == rhs.second) return false;

    // need to compare Options pointers
    if (lhs.second.valid() && rhs.second.valid())
    {
        // lhs & rhs have valid Options objects
        return *lhs.second < *rhs.second;
    }

    // finally use pointer comparison, expecting at least one will be NULL pointer
    return lhs.second < rhs.second;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// ObjectCache
//
ObjectCache::ObjectCache():
    osg::Referenced(true)
{
//    OSG_NOTICE<<"Constructed ObjectCache"<<std::endl;
}

ObjectCache::~ObjectCache()
{
//    OSG_NOTICE<<"Destructed ObjectCache"<<std::endl;
}

void ObjectCache::addObjectCache(ObjectCache* objectCache)
{
    // don't allow a cache to be added to itself.
    if (objectCache==this) return;

    // lock both ObjectCache to prevent their contents from being modified by other threads while we merge.
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock1(_objectCacheMutex);
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock2(objectCache->_objectCacheMutex);

    OSG_DEBUG<<"Inserting objects to main ObjectCache "<<objectCache->_objectCache.size()<<std::endl;

    _objectCache.insert(objectCache->_objectCache.begin(), objectCache->_objectCache.end());
}


void ObjectCache::addEntryToObjectCache(const std::string& filename, osg::Object* object, double timestamp, const Options *options)
{
    if (!object) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);
    _objectCache[FileNameOptionsPair(filename, options ? osg::clone(options) : 0)] = ObjectTimeStampPair(object,timestamp);
    OSG_DEBUG<<"Adding "<<filename<<" with options '"<<(options ? options->getOptionString() : "")<<"' to ObjectCache "<<this<<std::endl;
}

ObjectCache::ObjectCacheMap::iterator ObjectCache::find(const std::string& fileName, const osgDB::Options* options)
{
    for(ObjectCacheMap::iterator itr = _objectCache.begin();
        itr != _objectCache.end();
        ++itr)
    {
        if (itr->first.first==fileName)
        {
            if (itr->first.second.valid())
            {
                if (options && *(itr->first.second)==*options) return itr;
            }
            else if (!options) return itr;
        }
    }
    return _objectCache.end();
}


osg::Object* ObjectCache::getFromObjectCache(const std::string& fileName, const Options *options)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);
    ObjectCacheMap::iterator itr = find(fileName, options);
    if (itr!=_objectCache.end())
    {
        osg::ref_ptr<const osgDB::Options> o = itr->first.second;
        if (o.valid())
        {
            OSG_DEBUG<<"Found "<<fileName<<" with options '"<< o->getOptionString()<< "' in ObjectCache "<<this<<std::endl;
        }
        else
        {
            OSG_DEBUG<<"Found "<<fileName<<" in ObjectCache "<<this<<std::endl;
        }
        return itr->second.first.get();
    }
    else return 0;
}

osg::ref_ptr<osg::Object> ObjectCache::getRefFromObjectCache(const std::string& fileName, const Options *options)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);
    ObjectCacheMap::iterator itr = find(fileName, options);
    if (itr!=_objectCache.end())
    {
        osg::ref_ptr<const osgDB::Options> o = itr->first.second;
        if (o.valid())
        {
            OSG_DEBUG<<"Found "<<fileName<<" with options '"<< o->getOptionString()<< "' in ObjectCache "<<this<<std::endl;
        }
        else
        {
            OSG_DEBUG<<"Found "<<fileName<<" in ObjectCache "<<this<<std::endl;
        }
        return itr->second.first.get();
    }
    else return 0;
}

void ObjectCache::updateTimeStampOfObjectsInCacheWithExternalReferences(double referenceTime)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);

    // look for objects with external references and update their time stamp.
    for(ObjectCacheMap::iterator itr=_objectCache.begin();
        itr!=_objectCache.end();
        ++itr)
    {
        // if ref count is greater the 1 the object has an external reference.
        if (itr->second.first->referenceCount()>1)
        {
            // so update it time stamp.
            itr->second.second = referenceTime;
        }
    }
}

void ObjectCache::removeExpiredObjectsInCache(double expiryTime)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);

    // Remove expired entries from object cache
    ObjectCacheMap::iterator oitr = _objectCache.begin();
    while(oitr != _objectCache.end())
    {
        if (oitr->second.second<=expiryTime)
        {
#if __cplusplus > 199711L
            oitr = _objectCache.erase(oitr);
#else
            _objectCache.erase(oitr++);
#endif
        }
        else
        {
            ++oitr;
        }
    }
}

void ObjectCache::removeFromObjectCache(const std::string& fileName, const Options *options)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);
    ObjectCacheMap::iterator itr = find(fileName, options);
    if (itr!=_objectCache.end()) _objectCache.erase(itr);
}

void ObjectCache::clear()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);
    _objectCache.clear();
}

namespace ObjectCacheUtils
{

struct ContainsUnreffedTextures : public osg::NodeVisitor
{
    ContainsUnreffedTextures() :
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        foundUnreffedTexture(false)
    {}

    bool foundUnreffedTexture;

    bool check(const osg::Texture* texture)
    {
        if (!texture) return false;

        unsigned int numImages = 0;
        for(unsigned int i=0; i<texture->getNumImages(); ++i)
        {
            if (texture->getImage(i)) ++numImages;
        }

        return numImages==0;
    }

    bool check(const osg::StateSet* stateset)
    {
        for(unsigned int i=0; i<stateset->getNumTextureAttributeLists(); ++i)
        {
            const osg::StateAttribute* sa = stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE);
            if (sa && check(sa->asTexture())) return true;
        }
        return false;
    }

    bool check(osg::Object* object)
    {
        if (object->asStateAttribute()) return check(dynamic_cast<const osg::Texture*>(object));
        if (object->asStateSet()) return check(object->asStateSet());
        if (!object->asNode()) return false;

        foundUnreffedTexture = false;

        object->asNode()->accept(*this);

        return foundUnreffedTexture;
    }

    void apply(osg::Node& node)
    {
        if (node.getStateSet())
        {
            if (check(node.getStateSet()))
            {
                foundUnreffedTexture = true;
                return;
            }
        }

        traverse(node);
    }
};

} // ObjectCacheUtils

void ObjectCache::releaseGLObjects(osg::State* state)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_objectCacheMutex);

    ObjectCacheUtils::ContainsUnreffedTextures cut;

    for(ObjectCacheMap::iterator itr = _objectCache.begin();
        itr != _objectCache.end();
        )
    {
        ObjectCacheMap::iterator curr_itr = itr;

        // get object and advance iterator to next item
        osg::Object* object = itr->second.first.get();

        bool needToRemoveEntry = cut.check(itr->second.first.get());

        object->releaseGLObjects(state);

        ++itr;

        if (needToRemoveEntry)
        {
            _objectCache.erase(curr_itr);
        }
    }
}
