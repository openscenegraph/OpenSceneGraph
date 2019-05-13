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

#include <osgSim/LightPointNode>
#include <osgSim/LightPointSystem>

#include "LightPointDrawable.h"
#include "LightPointSpriteDrawable.h"

#include <osg/Timer>
#include <osg/BoundingBox>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/PointSprite>

#include <osgUtil/CullVisitor>

#include <typeinfo>

namespace osgSim
{

osg::StateSet* getSingletonLightPointSystemSet()
{
    static osg::ref_ptr<osg::StateSet> s_stateset = 0;
    if (!s_stateset)
    {
        s_stateset = new osg::StateSet;
        // force light point nodes to be drawn after everything else by picking a rendering bin number after
        // the transparent bin.
        s_stateset->setRenderBinDetails(20,"DepthSortedBin");
    }
    return s_stateset.get();
}


LightPointNode::LightPointNode():
    _minPixelSize(0.0f),
    _maxPixelSize(30.0f),
    _maxVisibleDistance2(FLT_MAX),
    _lightSystem(0),
    _pointSprites(false)
{
    setStateSet(getSingletonLightPointSystemSet());
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
LightPointNode::LightPointNode(const LightPointNode& lpn,const osg::CopyOp& copyop):
    osg::Node(lpn,copyop),
    _bbox(lpn._bbox),
    _lightPointList(lpn._lightPointList),
    _minPixelSize(lpn._minPixelSize),
    _maxPixelSize(lpn._maxPixelSize),
    _maxVisibleDistance2(lpn._maxVisibleDistance2),
    _lightSystem(lpn._lightSystem),
    _pointSprites(lpn._pointSprites)
{
}

unsigned int LightPointNode::addLightPoint(const LightPoint& lp)
{
    unsigned int num = _lightPointList.size();
    _lightPointList.push_back(lp);
    dirtyBound();
    return num;
}

void LightPointNode::removeLightPoint(unsigned int pos)
{
    if (pos<_lightPointList.size())
    {
        _lightPointList.erase(_lightPointList.begin()+pos);
        dirtyBound();
    }
    dirtyBound();
}

osg::BoundingSphere LightPointNode::computeBound() const
{
    osg::BoundingSphere bsphere;
    bsphere.init();
    _bbox.init();

    if (_lightPointList.empty())
    {
        return bsphere;
    }


    LightPointList::const_iterator itr;
    for(itr=_lightPointList.begin();
        itr!=_lightPointList.end();
        ++itr)
    {
        _bbox.expandBy(itr->_position);
    }


    bsphere.set(_bbox.center(),0.0f);

    for(itr=_lightPointList.begin();
        itr!=_lightPointList.end();
        ++itr)
    {
        osg::Vec3 dv(itr->_position-bsphere.center());
        float radius = dv.length()+itr->_radius;
        if (bsphere.radius()<radius) bsphere.radius()=radius;
    }

    bsphere.radius()+=1.0f;
    return bsphere;
}


void LightPointNode::traverse(osg::NodeVisitor& nv)
{
    if (_lightPointList.empty())
    {
        // no light points so no op.
        return;
    }

    //#define USE_TIMER
    #ifdef USE_TIMER
    osg::Timer timer;
    osg::Timer_t t1=0,t2=0,t3=0,t4=0,t5=0,t6=0,t7=0,t8=0;
    #endif


#ifdef USE_TIMER
    t1 = timer.tick();
#endif

    osgUtil::CullVisitor* cv = nv.asCullVisitor();

#ifdef USE_TIMER
    t2 = timer.tick();
#endif


    // should we disable small feature culling here?
    if (cv /*&& !cv->isCulled(_bbox)*/)
    {

        osg::Matrix matrix = *(cv->getModelViewMatrix());
        osg::RefMatrix& projection = *(cv->getProjectionMatrix());
        osgUtil::StateGraph* rg = cv->getCurrentStateGraph();

        if (rg->leaves_empty())
        {
            // this is first leaf to be added to StateGraph
            // and therefore should not already know current render bin,
            // so need to add it.
            cv->getCurrentRenderBin()->addStateGraph(rg);
        }

#ifdef USE_TIMER
        t3 = timer.tick();
#endif


        LightPointDrawable* drawable = NULL;
        osg::Referenced* object = rg->getUserData();
        if (object)
        {
            if (typeid(*object)==typeid(LightPointDrawable))
            {
                // resuse the user data attached to the render graph.
                drawable = static_cast<LightPointDrawable*>(object);

            }
            else if (typeid(*object)==typeid(LightPointSpriteDrawable))
            {
                drawable = static_cast<LightPointSpriteDrawable*>(object);
            }
            else
            {
                // will need to replace UserData.
                OSG_WARN << "Warning: Replacing osgUtil::StateGraph::_userData to support osgSim::LightPointNode, may have undefined results."<<std::endl;
            }
        }

        if (!drawable)
        {
            drawable = _pointSprites ? new LightPointSpriteDrawable : new LightPointDrawable;
            rg->setUserData(drawable);

            if (cv->getFrameStamp())
            {
                drawable->setSimulationTime(cv->getFrameStamp()->getSimulationTime());
            }
        }

        // search for a drawable in the RenderLeaf list equal to the attached the one attached to StateGraph user data
        // as this will be our special light point drawable.
        osgUtil::StateGraph::LeafList::iterator litr;
        for(litr = rg->_leaves.begin();
            litr != rg->_leaves.end() && (*litr)->getDrawable()!=drawable;
            ++litr)
        {}

        if (litr == rg->_leaves.end())
        {
            // haven't found the drawable added in the RenderLeaf list, therefore this may be the
            // first time through LightPointNode in this frame, so need to add drawable into the StateGraph RenderLeaf list
            // and update its time signatures.

            drawable->reset();
            rg->addLeaf(new osgUtil::RenderLeaf(drawable,&projection,NULL,FLT_MAX));

            // need to update the drawable's frame count.
            if (cv->getFrameStamp())
            {
                drawable->updateSimulationTime(cv->getFrameStamp()->getSimulationTime());
            }

        }

#ifdef USE_TIMER
        t4 = timer.tick();
#endif


#ifdef USE_TIMER
        t7 = timer.tick();
#endif


        if (cv->getComputeNearFarMode() != osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR)
            cv->updateCalculatedNearFar(matrix,_bbox);


        const float minimumIntensity = 1.0f/256.0f;
        const osg::Vec3 eyePoint = cv->getEyeLocal();

        double time=drawable->getSimulationTime();
        double timeInterval=drawable->getSimulationTimeInterval();

        const osg::Polytope clipvol(cv->getCurrentCullingSet().getFrustum());
        const bool computeClipping = false;//(clipvol.getCurrentMask()!=0);

        //LightPointDrawable::ColorPosition cp;
        for(LightPointList::iterator itr=_lightPointList.begin();
            itr!=_lightPointList.end();
            ++itr)
        {
            const LightPoint& lp = *itr;

            if (!lp._on) continue;

            const osg::Vec3& position = lp._position;

            // skip light point if it is not contianed in the view frustum.
            if (computeClipping && !clipvol.contains(position)) continue;

            // delta vector between eyepoint and light point.
            osg::Vec3 dv(eyePoint-position);

            float intensity = (_lightSystem.valid()) ? _lightSystem->getIntensity() : lp._intensity;

            // slip light point if its intensity is 0.0 or negative.
            if (intensity<=minimumIntensity) continue;

            // (SIB) Clip on distance, if close to limit, add transparency
            float distanceFactor = 1.0f;
            if (_maxVisibleDistance2!=FLT_MAX)
            {
                if (dv.length2()>_maxVisibleDistance2) continue;
                else if (_maxVisibleDistance2 > 0)
                    distanceFactor = 1.0f - osg::square(dv.length2() / _maxVisibleDistance2);
            }

            osg::Vec4 color = lp._color;

            // check the sector.
            if (lp._sector.valid())
            {
                intensity *= (*lp._sector)(dv);

                // skip light point if it is intensity is 0.0 or negative.
                if (intensity<=minimumIntensity) continue;

            }

            // temporary accounting of intensity.
            //color *= intensity;

            // check the blink sequence.
            bool doBlink = lp._blinkSequence.valid();
            if (doBlink && _lightSystem.valid())
                doBlink = (_lightSystem->getAnimationState() == LightPointSystem::ANIMATION_ON);

            if (doBlink)
            {
                osg::Vec4 bs = lp._blinkSequence->color(time,timeInterval);
                color[0] *= bs[0];
                color[1] *= bs[1];
                color[2] *= bs[2];
                color[3] *= bs[3];
            }

            // if alpha value is less than the min intentsity then skip
            if (color[3]<=minimumIntensity) continue;

            float pixelSize = cv->pixelSize(position,lp._radius);

            //            cout << "pixelsize = "<<pixelSize<<endl;

            // adjust pixel size to account for intensity.
            if (intensity!=1.0) pixelSize *= sqrt(intensity);

            // adjust alpha to account for max range (Fade on distance)
            color[3] *= distanceFactor;

            // round up to the minimum pixel size if required.
            float orgPixelSize = pixelSize;
            if (pixelSize<_minPixelSize) pixelSize = _minPixelSize;

            osg::Vec3 xpos(position*matrix);

            if (lp._blendingMode==LightPoint::BLENDED)
            {
                if (pixelSize<1.0f)
                {
                    // need to use alpha blending...
                    color[3] *= pixelSize;
                    // color[3] *= osg::square(pixelSize);

                    if (color[3]<=minimumIntensity) continue;

                    drawable->addBlendedLightPoint(0, xpos,color);
                }
                else if (pixelSize<_maxPixelSize)
                {

                    unsigned int lowerBoundPixelSize = (unsigned int)pixelSize;
                    float remainder = osg::square(pixelSize-(float)lowerBoundPixelSize);

                    // (SIB) Add transparency if pixel is clamped to minpixelsize
                    if (orgPixelSize<_minPixelSize)
                        color[3] *= (2.0/3.0) + (1.0/3.0) * sqrt(orgPixelSize / pixelSize);

                    drawable->addBlendedLightPoint(lowerBoundPixelSize-1, xpos,color);
                    color[3] *= remainder;
                    drawable->addBlendedLightPoint(lowerBoundPixelSize, xpos,color);
                }
                else // use a billboard geometry.
                {
                    drawable->addBlendedLightPoint((unsigned int)(_maxPixelSize-1.0), xpos,color);
                }
            }
            else // ADDITIVE blending.
            {
                if (pixelSize<1.0f)
                {
                    // need to use alpha blending...
                    color[3] *= pixelSize;
                    // color[3] *= osg::square(pixelSize);

                    if (color[3]<=minimumIntensity) continue;

                    drawable->addAdditiveLightPoint(0, xpos,color);
                }
                else if (pixelSize<_maxPixelSize)
                {

                    unsigned int lowerBoundPixelSize = (unsigned int)pixelSize;
                    float remainder = osg::square(pixelSize-(float)lowerBoundPixelSize);

                    // (SIB) Add transparency if pixel is clamped to minpixelsize
                    if (orgPixelSize<_minPixelSize)
                        color[3] *= (2.0/3.0) + (1.0/3.0) * sqrt(orgPixelSize / pixelSize);

                    float alpha = color[3];
                    color[3] = alpha*(1.0f-remainder);
                    drawable->addAdditiveLightPoint(lowerBoundPixelSize-1, xpos,color);
                    color[3] = alpha*remainder;
                    drawable->addAdditiveLightPoint(lowerBoundPixelSize, xpos,color);
                }
                else // use a billboard geometry.
                {
                    drawable->addAdditiveLightPoint((unsigned int)(_maxPixelSize-1.0), xpos,color);
                }
            }
        }

#ifdef USE_TIMER
        t8 = timer.tick();
#endif

    }
#ifdef USE_TIMER
    cout << "compute"<<endl;
    cout << "  t2-t1="<<t2-t1<<endl;
    cout << "  t4-t3="<<t4-t3<<endl;
    cout << "  t6-t5="<<t6-t5<<endl;
    cout << "  t8-t7="<<t8-t7<<endl;
    cout << "_lightPointList.size()="<<_lightPointList.size()<<endl;
    cout << "  t8-t7/size = "<<(float)(t8-t7)/(float)_lightPointList.size()<<endl;
#endif
}


} // end of namespace
