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

#include <osg/Timer>
#include <osgDB/SharedStateManager>

using namespace osgDB;

SharedStateManager::SharedStateManager(unsigned int mode):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    setShareMode(mode);
    _mutex=0;
}

void SharedStateManager::setShareMode(unsigned int mode)
{
    _shareMode = mode;

    _shareTexture[osg::Object::DYNAMIC] =       (_shareMode & SHARE_DYNAMIC_TEXTURES)!=0;
    _shareTexture[osg::Object::STATIC] =        (_shareMode & SHARE_STATIC_TEXTURES)!=0;
    _shareTexture[osg::Object::UNSPECIFIED] =   (_shareMode & SHARE_UNSPECIFIED_TEXTURES)!=0;

    _shareStateSet[osg::Object::DYNAMIC] =      (_shareMode & SHARE_DYNAMIC_STATESETS)!=0;
    _shareStateSet[osg::Object::STATIC] =       (_shareMode & SHARE_STATIC_STATESETS)!=0;
    _shareStateSet[osg::Object::UNSPECIFIED] =  (_shareMode & SHARE_UNSPECIFIED_STATESETS)!=0;
}

//----------------------------------------------------------------
// SharedStateManager::prune
//----------------------------------------------------------------
void SharedStateManager::prune()
{
    StateSetSet::iterator sitr;
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_listMutex);
    for(sitr=_sharedStateSetList.begin(); sitr!=_sharedStateSetList.end();)
    {
        if ((*sitr)->referenceCount()<=1)
            _sharedStateSetList.erase(sitr++);
        else
            ++sitr;
    }

    TextureSet::iterator titr;
    for(titr=_sharedTextureList.begin(); titr!=_sharedTextureList.end();)
    {
        if ((*titr)->referenceCount()<=1)
            _sharedTextureList.erase(titr++);
        else
            ++titr;
    }

}


//----------------------------------------------------------------
// SharedStateManager::share
//----------------------------------------------------------------
void SharedStateManager::share(osg::Node *node, OpenThreads::Mutex *mt)
{
//    const osg::Timer& timer = *osg::Timer::instance();
//    osg::Timer_t start_tick = timer.tick();

    _mutex = mt;
    node->accept(*this);
    tmpSharedTextureList.clear();
    tmpSharedStateSetList.clear();
    _mutex = 0;

//    osg::Timer_t end_tick = timer.tick();
//     std::cout << "SHARING TIME = "<<timer.delta_m(start_tick,end_tick)<<"ms"<<std::endl;
//     std::cout << "   _sharedStateSetList.size() = "<<_sharedStateSetList.size()<<std::endl;
//     std::cout << "   _sharedTextureList.size() = "<<_sharedTextureList.size()<<std::endl;
}


//----------------------------------------------------------------
// SharedStateManager::apply
//----------------------------------------------------------------
void SharedStateManager::apply(osg::Node& node)
{
    osg::StateSet* ss = node.getStateSet();
    if(ss) process(ss, &node);
    traverse(node);
}
void SharedStateManager::apply(osg::Geode& geode)
{
    osg::StateSet* ss = geode.getStateSet();
    if(ss) process(ss, &geode);
    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {
        osg::Drawable* drawable = geode.getDrawable(i);
        if(drawable)
        {
            ss = drawable->getStateSet();
            if(ss) process(ss, drawable);
        }
    }
}

bool SharedStateManager::isShared(osg::StateSet* ss)
{
    if (shareStateSet(ss->getDataVariance()))
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_listMutex);
        return find(ss) != 0;
    }
    else
        return false;
}

bool SharedStateManager::isShared(osg::Texture* texture)
{
    if (shareTexture(texture->getDataVariance()))
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_listMutex);
        return find(texture) != 0;
    }
    else
        return false;
}

//----------------------------------------------------------------
// SharedStateManager::find
//----------------------------------------------------------------
//
// The find methods don't need to lock _listMutex because the thread
// from which they are called is doing the writing to the lists.
osg::StateSet *SharedStateManager::find(osg::StateSet *ss)
{
    StateSetSet::iterator result
        = _sharedStateSetList.find(osg::ref_ptr<osg::StateSet>(ss));
    if (result == _sharedStateSetList.end())
        return NULL;
    else
        return result->get();
}

osg::StateAttribute *SharedStateManager::find(osg::StateAttribute *sa)
{
    TextureSet::iterator result
        = _sharedTextureList.find(osg::ref_ptr<osg::StateAttribute>(sa));
    if (result == _sharedTextureList.end())
        return NULL;
    else
        return result->get();
}


//----------------------------------------------------------------
// SharedStateManager::setStateSet
//----------------------------------------------------------------
void SharedStateManager::setStateSet(osg::StateSet* ss, osg::Object* object)
{
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(object);
    if (drawable)
    {
        drawable->setStateSet(ss);
    }
    else
    {
        osg::Node* node = dynamic_cast<osg::Node*>(object);
        if (node)
        {
            node->setStateSet(ss);
        }
    }
}


