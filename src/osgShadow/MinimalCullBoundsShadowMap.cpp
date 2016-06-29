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
 *
 * ViewDependentShadow codes Copyright (C) 2008 Wojciech Lewandowski
 * Thanks to to my company http://www.ai.com.pl for allowing me free this work.
*/


#include <osgShadow/MinimalCullBoundsShadowMap>
#include <osgUtil/RenderLeaf>
#include <string.h>

#define IGNORE_OBJECTS_LARGER_THAN_HEIGHT 0

using namespace osgShadow;

MinimalCullBoundsShadowMap::MinimalCullBoundsShadowMap(): BaseClass()
{
}

MinimalCullBoundsShadowMap::MinimalCullBoundsShadowMap
(const MinimalCullBoundsShadowMap& copy, const osg::CopyOp& copyop) :
    BaseClass(copy,copyop)
{
}

MinimalCullBoundsShadowMap::~MinimalCullBoundsShadowMap()
{
}

void MinimalCullBoundsShadowMap::ViewData::init
    ( ThisClass *st, osgUtil::CullVisitor *cv )
{
    BaseClass::ViewData::init( st, cv );
    _frameShadowCastingCameraPasses = 2;
}

void MinimalCullBoundsShadowMap::ViewData::aimShadowCastingCamera
                                             ( const osg::Light *light,
                                               const osg::Vec4 &lightPos,
                                               const osg::Vec3 &lightDir,
                                               const osg::Vec3 &lightUp )
{
    MinimalShadowMap::ViewData::aimShadowCastingCamera
           ( light, lightPos, lightDir, lightUp );

    frameShadowCastingCamera
            ( _cv->getCurrentRenderBin()->getStage()->getCamera(), _camera.get() );
}

void MinimalCullBoundsShadowMap::ViewData::cullShadowReceivingScene( )
{
    RenderLeafList rllOld, rllNew;

    GetRenderLeaves( _cv->getRenderStage(), rllOld );

    MinimalShadowMap::ViewData::cullShadowReceivingScene( );

    GetRenderLeaves( _cv->getRenderStage(), rllNew );

    RemoveOldRenderLeaves( rllNew, rllOld );
    RemoveIgnoredRenderLeaves( rllNew );

    osg::Matrix projectionToModelSpace =
        osg::Matrix::inverse( *_modellingSpaceToWorldPtr *
            *_cv->getModelViewMatrix() * *_cv->getProjectionMatrix() );

    osg::BoundingBox bb;
    if( *_cv->getProjectionMatrix() != _clampedProjection ) {

        osg::Polytope polytope;
#if 1
        polytope.setToUnitFrustum();
#else
        polytope.add( osg::Plane( 0.0,0.0,-1.0,1.0 ) ); // only far plane
#endif
        polytope.transformProvidingInverse( *_modellingSpaceToWorldPtr *
                                  *_cv->getModelViewMatrix() * _clampedProjection );

        bb = ComputeRenderLeavesBounds( rllNew, projectionToModelSpace, polytope );
    } else {
        bb = ComputeRenderLeavesBounds( rllNew, projectionToModelSpace );
    }

    cutScenePolytope( *_modellingSpaceToWorldPtr,
                      osg::Matrix::inverse( *_modellingSpaceToWorldPtr ), bb );
}

void MinimalCullBoundsShadowMap::ViewData::GetRenderLeaves
    ( osgUtil::RenderBin *rb, RenderLeafList & rll )
{
    osgUtil::RenderBin::RenderBinList& bins = rb->getRenderBinList();
    osgUtil::RenderBin::RenderBinList::const_iterator rbitr;

    // scan pre render bins
    for(rbitr = bins.begin(); rbitr!=bins.end() && rbitr->first<0; ++rbitr )
       GetRenderLeaves( rbitr->second.get(), rll );

    // scan fine grained ordering.
    osgUtil::RenderBin::RenderLeafList& renderLeafList = rb->getRenderLeafList();
    osgUtil::RenderBin::RenderLeafList::const_iterator rlitr;
    for( rlitr= renderLeafList.begin(); rlitr!= renderLeafList.end(); ++rlitr )
    {
        rll.push_back( *rlitr );
    }

    // scan coarse grained ordering.
    osgUtil::RenderBin::StateGraphList& stateGraphList = rb->getStateGraphList();
    osgUtil::RenderBin::StateGraphList::const_iterator oitr;
    for( oitr= stateGraphList.begin(); oitr!= stateGraphList.end(); ++oitr )
    {
        for( osgUtil::StateGraph::LeafList::const_iterator dw_itr =
            (*oitr)->_leaves.begin(); dw_itr != (*oitr)->_leaves.end(); ++dw_itr)
        {
            rll.push_back( dw_itr->get() );
        }
    }

    // scan post render bins
    for(; rbitr!=bins.end(); ++rbitr )
       GetRenderLeaves( rbitr->second.get(), rll );
}

