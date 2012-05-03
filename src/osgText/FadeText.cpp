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

struct FadeTextData : public osg::Referenced
{
    FadeTextData(FadeText* fadeText=0):
        _fadeText(fadeText),
        _visible(true) {}

    bool operator < (const FadeTextData& rhs) const
    {
        return _fadeText < rhs._fadeText;
    }

    double getNearestZ() const
    {
        double nearestZ = _vertices[0].z();
        if (nearestZ < _vertices[1].z()) nearestZ = _vertices[1].z();
        if (nearestZ < _vertices[2].z()) nearestZ = _vertices[2].z();
        if (nearestZ < _vertices[3].z()) nearestZ = _vertices[3].z();

        // OSG_NOTICE<<"getNearestZ()="<<_fadeText->getText().createUTF8EncodedString()<<" "<<nearestZ<<std::endl;

        return nearestZ;
    }

    FadeText*   _fadeText;
    osg::Vec3d   _vertices[4];
    bool        _visible;
};

struct FadeTextPolytopeData : public FadeTextData, public osg::Polytope
{
    FadeTextPolytopeData(FadeTextData& fadeTextData):
        FadeTextData(fadeTextData)
    {
        _referenceVertexList.push_back(_vertices[0]);
        _referenceVertexList.push_back(_vertices[1]);
        _referenceVertexList.push_back(_vertices[2]);
        _referenceVertexList.push_back(_vertices[3]);
    }

    void addEdgePlane(const osg::Vec3& corner, const osg::Vec3& edge)
    {
        osg::Vec3 normal( edge.y(), -edge.x(), 0.0f);
        normal.normalize();

        add(osg::Plane(normal, corner));
    }

    void buildPolytope()
    {
        osg::Vec3d edge01 = _vertices[1] - _vertices[0];
        osg::Vec3d edge12 = _vertices[2] - _vertices[1];

        osg::Vec3d normalFrontFace = edge01 ^ edge12;
        bool needToFlip = normalFrontFace.z()>0.0f;

        normalFrontFace.normalize();
        add(osg::Plane(normalFrontFace, _vertices[0]));

        add(osg::Plane( osg::Vec3d(0.0f,0.0f,0.0f), _vertices[0], _vertices[1]));
        add(osg::Plane( osg::Vec3d(0.0f,0.0f,0.0f), _vertices[1], _vertices[2]));
        add(osg::Plane( osg::Vec3d(0.0f,0.0f,0.0f), _vertices[2], _vertices[3]));
        add(osg::Plane( osg::Vec3d(0.0f,0.0f,0.0f), _vertices[3], _vertices[0]));

#if 0
        OSG_NOTICE<<" normalFrontFace = "<<normalFrontFace<<std::endl;
        OSG_NOTICE<<" edge01 = "<<edge01<<std::endl;
        OSG_NOTICE<<" edge12 = "<<edge12<<std::endl;
        OSG_NOTICE<<" _vertices[0]= "<<_vertices[0]<<std::endl;
        OSG_NOTICE<<" _vertices[1]= "<<_vertices[1]<<std::endl;
        OSG_NOTICE<<" _vertices[2]= "<<_vertices[2]<<std::endl;
        OSG_NOTICE<<" _vertices[3]= "<<_vertices[3]<<std::endl;
#endif

        if (needToFlip) flip();

#if 0
        OSG_NOTICE<<"   plane 0 "<< _planeList[0]<<std::endl;
        OSG_NOTICE<<"   plane 1 "<< _planeList[1]<<std::endl;
        OSG_NOTICE<<"   plane 2 "<< _planeList[2]<<std::endl;
        OSG_NOTICE<<"   plane 3 "<< _planeList[3]<<std::endl;
        OSG_NOTICE<<"   plane 4 "<< _planeList[4]<<std::endl;
#endif

    }

    inline bool contains(const std::vector<osg::Vec3>& vertices)
    {
        for(std::vector<osg::Vec3>::const_iterator itr = vertices.begin();
            itr != vertices.end();
            ++itr)
        {
            if (osg::Polytope::contains(*itr))
            {
                return true;
            }
        }
        return false;
    }

};

struct FadeTextUserData : public osg::Referenced
{
    FadeTextUserData():
        _frameNumber(0) {}

    typedef std::list<FadeTextData> FadeTextList;
    unsigned int _frameNumber;
    FadeTextList _fadeTextInView;
};

