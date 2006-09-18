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


#include <osgText/FadeText>
#include <osg/Notify>
#include <osg/io_utils>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

using namespace osgText;

struct FadeTextUserData : public osg::Referenced
{
    FadeTextUserData():
        _frameNumber(0) {}

    typedef std::list<FadeText*> FadeTextList;
    unsigned int _frameNumber;
    FadeTextList _fadeTextInView;
};

struct GlobalFadeText : public osg::Referenced
{
    typedef std::set< osg::ref_ptr<FadeTextUserData> > UserDataSet;
    typedef std::set<FadeText*> FadeTextSet;
    typedef std::map<osg::View*, UserDataSet> ViewUserDataMap;
    typedef std::map<osg::View*, FadeTextSet> ViewFadeTextMap;

    GlobalFadeText():
        _frameNumber(0xffffffff)
    {
    }

    
    FadeTextUserData* createNewFadeTextUserData(osg::View* view)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        
        FadeTextUserData* userData = new FadeTextUserData;

        if (!userData)
        {
            osg::notify(osg::NOTICE)<<"Memory error, unable to create FadeTextUserData."<<std::endl;
            return 0;
        }

        _viewMap[view].insert(userData);
        
        return userData;
    }
    
    
    void update(unsigned int frameNumber)
    {
        _frameNumber = frameNumber;
        
        osg::notify(osg::NOTICE)<<"New frame, updating GlobalFadeText structure."<<std::endl;
        
        for(GlobalFadeText::ViewUserDataMap::iterator vitr = _viewMap.begin();
            vitr != _viewMap.end();
            ++vitr)
        {
        
            osg::View* view = vitr->first;

            FadeTextSet& fadeTextSet = _viewFadeTextMap[view];
            fadeTextSet.clear();

            for(GlobalFadeText::UserDataSet::iterator uitr = vitr->second.begin();
                uitr != vitr->second.end();
                ++uitr)
            {
                FadeTextUserData* userData = uitr->get();
                
                int frameDelta = frameNumber - userData->_frameNumber;
                if (frameDelta<=1)
                {
                    for(FadeTextUserData::FadeTextList::iterator fitr = userData->_fadeTextInView.begin();
                        fitr != userData->_fadeTextInView.end();
                        ++fitr)
                    {
                        FadeText* fadeText = *fitr;
                        fadeTextSet.insert(fadeText);
                    }
                }
            }

            osg::notify(osg::NOTICE)<<"   view="<<view<<"\tnumOfText="<<fadeTextSet.size()<<std::endl;

        }
    }
    
    inline void updateIfRequired(unsigned int frameNumber)
    {
        if (_frameNumber!=frameNumber) update(frameNumber);
    }

    unsigned int _frameNumber;
    OpenThreads::Mutex _mutex;
    ViewUserDataMap _viewMap;
    ViewFadeTextMap _viewFadeTextMap;
};

GlobalFadeText* getGlobalFadeText()
{
    static osg::ref_ptr<GlobalFadeText> s_globalFadeText = new GlobalFadeText;
    return s_globalFadeText.get();
}

struct FadeTextUpdateCallback : public osg::Drawable::UpdateCallback
{
    virtual void update(osg::NodeVisitor* nv, osg::Drawable* drawable)
    {
        osgText::FadeText* fadeText = dynamic_cast<osgText::FadeText*>(drawable);
        if (!fadeText) return;
    
        unsigned int frameNumber = nv->getFrameStamp()->getFrameNumber();

        osg::notify(osg::NOTICE)<<"Update FadeText "<<fadeText<<std::endl;
        
        typedef std::set<FadeText*> FadeTextSet;

        GlobalFadeText* gft = getGlobalFadeText();
        gft->updateIfRequired(frameNumber);
        
        osgText::FadeText::ViewBlendColourMap& vbcm = fadeText->getViewBlendColourMap();

        GlobalFadeText::ViewFadeTextMap& vftm = gft->_viewFadeTextMap;
        for(GlobalFadeText::ViewFadeTextMap::iterator itr = vftm.begin();
            itr != vftm.end();
            ++itr)
        {
            osg::View* view = itr->first;
            FadeTextSet& fadeTextSet = itr->second;
            if (fadeTextSet.count(fadeText)==0)
            {
                osg::notify(osg::NOTICE)<<"Text "<<fadeText<<" not in "<<view<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Text "<<fadeText<<" is in "<<view<<std::endl;
            }

            osg::Vec4& tec = vbcm[view];
            tec.set(1.0f,1.0f,1.0f, 0.5f + 0.5f * sin( float(frameNumber)*0.1));
        }

    }
};


FadeText::FadeText()
{
    init();
}

FadeText::FadeText(const Text& text,const osg::CopyOp& copyop):
    Text(text,copyop)
{
    init();
}

void FadeText::init()
{
    setUpdateCallback(new FadeTextUpdateCallback());
}



void FadeText::drawImplementation(osg::RenderInfo& renderInfo) const
{

    ViewBlendColourMap::iterator itr = _viewBlendColourMap.find(renderInfo.getView());
    if (itr != _viewBlendColourMap.end())
    {
        Text::drawImplementation(*renderInfo.getState(), itr->second );
    }
    else
    {
        Text::drawImplementation(*renderInfo.getState(), osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
    }
    

    // now pass on new details 

    FadeTextUserData* userData = dynamic_cast<FadeTextUserData*>(renderInfo.getUserData());
    if (!userData)
    {
        if (renderInfo.getUserData())
        {
            osg::notify(osg::NOTICE)<<"Warning user data not of supported type."<<std::endl;
            return;
        }

        userData = getGlobalFadeText()->createNewFadeTextUserData(renderInfo.getView());

        if (!userData)
        {
            osg::notify(osg::NOTICE)<<"Memory error, unable to create FadeTextUserData."<<std::endl;
            return;
        }

        renderInfo.setUserData(userData);
    }

    unsigned int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
    if (frameNumber != userData->_frameNumber)
    {
        // new frame so must reset UserData structure.
        userData->_frameNumber = frameNumber;
        userData->_fadeTextInView.clear();
    }

    userData->_fadeTextInView.push_back(const_cast<osgText::FadeText*>(this));

    osgText::Text::AutoTransformCache& atc = _autoTransformCache[renderInfo.getContextID()];
    osg::Matrix& matrix = atc._matrix;

    osg::notify(osg::NOTICE)<<"cull Matrix = "<<matrix<<std::endl;


}
