/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/Impostor>

#include <algorithm>

using namespace osg;

Impostor::Impostor()
{
    _impostorThreshold = -1.0f;
}


ImpostorSprite* Impostor::findBestImpostorSprite(unsigned int contextID, const osg::Vec3& currLocalEyePoint) const
{
    ImpostorSpriteList& impostorSpriteList = _impostorSpriteListBuffer[contextID];

    float min_distance2 = FLT_MAX;
    ImpostorSprite* impostorSprite = NULL;
    for(ImpostorSpriteList::iterator itr=impostorSpriteList.begin();
        itr!=impostorSpriteList.end();
        ++itr)
    {
        float distance2 = (currLocalEyePoint-(*itr)->getStoredLocalEyePoint()).length2();
        if (distance2<min_distance2)
        {
            min_distance2 = distance2;
            impostorSprite = itr->get();
        }
    }
    return impostorSprite;
}

void Impostor::addImpostorSprite(unsigned int contextID, ImpostorSprite* is)
{
    if (is && is->getParent()!=this)
    {
        ImpostorSpriteList& impostorSpriteList = _impostorSpriteListBuffer[contextID];

        // add it to my impostor list first, so it remains referenced
        // when its reference in the previous_owner is removed.
        impostorSpriteList.push_back(is);

        if (is->getParent())
        {
            Impostor* previous_owner = is->getParent();
            ImpostorSpriteList& isl = previous_owner->_impostorSpriteListBuffer[contextID];
            
            // find and erase reference to is.
            for(ImpostorSpriteList::iterator itr=isl.begin();
                itr!=isl.end();
                ++itr)
            {
                if ((*itr)==is)
                {
                    isl.erase(itr);
                    break;
                }
            }
        }
        is->setParent(this);
        
    }
}

bool Impostor::computeBound() const
{
    return LOD::computeBound();
}