class CompareRenderLeavesByMatrices {
public:
  bool operator()( const osgUtil::RenderLeaf *a, const osgUtil::RenderLeaf *b )
  {
      if ( !a ) return false; // NULL render leaf goes last
      return !b ||
             a->_projection < b->_projection ||
             (a->_projection == b->_projection && a->_modelview < b->_modelview);
  }
};

inline bool CheckAndMultiplyBoxIfWithinPolytope
    ( osg::BoundingBox & bb, osg::Matrix & m, osg::Polytope &p )
{
    if( !bb.valid() ) return false;

    osg::Vec3 o = bb._min * m, s[3];

    for( int i = 0; i < 3; i ++ )
        s[i] = osg::Vec3( m(i,0), m(i,1), m(i,2) ) * ( bb._max[i] - bb._min[i] );

    for( osg::Polytope::PlaneList::iterator it = p.getPlaneList().begin();
         it != p.getPlaneList().end();
         ++it )
    {
        float dist = it->distance( o ), dist_min = dist, dist_max = dist;

        for( int i = 0; i < 3; i ++ ) {
            dist = it->dotProductNormal( s[i] );
            if( dist < 0 ) dist_min += dist; else dist_max += dist;
        }

        if( dist_max < 0 )
            return false;
    }

    bb._max = bb._min = o;
#if 1
    for( int i = 0; i < 3; i ++ )
        for( int j = 0; j < 3; j ++ )
            if( s[i][j] < 0 ) bb._min[j] += s[i][j]; else bb._max[j] += s[i][j];
#else
    b.expandBy( o + s[0] );
    b.expandBy( o + s[1] );
    b.expandBy( o + s[2] );
    b.expandBy( o + s[0] + s[1] );
    b.expandBy( o + s[0] + s[2] );
    b.expandBy( o + s[1] + s[2] );
    b.expandBy( o + s[0] + s[1] + s[2] );
#endif

#if ( IGNORE_OBJECTS_LARGER_THAN_HEIGHT > 0 )
   if( bb._max[2] - bb._min[2] > IGNORE_OBJECTS_LARGER_THAN_HEIGHT ) // ignore huge objects
       return false;
#endif

    return true;
}

unsigned MinimalCullBoundsShadowMap::ViewData::RemoveOldRenderLeaves
    ( RenderLeafList &rllNew, RenderLeafList &rllOld )
{
    unsigned count = 0;

    std::sort( rllOld.begin(), rllOld.end() );
    RenderLeafList::iterator itNew, itOld;
    for( itNew = rllNew.begin(); itNew != rllNew.end() && rllOld.size(); ++itNew )
    {
        itOld = std::lower_bound( rllOld.begin(), rllOld.end(), *itNew );

        if( itOld == rllOld.end() || *itOld != *itNew ) continue;
        // found !
        rllOld.erase( itOld ); //remove it from old range to speed up search
        *itNew = NULL; //its not new = invalidate it among new render leaves
        count ++;
    }

    return count;
}

unsigned MinimalCullBoundsShadowMap::ViewData::RemoveIgnoredRenderLeaves
    ( RenderLeafList &rll )
{
    unsigned count = 0;

    for( RenderLeafList::iterator it = rll.begin(); it != rll.end(); ++it )
    {
        if( !*it ) continue;

        const char * name = (*it)->_drawable->className();

        // Its a dirty but quick test (not very future proof)
        if( !name || name[0] != 'L' ) continue;

        if( !strcmp( name, "LightPointDrawable" ) ||
            !strcmp( name, "LightPointSpriteDrawable" ) )
        {
            *it = NULL; //ignored = invalidate this in new render leaves list
            count++;
        }
    }

    return count;
}

