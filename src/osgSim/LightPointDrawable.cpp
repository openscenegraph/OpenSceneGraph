//C++ header - Open Scene Graph Simulation - Copyright (C) 1998-2002 Robert Osfield
// Distributed under the terms of the GNU General Public License (GPL)
// as published by the Free Software Foundation.
//
// All software using osgSim must be GPL'd or excempted via the 
// purchase of the Open Scene Graph Professional License (OSGPL)
// for further information contact robert@openscenegraph.com.

#include "LightPointDrawable.h"

#include <osg/Point>

using namespace osgSim;

LightPointDrawable::LightPointDrawable():
    osg::Drawable(),
    _referenceTime(0.0),
    _referenceTimeInterval(0.0)
{
    setSupportsDisplayList(false);
    
    _depthOff = new osg::Depth;
    _depthOff->setWriteMask(false);

    _depthOn = new osg::Depth;
    _depthOn->setWriteMask(true);

    _blendOne = new osg::BlendFunc;
    _blendOne->setFunction(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE);
    
    _blendOneMinusSrcAlpha = new osg::BlendFunc;
    _blendOneMinusSrcAlpha->setFunction(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);

    _colorMaskOff = new osg::ColorMask;
    _colorMaskOff->setMask(false,false,false,false);

    _point = new osg::Point;
}

LightPointDrawable::LightPointDrawable(const LightPointDrawable& lpd,const osg::CopyOp& copyop):
    osg::Drawable(lpd,copyop),
    _referenceTime(lpd._referenceTime),
    _referenceTimeInterval(lpd._referenceTimeInterval),
    _sizedOpaqueLightPointList(lpd._sizedOpaqueLightPointList),
    _sizedAdditiveLightPointList(lpd._sizedAdditiveLightPointList),
    _sizedBlendedLightPointList(lpd._sizedBlendedLightPointList)
{
}

void LightPointDrawable::reset()
{
    SizedLightPointList::iterator itr;
    for(itr=_sizedOpaqueLightPointList.begin();
        itr!=_sizedOpaqueLightPointList.end();
        ++itr)
    {
        if (!itr->empty())
            itr->erase(itr->begin(),itr->end());
    }
    
    for(itr=_sizedAdditiveLightPointList.begin();
        itr!=_sizedAdditiveLightPointList.end();
        ++itr)
    {
        if (!itr->empty())
            itr->erase(itr->begin(),itr->end());
    }

    for(itr=_sizedBlendedLightPointList.begin();
        itr!=_sizedBlendedLightPointList.end();
        ++itr)
    {
        if (!itr->empty())
            itr->erase(itr->begin(),itr->end());
    }
}


void LightPointDrawable::drawImplementation(osg::State& state) const
{

    state.applyMode(GL_POINT_SMOOTH,true);
    state.applyMode(GL_BLEND,true);
    state.applyMode(GL_LIGHTING,false);
    state.applyTextureMode(0,GL_TEXTURE_1D,false);
    state.applyTextureMode(0,GL_TEXTURE_2D,false);
    
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);

    state.applyAttribute(_depthOn.get());

    state.applyAttribute(_blendOneMinusSrcAlpha.get());
    state.applyMode(GL_POINT_SMOOTH,true);

    SizedLightPointList::const_iterator sitr;
    unsigned int pointsize;
    for(pointsize=1,sitr=_sizedOpaqueLightPointList.begin();
        sitr!=_sizedOpaqueLightPointList.end();
        ++sitr,++pointsize)
    {

        const LightPointList& lpl = *sitr;
        if (!lpl.empty())
        {
            glPointSize(pointsize);
            glInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            //state.setInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            glDrawArrays(GL_POINTS,0,lpl.size());
        }
    }

    state.applyMode(GL_BLEND,true);
    state.applyAttribute(_depthOff.get());


    state.applyAttribute(_blendOneMinusSrcAlpha.get());

    for(pointsize=1,sitr=_sizedBlendedLightPointList.begin();
        sitr!=_sizedBlendedLightPointList.end();
        ++sitr,++pointsize)
    {

        const LightPointList& lpl = *sitr;
        if (!lpl.empty())
        {
            glPointSize(pointsize);
            glInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            //state.setInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            glDrawArrays(GL_POINTS,0,lpl.size());
        }
    }


    state.applyAttribute(_blendOne.get());    

    for(pointsize=1,sitr=_sizedAdditiveLightPointList.begin();
        sitr!=_sizedAdditiveLightPointList.end();
        ++sitr,++pointsize)
    {

        const LightPointList& lpl = *sitr;
        if (!lpl.empty())
        {
            //state.applyMode(GL_POINT_SMOOTH,pointsize!=1);
            glPointSize(pointsize);
            glInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            //state.setInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            glDrawArrays(GL_POINTS,0,lpl.size());
        }
    }

    glPointSize(1);
    
    glHint(GL_POINT_SMOOTH_HINT,GL_FASTEST);

    state.haveAppliedAttribute(osg::StateAttribute::POINT);
    
    // restore the state afterwards.
    state.apply();

}

bool LightPointDrawable::computeBound() const
{
    _bbox.init();

    SizedLightPointList::const_iterator sitr;
    for(sitr=_sizedOpaqueLightPointList.begin();
        sitr!=_sizedOpaqueLightPointList.end();
        ++sitr)
    {
        const LightPointList& lpl = *sitr;
        for(LightPointList::const_iterator litr=lpl.begin();
            litr!=lpl.end();
            ++litr)
        {
            _bbox.expandBy(litr->second);
        }
    }
    for(sitr=_sizedAdditiveLightPointList.begin();
        sitr!=_sizedAdditiveLightPointList.end();
        ++sitr)
    {
        const LightPointList& lpl = *sitr;
        for(LightPointList::const_iterator litr=lpl.begin();
            litr!=lpl.end();
            ++litr)
        {
            _bbox.expandBy(litr->second);
        }
    }
    for(sitr=_sizedBlendedLightPointList.begin();
        sitr!=_sizedBlendedLightPointList.end();
        ++sitr)
    {
        const LightPointList& lpl = *sitr;
        for(LightPointList::const_iterator litr=lpl.begin();
            litr!=lpl.end();
            ++litr)
        {
            _bbox.expandBy(litr->second);
        }
    }

    return true;
}
