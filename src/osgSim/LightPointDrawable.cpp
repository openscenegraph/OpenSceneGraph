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

#include "LightPointDrawable.h"

#include <osg/Point>

using namespace osgSim;

LightPointDrawable::LightPointDrawable():
    osg::Drawable(),
    _endian(osg::getCpuByteOrder()),
    _simulationTime(0.0),
    _simulationTimeInterval(0.0)
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

}

LightPointDrawable::LightPointDrawable(const LightPointDrawable& lpd,const osg::CopyOp& copyop):
    osg::Drawable(lpd,copyop),
    _simulationTime(lpd._simulationTime),
    _simulationTimeInterval(lpd._simulationTimeInterval),
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


void LightPointDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = *renderInfo.getState();

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
            //glInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            state.setInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
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
            //glInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            state.setInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
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
            //glInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            state.setInterleavedArrays(GL_C4UB_V3F,0,&lpl.front());
            glDrawArrays(GL_POINTS,0,lpl.size());
        }
    }

    glPointSize(1);
    
    glHint(GL_POINT_SMOOTH_HINT,GL_FASTEST);

    state.haveAppliedAttribute(osg::StateAttribute::POINT);
    
    state.dirtyAllVertexArrays();
    state.disableAllVertexArrays();
    
    // restore the state afterwards.
    state.apply();

}

osg::BoundingBox LightPointDrawable::computeBound() const
{
    osg::BoundingBox bbox;

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
            bbox.expandBy(litr->second);
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
            bbox.expandBy(litr->second);
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
            bbox.expandBy(litr->second);
        }
    }

    return bbox;
}
