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

SharedStateManager::SharedStateManager():
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) 
{
    shareMode = SHARE_TEXTURES;
    //shareMode = SHARE_STATESETS;
    mutex=0;
}

//----------------------------------------------------------------
// SharedStateManager::prune
//----------------------------------------------------------------
void SharedStateManager::prune()
{
    StateSetSet::iterator sitr, sitr1;
    for(sitr=_sharedStateSetList.begin(); sitr!=_sharedStateSetList.end(); sitr=sitr1)
    {
        sitr1=sitr; ++sitr1;
        if((*sitr)->referenceCount()<=1) _sharedStateSetList.erase(sitr);
    }

    TextureSet::iterator titr, titr1;
    for(titr=_sharedTextureList.begin(); titr!=_sharedTextureList.end(); titr=titr1)
    {
        titr1=titr; ++titr1;
        if((*titr)->referenceCount()<=1) _sharedTextureList.erase(titr);
    }

} 


//----------------------------------------------------------------
// SharedStateManager::share
//----------------------------------------------------------------
void SharedStateManager::share(osg::Node *node, OpenThreads::Mutex *mt)
{
//    const osg::Timer& timer = *osg::Timer::instance();
//    osg::Timer_t start_tick = timer.tick();
    
    mutex = mt;
    apply(*node);
    tmpSharedTextureList.clear();
    tmpSharedStateSetList.clear();
    mutex = 0;

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
 

//----------------------------------------------------------------
// SharedStateManager::find
//----------------------------------------------------------------
osg::StateSet *SharedStateManager::find(osg::StateSet *ss)
{
    for(StateSetSet::iterator itr=_sharedStateSetList.begin(); 
        itr!=_sharedStateSetList.end(); 
        ++itr)
    {
        if(ss->compare(*(itr->get()),true)==0) 
            return (osg::StateSet *)itr->get();
    }
    return NULL;
}
osg::StateAttribute *SharedStateManager::find(osg::StateAttribute *sa)
{
    for(TextureSet::iterator itr=_sharedTextureList.begin(); 
    itr!=_sharedTextureList.end(); 
    ++itr)
    {
        if(sa->compare(*(itr->get()))==0) 
            return (osg::StateAttribute *)itr->get();
    }
    return NULL;
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
    osg::StateSet::TextureAttributeList& texAttributes = ss->getTextureAttributeList();
    for(unsigned int unit=0;unit<texAttributes.size();++unit)
    {
        osg::StateAttribute *texture = ss->getTextureAttribute(unit, osg::StateAttribute::TEXTURE);

        // Valid Texture to be shared
        if(texture && texture->getDataVariance()==osg::Object::STATIC)
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
                    if(mutex) mutex->lock();
                    ss->setTextureAttributeAndModes(unit, textureFromSharedList, osg::StateAttribute::ON);
                    if(mutex) mutex->unlock();
                    tmpSharedTextureList[texture] = TextureSharePair(textureFromSharedList, true);
                }
                else
                {
                    // Texture is not in _sharedAttributeList: 
                    // Add to _sharedAttributeList. Not needed to be shared all next times.
                    _sharedTextureList.insert(texture); 
                    tmpSharedTextureList[texture] = TextureSharePair(texture, false);            
                }
            }
            else if(titr->second.second)
            {
                // Texture is in tmpSharedAttributeList and share flag is on:
                // It should be shared
                if(mutex) mutex->lock();
                ss->setTextureAttributeAndModes(unit, titr->second.first, osg::StateAttribute::ON);
                if(mutex) mutex->unlock();
            }
        }
    }
}


//----------------------------------------------------------------
// SharedStateManager::process
//----------------------------------------------------------------
void SharedStateManager::process(osg::StateSet* ss, osg::Object* parent)
{
    if(shareMode & SHARE_STATESETS)
    {
        // Valid StateSet to be shared
        if(ss->getDataVariance()==osg::Object::STATIC)
        {
            StateSetStateSetSharePairMap::iterator sitr = tmpSharedStateSetList.find(ss);
            if(sitr==tmpSharedStateSetList.end())
            {
                // StateSet is not in tmp list: 
                // First time it appears in this file, search StateSet in sharedObjectList
                osg::StateSet *ssFromSharedList = find(ss);
                if(ssFromSharedList)
                {
                    // StateSet is in sharedStateSetList: 
                    // Share now. Required to be shared all next times
                    if(mutex) mutex->lock();
                    setStateSet(ssFromSharedList, parent);
                    if(mutex) mutex->unlock();
                    tmpSharedStateSetList[ss] = StateSetSharePair(ssFromSharedList, true);
                }
                else
                {
                    // StateSet is not in sharedStateSetList: 
                    // Add to sharedStateSetList. Not needed to be shared all next times.
                    _sharedStateSetList.insert(ss); 
                    tmpSharedStateSetList[ss] = StateSetSharePair(ss, false);            

                    // Only in this case sharing textures is also required
                    if(shareMode & SHARE_TEXTURES)
                    {
                        shareTextures(ss);
                    }
                }
            }
            else if(sitr->second.second)
            {
                // StateSet is in tmpSharedStateSetList and share flag is on:
                // It should be shared
                if(mutex) mutex->lock();
                setStateSet(sitr->second.first, parent);
                if(mutex) mutex->unlock();
            }
        }
    }

    else if(shareMode & SHARE_TEXTURES)
    {
        shareTextures(ss);
    }
}