struct GlobalFadeText : public osg::Referenced
{
    typedef std::set< osg::ref_ptr<FadeTextUserData> > UserDataSet;
    typedef std::set<FadeText*> FadeTextSet;
    typedef std::multimap<double, osg::ref_ptr<FadeTextPolytopeData> > FadeTextPolytopeMap;
    typedef std::map<osg::View*, UserDataSet> ViewUserDataMap;
    typedef std::map<osg::View*, FadeTextSet > ViewFadeTextMap;

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
            OSG_NOTICE<<"Memory error, unable to create FadeTextUserData."<<std::endl;
            return 0;
        }

        _viewMap[view].insert(userData);

        return userData;
    }


    void update(unsigned int frameNumber)
    {
        _frameNumber = frameNumber;

        for(GlobalFadeText::ViewUserDataMap::iterator vitr = _viewMap.begin();
            vitr != _viewMap.end();
            ++vitr)
        {

            osg::View* view = vitr->first;

            FadeTextSet& fadeTextSet = _viewFadeTextMap[view];
            fadeTextSet.clear();

            FadeTextPolytopeMap fadeTextPolytopeMap;

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
                        FadeTextData& fadeTextData = *fitr;
                        if (fadeTextSet.count(fadeTextData._fadeText)==0)
                        {
                            fadeTextSet.insert(fadeTextData._fadeText);
                            fadeTextPolytopeMap.insert(FadeTextPolytopeMap::value_type(
                                -fadeTextData.getNearestZ(), new FadeTextPolytopeData(fadeTextData)));
                        }
                    }
                }
            }

            // for each FadeTexPoltopeData
            //    create polytopes
            //    test against all FTPD's later in the list
            //       test all control points on FTPD against each plane of the current polytope
            //       if all control points removed or outside then discard FTPD and make FT visible = false;

            FadeTextPolytopeMap::iterator outer_itr = fadeTextPolytopeMap.begin();
            while (outer_itr != fadeTextPolytopeMap.end())
            {
                FadeTextPolytopeMap::iterator inner_itr = outer_itr;
                ++inner_itr;

                if (inner_itr == fadeTextPolytopeMap.end()) break;

                FadeTextPolytopeData& outer_ftpm = *(outer_itr->second);
                outer_ftpm.buildPolytope();

                // OSG_NOTICE<<"Outer z "<<outer_ftpm.getNearestZ()<<std::endl;

                while(inner_itr != fadeTextPolytopeMap.end())
                {
                    FadeTextPolytopeData& inner_ftpm = *(inner_itr->second);

                    // OSG_NOTICE<<"Inner z "<<inner_ftpm.getNearestZ()<<std::endl;

                    if (outer_ftpm.contains(inner_ftpm.getReferenceVertexList()))
                    {
                        FadeTextPolytopeMap::iterator erase_itr = inner_itr;
                        // move to next ftpm
                        ++inner_itr;

                        fadeTextSet.erase(inner_ftpm._fadeText);

                        // need to remove inner_ftpm as its occluded.
                        fadeTextPolytopeMap.erase(erase_itr);

                    }
                    else
                    {
                        // move to next ftpm
                        ++inner_itr;
                    }
                }

                ++outer_itr;

            }
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

struct FadeText::FadeTextUpdateCallback : public osg::Drawable::UpdateCallback
{
    FadeTextData _ftd;

    virtual void update(osg::NodeVisitor* nv, osg::Drawable* drawable)
    {
        osgText::FadeText* fadeText = dynamic_cast<osgText::FadeText*>(drawable);
        if (!fadeText) return;

        unsigned int frameNumber = nv->getFrameStamp()->getFrameNumber();

        GlobalFadeText* gft = getGlobalFadeText();
        gft->updateIfRequired(frameNumber);

        osgText::FadeText::ViewBlendColourMap& vbcm = fadeText->getViewBlendColourMap();

        _ftd._fadeText = fadeText;

        float fadeSpeed = fadeText->getFadeSpeed();

        GlobalFadeText::ViewFadeTextMap& vftm = gft->_viewFadeTextMap;
        for(GlobalFadeText::ViewFadeTextMap::iterator itr = vftm.begin();
            itr != vftm.end();
            ++itr)
        {
            osg::View* view = itr->first;
            GlobalFadeText::FadeTextSet& fadeTextSet = itr->second;
            bool visible = fadeTextSet.count(fadeText)!=0;

            osg::Vec4& tec = vbcm[view];
            tec[0] = 1.0f;
            tec[1] = 1.0f;
            tec[2] = 1.0f;
            if (visible)
            {
                if (tec[3]<1.0f)
                {
                    tec[3] += fadeSpeed;
                    if (tec[3]>1.0f) tec[3] = 1.0f;
                }

            }
            else
            {
                if (tec[3]>0.0f)
                {
                    tec[3] -= fadeSpeed;
                    if (tec[3]<0.0f) tec[3] = 0.0f;
                }
            }
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
    setDataVariance(osg::Object::DYNAMIC);

    _fadeSpeed = 0.01f;
    setUpdateCallback(new FadeTextUpdateCallback());
}



void FadeText::drawImplementation(osg::RenderInfo& renderInfo) const
{

    osg::State& state = *renderInfo.getState();

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
            OSG_NOTICE<<"Warning user data not of supported type."<<std::endl;
            return;
        }

        userData = getGlobalFadeText()->createNewFadeTextUserData(renderInfo.getView());

        if (!userData)
        {
            OSG_NOTICE<<"Memory error, unable to create FadeTextUserData."<<std::endl;
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



    osgText::Text::AutoTransformCache& atc = _autoTransformCache[renderInfo.getContextID()];

    osg::Matrix lmv = atc._matrix;
    lmv.postMult(state.getModelViewMatrix());

    if (renderInfo.getView() && renderInfo.getView()->getCamera())
    {
        // move from camera into the view space.
        lmv.postMult(state.getInitialInverseViewMatrix());
        lmv.postMult(renderInfo.getView()->getCamera()->getViewMatrix());
    }

    FadeTextData ftd(const_cast<osgText::FadeText*>(this));

    ftd._vertices[0].set(osg::Vec3d(_textBB.xMin(),_textBB.yMin(),_textBB.zMin())*lmv);
    ftd._vertices[1].set(osg::Vec3d(_textBB.xMax(),_textBB.yMin(),_textBB.zMin())*lmv);
    ftd._vertices[2].set(osg::Vec3d(_textBB.xMax(),_textBB.yMax(),_textBB.zMin())*lmv);
    ftd._vertices[3].set(osg::Vec3d(_textBB.xMin(),_textBB.yMax(),_textBB.zMin())*lmv);

    userData->_fadeTextInView.push_back(ftd);

}