osg::BoundingBox MinimalCullBoundsShadowMap::ViewData::ComputeRenderLeavesBounds
    ( RenderLeafList &rll, osg::Matrix & projectionToWorld )
{
    osg::BoundingBox bbResult;

    if( rll.size() == 0 ) return bbResult;

    std::sort( rll.begin(), rll.end(), CompareRenderLeavesByMatrices() );

    osg::ref_ptr< osg::RefMatrix > modelview;
    osg::ref_ptr< osg::RefMatrix > projection;
    osg::Matrix viewToWorld, modelToWorld, *ptrProjection = NULL,
                *ptrViewToWorld = &projectionToWorld, *ptrModelToWorld;

    osg::BoundingBox bb;

    // compute bounding boxes but skip old ones (placed at the end as NULLs)
    for( RenderLeafList::iterator it = rll.begin(); ; ++it ) {
        // we actually allow to pass one element behind end to flush bb queue
        osgUtil::RenderLeaf *rl = ( it != rll.end() ? *it : NULL );

        // Don't trust already computed bounds for cull generated drawables
        // LightPointDrawable & LightPointSpriteDrawable are such examples
        // they store wrong recorded bounds from very first pass
        if( rl && rl->_modelview == NULL )
            rl->_drawable->dirtyBound();

        // Stay as long as possible in model space to minimize matrix ops
        if( rl && rl->_modelview == modelview && rl->_projection == projection ){
            bb.expandBy( rl->_drawable->getBoundingBox() );
        } else {
            if( bb.valid() ) {
                // Conditions to avoid matrix multiplications
                if( projection.valid() )
                {
                    if( projection.get() != ptrProjection )
                    {
                        ptrProjection = projection.get();
                        viewToWorld = *ptrProjection * projectionToWorld;
                    }
                    ptrViewToWorld = &viewToWorld;
                } else {
                    ptrViewToWorld = &projectionToWorld;
                }

                if( modelview.valid() )
                {
                    modelToWorld = *modelview.get() * *ptrViewToWorld;
                    ptrModelToWorld = &modelToWorld;
                } else {
                    ptrModelToWorld = ptrViewToWorld;
                }

                for( int i = 0; i < 8; i++ )
                    bbResult.expandBy( bb.corner( i ) * *ptrModelToWorld );
            }
            if( !rl ) break;
            bb = rl->_drawable->getBoundingBox();
            modelview = rl->_modelview;
            projection = rl->_projection;
        }
    }

    rll.clear();

    return bbResult;
}

osg::BoundingBox MinimalCullBoundsShadowMap::ViewData::ComputeRenderLeavesBounds
    ( RenderLeafList &rll, osg::Matrix & projectionToWorld, osg::Polytope & p )
{
    osg::BoundingBox bbResult, bb;

    if( rll.size() == 0 ) return bbResult;

    std::sort( rll.begin(), rll.end(), CompareRenderLeavesByMatrices() );

    osg::ref_ptr< osg::RefMatrix > modelview;
    osg::ref_ptr< osg::RefMatrix > projection;
    osg::Matrix viewToWorld, modelToWorld,
                *ptrProjection = NULL,
                *ptrViewToWorld = &projectionToWorld,
                *ptrModelToWorld = NULL;

    // compute bounding boxes but skip old ones (placed at the end as NULLs)
    for( RenderLeafList::iterator it = rll.begin(); it != rll.end(); ++it )
    {
        // we actually allow to pass one element behind end to flush bb queue
        osgUtil::RenderLeaf *rl = *it;
        if( !rl ) break;

        // Don't trust already computed bounds for cull generated drawables
        // LightPointDrawable & LightPointSpriteDrawable are such examples
        // they store wrong recorded bounds from very first pass
        if(rl->_modelview == NULL )
            rl->_drawable->dirtyBound();

        bb = rl->_drawable->getBoundingBox();
        if( !bb.valid() ) continue;

        // Stay as long as possible in model space to minimize matrix ops
        if( rl->_modelview != modelview || rl->_projection != projection )
        {
            projection = rl->_projection;
            if( projection.valid() )
            {
               if( projection.get() != ptrProjection )
               {
                   ptrProjection = projection.get();
                   viewToWorld = *ptrProjection * projectionToWorld;
               }
               ptrViewToWorld = &viewToWorld;
            }
            else
            {
               ptrViewToWorld = &projectionToWorld;
            }

            modelview = rl->_modelview;
            if( modelview.valid() )
            {
                modelToWorld = *modelview.get() * *ptrViewToWorld;
                ptrModelToWorld = &modelToWorld;
            }
            else
            {
                ptrModelToWorld = ptrViewToWorld;
            }
        }

        if( ptrModelToWorld && CheckAndMultiplyBoxIfWithinPolytope( bb, *ptrModelToWorld, p ) )
            bbResult.expandBy( bb );
    }

    rll.clear();

    return bbResult;
}