//----------------------------------------------------------------
// SharedStateManager::shareTextures
//----------------------------------------------------------------
void SharedStateManager::shareTextures(osg::StateSet* ss)
{
    const osg::StateSet::TextureAttributeList& texAttributes = ss->getTextureAttributeList();
    for(unsigned int unit=0;unit<texAttributes.size();++unit)
    {
        osg::StateAttribute *texture = ss->getTextureAttribute(unit, osg::StateAttribute::TEXTURE);

        // Valid Texture to be shared
        if(texture && shareTexture(texture->getDataVariance()))
        {
            TextureTextureSharePairMap::iterator titr = tmpSharedTextureList.find(texture);
            if(titr==tmpSharedTextureList.end())
            {
                // Texture is not in tmp list:
                // First time it appears in this file, search Texture in sharedAttributeList
                osg::StateAttribute *textureFromSharedList = find(texture);
                if(textureFromSharedList)
                {
                    // Texture is in sharedAttributeList:
                    // Share now. Required to be shared all next times
                    if(_mutex) _mutex->lock();
                    ss->setTextureAttributeAndModes(unit, textureFromSharedList, osg::StateAttribute::ON);
                    if(_mutex) _mutex->unlock();
                    tmpSharedTextureList[texture] = TextureSharePair(textureFromSharedList, true);
                }
                else
                {
                    // Texture is not in _sharedAttributeList:
                    // Add to _sharedAttributeList. Not needed to be
                    // shared all next times.
                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_listMutex);
                    _sharedTextureList.insert(texture);
                    tmpSharedTextureList[texture] = TextureSharePair(texture, false);
                }
            }
            else if(titr->second.second)
            {
                // Texture is in tmpSharedAttributeList and share flag is on:
                // It should be shared
                if(_mutex) _mutex->lock();
                ss->setTextureAttributeAndModes(unit, titr->second.first, osg::StateAttribute::ON);
                if(_mutex) _mutex->unlock();
            }
        }
    }
}


//----------------------------------------------------------------
// SharedStateManager::process
//----------------------------------------------------------------
void SharedStateManager::process(osg::StateSet* ss, osg::Object* parent)
{
    // Valid StateSet to be shared
    if (shareStateSet(ss->getDataVariance()))
    {
        StateSetStateSetSharePairMap::iterator sitr = tmpSharedStateSetList.find(ss);
        if (sitr==tmpSharedStateSetList.end())
        {
            // StateSet is not in tmp list:
            // First time it appears in this file, search StateSet in sharedObjectList
            osg::StateSet *ssFromSharedList = find(ss);
            if (ssFromSharedList)
            {
                // StateSet is in sharedStateSetList:
                // Share now. Required to be shared all next times
                if (_mutex) _mutex->lock();
                setStateSet(ssFromSharedList, parent);
                if (_mutex) _mutex->unlock();
                tmpSharedStateSetList[ss] = StateSetSharePair(ssFromSharedList, true);
            }
            else
            {
                // StateSet is not in sharedStateSetList:
                // Add to sharedStateSetList. Not needed to be shared all next times.
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_listMutex);
                    _sharedStateSetList.insert(ss);
                    tmpSharedStateSetList[ss]
                        = StateSetSharePair(ss, false);
                }
                // Only in this case sharing textures is also required
                if (_shareMode & (SHARE_DYNAMIC_TEXTURES | SHARE_STATIC_TEXTURES | SHARE_UNSPECIFIED_TEXTURES))
                {
                    shareTextures(ss);
                }
            }
        }
        else if (sitr->second.second)
        {
            // StateSet is in tmpSharedStateSetList and share flag is on:
            // It should be shared
            if(_mutex) _mutex->lock();
            setStateSet(sitr->second.first, parent);
            if(_mutex) _mutex->unlock();
        }
    }
    else if (_shareMode & (SHARE_DYNAMIC_TEXTURES | SHARE_STATIC_TEXTURES | SHARE_UNSPECIFIED_TEXTURES))
    {
        shareTextures(ss);
    }
}

void SharedStateManager::releaseGLObjects(osg::State* state) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_listMutex);

    {
        TextureSet::const_iterator it;
        for ( it = _sharedTextureList.begin(); it != _sharedTextureList.end(); ++it )
        {
            if ( it->valid() )
            {
                it->get()->releaseGLObjects(state);
            }
        }
    }

    {
        StateSetSet::const_iterator it;
        for( it = _sharedStateSetList.begin(); it != _sharedStateSetList.end(); ++it )
        {
            if ( it->valid() )
            {
                it->get()->releaseGLObjects(state);
            }
        }
    }
}
