#include <osg/Impostor>

#include <algorithm>

using namespace osg;

Impostor::Impostor()
{
    _impostorThreshold = -1.0f;
}


ImpostorSprite* Impostor::findBestImpostorSprite(const osg::Vec3& currLocalEyePoint)
{
    float min_distance2 = FLT_MAX;
    ImpostorSprite* impostorSprite = NULL;
    for(ImpostorSpriteList::iterator itr=_impostorSpriteList.begin();
        itr!=_impostorSpriteList.end();
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

void Impostor::addImpostorSprite(ImpostorSprite* is)
{
    if (is && is->getParent()!=this)
    {
        // add it to my impostor list first, so it remains referenced
        // when its reference in the previous_owner is removed.
        _impostorSpriteList.push_back(is);

        if (is->getParent())
        {
            Impostor* previous_owner = is->getParent();
            ImpostorSpriteList& isl = previous_owner->_impostorSpriteList;
            
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
