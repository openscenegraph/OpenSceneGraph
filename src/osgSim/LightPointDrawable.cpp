//C++ header - Open Scene Graph Simulation - Copyright (C) 1998-2002 Robert Osfield
// Distributed under the terms of the GNU General Public License (GPL)
// as published by the Free Software Foundation.
//
// All software using osgSim must be GPL'd or excempted via the 
// purchase of the Open Scene Graph Professional License (OSGPL)
// for further information contact robert@openscenegraph.com.

#include <osgSim/LightPointDrawable>

#include <osg/Point>

using namespace osgSim;

LightPointDrawable::LightPointDrawable():
    Drawable(),
    _referenceTime(0.0),
    _referenceTimeInterval(0.0),
    _sizedLightPointList()
{
    setSupportsDisplayList(false);
    
    _depthOff = osgNew osg::Depth;
    _depthOff->setWriteMask(false);

    _depthOn = osgNew osg::Depth;
    _depthOn->setWriteMask(true);

    _blendOn = osgNew osg::BlendFunc;
    _blendOn->setFunction(osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE);

    _colorMaskOff = osgNew osg::ColorMask;
    _colorMaskOff->setMask(false,false,false,false);

    _point = osgNew osg::Point;
}

LightPointDrawable::LightPointDrawable(const LightPointDrawable& lpd,const osg::CopyOp& copyop):
    Drawable(lpd,copyop),
    _referenceTime(lpd._referenceTime),
    _referenceTimeInterval(lpd._referenceTimeInterval),
    _sizedLightPointList(lpd._sizedLightPointList)
{
}

void LightPointDrawable::drawImplementation(osg::State& state) const
{

    if (_sizedLightPointList.empty()) return;
    
        
    state.applyMode(GL_POINT_SMOOTH,true);
    state.applyMode(GL_BLEND,true);
    state.applyMode(GL_LIGHTING,false);
    state.applyTextureMode(0,GL_TEXTURE_1D,false);
    state.applyTextureMode(0,GL_TEXTURE_2D,false);
    

    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    
    state.applyAttribute(_blendOn.get());    
    state.applyAttribute(_depthOff.get());

    int pointsize;
    SizedLightPointList::const_iterator sitr;
    for(pointsize=1,sitr=_sizedLightPointList.begin();
        sitr!=_sizedLightPointList.end();
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

    // switch on depth mask to do set up depth mask.    
    state.applyAttribute(_depthOn.get());
//    glDepthMask(GL_TRUE);

    state.applyMode(GL_BLEND,false);
    
    state.applyAttribute(_colorMaskOff.get());
//    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

    for(pointsize=1,sitr=_sizedLightPointList.begin();
        sitr!=_sizedLightPointList.end();
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

    glPointSize(1);
    
    glHint(GL_POINT_SMOOTH_HINT,GL_FASTEST);

    state.haveAppliedAttribute(osg::StateAttribute::POINT);

}

bool LightPointDrawable::computeBound() const
{
    _bbox.init();

    for(SizedLightPointList::const_iterator sitr=_sizedLightPointList.begin();
        sitr!=_sizedLightPointList.end();
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
