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

#include <osgSim/LightPointNode>

#include "LightPointDrawable.h"

#include <osg/Timer>
#include <osg/BoundingBox>
#include <osg/BlendFunc>
#include <osg/Material>

#include <osgUtil/CullVisitor>

#include <typeinfo>

using namespace osgSim;

LightPointNode::LightPointNode():
    _minPixelSize(0.0f),
    _maxPixelSize(30.0f),
    _maxVisableDistance2(FLT_MAX)
{
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
LightPointNode::LightPointNode(const LightPointNode& lpn,const osg::CopyOp& copyop):
    osg::Node(lpn,copyop),
    _lightPointList(lpn._lightPointList),
    _minPixelSize(lpn._minPixelSize),
    _maxPixelSize(lpn._maxPixelSize),
    _maxVisableDistance2(lpn._maxVisableDistance2)    
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

bool LightPointNode::computeBound() const
{
    _bsphere.init();
    _bbox.init();


    LightPointList::const_iterator itr;
    for(itr=_lightPointList.begin();
        itr!=_lightPointList.end();
        ++itr)
    {
        _bbox.expandBy(itr->_position);
    }


    _bsphere.set(_bbox.center(),0.0f);
    
    for(itr=_lightPointList.begin();
        itr!=_lightPointList.end();
        ++itr)
    {
        osg::Vec3 dv(itr->_position-_bsphere.center());
        float radius = dv.length()+itr->_radius;
        if (_bsphere.radius()<radius) _bsphere.radius()=radius;
    }

    _bsphere.radius()+=1.0f;

    _bsphere_computed=true;
    return true;
}


void LightPointNode::traverse(osg::NodeVisitor& nv)
{
    //#define USE_TIMER
    #ifdef USE_TIMER
    osg::Timer timer;
    osg::Timer_t t1=0,t2=0,t3=0,t4=0,t5=0,t6=0,t7=0,t8=0;
    #endif

    
#ifdef USE_TIMER
    t1 = timer.tick();
#endif

    osgUtil::CullVisitor* cv = NULL;
    if (typeid(nv)==typeid(osgUtil::CullVisitor))
    {
        cv = static_cast<osgUtil::CullVisitor*>(&nv);
    }
    
#ifdef USE_TIMER
    t2 = timer.tick();
#endif
    

    // should we disabled small feature culling here?
    if (cv /*&& !cv->isCulled(_bbox)*/)
    {
    
        osg::Matrix matrix = cv->getModelViewMatrix();
        osg::RefMatrix& projection = cv->getProjectionMatrix();
        osgUtil::RenderGraph* rg = cv->getCurrentRenderGraph();

        if (rg->leaves_empty())
        {
            // this is first leaf to be added to RenderGraph
            // and therefore should not already know to current render bin,
            // so need to add it.
            cv->getCurrentRenderBin()->addRenderGraph(rg);
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
            else
            {
                // will need to replace UserData.
                osg::notify(osg::WARN) << "Warning: Replacing osgUtil::RenderGraph::_userData to support osgSim::LightPointNode, may have undefined results."<<std::endl;
            }
        }

        if (!drawable)
        {
            // set it for the frst time.
            drawable = new LightPointDrawable;
            rg->setUserData(drawable);
            
            if (cv->getFrameStamp())
            {
                drawable->setReferenceTime(cv->getFrameStamp()->getReferenceTime());
            }
        }

        // search for a drawable in the RenderLead list equal to the attached the one attached to RenderGraph user data
        // as this will be our special light point drawable.
        osgUtil::RenderGraph::LeafList::iterator litr;
        for(litr = rg->_leaves.begin();
            litr != rg->_leaves.end() && (*litr)->_drawable!=drawable;
            ++litr)
        {}
        
        if (litr == rg->_leaves.end())
        {
            // havn't found the drawable added in the RenderLeaf list, there this my be the 
            // first time through LightPointNode in this frame, so need to add drawable into the RenderGraph RenderLeaf list
            // and update its time signatures.

            drawable->reset();
            rg->addLeaf(new osgUtil::RenderLeaf(drawable,&projection,NULL,FLT_MAX));

            // need to update the drawable's frame count.
            if (cv->getFrameStamp())
            {
                drawable->updateReferenceTime(cv->getFrameStamp()->getReferenceTime());
            }

        }

#ifdef USE_TIMER
        t4 = timer.tick();
#endif


#ifdef USE_TIMER
        t7 = timer.tick();
#endif

        cv->updateCalculatedNearFar(matrix,_bbox);
        
        
        const float minimumIntensity = 1.0f/256.0f;
        const osg::Vec3 eyePoint = cv->getEyeLocal();
        
        double time=drawable->getReferenceTime();
        double timeInterval=drawable->getReferenceTimeInterval();
        
        const osg::Polytope clipvol(cv->getModelViewCullingStack().back()->getFrustum());
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
            
            float intensity = lp._intensity;
            
            // slip light point if it is intensity is 0.0 or negative.
            if (intensity<=minimumIntensity) continue;


            if (_maxVisableDistance2!=FLT_MAX)
            {
                if (dv.length2()>_maxVisableDistance2) continue;
            }


            osg::Vec4 color = lp._color;

            // check the sector.
            if (lp._sector.valid())
            {
                intensity *= (*lp._sector)(dv);
            
                // slip light point if it is intensity is 0.0 or negative.
                if (intensity<=minimumIntensity) continue;
                
            }
            
            // temporary accounting of intensity.
            //color *= intensity;

            // check the blink sequence.
            if (lp._blinkSequence.valid())
            {
                osg::Vec4 bs = lp._blinkSequence->color(time,timeInterval);
                color[0] *= bs[0];
                color[1] *= bs[1];
                color[2] *= bs[2];
                color[3] *= bs[3];
            }
            
            // if alpha value is less than the min intentsive then skip
            if (color[3]<=minimumIntensity) continue;

            float pixelSize = cv->pixelSize(position,lp._radius);

//            cout << "pixelsize = "<<pixelSize<<endl;
            
            // adjust pixel size to account for intensity.
            if (intensity!=1.0) pixelSize *= sqrt(intensity);

            // round up to the minimum pixel size if required.
            if (pixelSize<_minPixelSize) pixelSize = _minPixelSize;
            
            osg::Vec3 xpos(position*matrix);

            if (lp._blendingMode==LightPoint::BLENDED)
            {
                if (pixelSize<1.0f)
                {
                    // need to use alpha blending...
                    //color[3] = pixelSize;
                    color[3] *= osg::square(pixelSize);

                    if (color[3]<=minimumIntensity) continue;

                    drawable->addBlendedLightPoint(0, xpos,color);
                }
                else if (pixelSize<_maxPixelSize)
                {

                    unsigned int lowerBoundPixelSize = (unsigned int)pixelSize;
                    //float remainder = pixelSize-(float)lowerBoundPixelSize;
                    float remainder = osg::square(pixelSize-(float)lowerBoundPixelSize);
                    float alpha = color[3];
                    drawable->addBlendedLightPoint(lowerBoundPixelSize-1, xpos,color);

                    color[3] = alpha*remainder;
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
                    //color[3] = pixelSize;
                    color[3] *= osg::square(pixelSize);

                    if (color[3]<=minimumIntensity) continue;

                    drawable->addAdditiveLightPoint(0, xpos,color);
                }
                else if (pixelSize<_maxPixelSize)
                {

                    unsigned int lowerBoundPixelSize = (unsigned int)pixelSize;
                    //float remainder = pixelSize-(float)lowerBoundPixelSize;
                    float remainder = osg::square(pixelSize-(float)lowerBoundPixelSize);
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


