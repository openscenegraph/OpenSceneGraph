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

#include <osgShadow/MinimalShadowMap>
#include <osgShadow/ConvexPolyhedron>
#include <osg/MatrixTransform>
#include <osgShadow/ShadowedScene>
#include <osg/ComputeBoundsVisitor>

using namespace osgShadow;

#define PRINT_SHADOW_TEXEL_TO_PIXEL_ERROR 0

MinimalShadowMap::MinimalShadowMap(): 
    BaseClass(), 
    _maxFarPlane( FLT_MAX ),
    _minLightMargin( 0 ),
    _shadowReceivingCoarseBoundAccuracy( BOUNDING_BOX )
{
    
}

MinimalShadowMap::MinimalShadowMap
(const MinimalShadowMap& copy, const osg::CopyOp& copyop) :
    BaseClass(copy,copyop),
    _maxFarPlane( copy._maxFarPlane ),
    _minLightMargin( copy._minLightMargin ),
    _shadowReceivingCoarseBoundAccuracy( copy._shadowReceivingCoarseBoundAccuracy )
{
}

MinimalShadowMap::~MinimalShadowMap() 
{
}

osg::BoundingBox MinimalShadowMap::ViewData::computeShadowReceivingCoarseBounds()
{
    // Default slowest but most precise
    ShadowReceivingCoarseBoundAccuracy accuracy = DEFAULT_ACCURACY;

    MinimalShadowMap * msm = dynamic_cast< MinimalShadowMap* >( _st.get() );
    if( msm ) accuracy = msm->getShadowReceivingCoarseBoundAccuracy();

    if( accuracy == MinimalShadowMap::EMPTY_BOX )
    {
        // One may skip coarse scene bounds computation if light is infinite.
        // Empty box will be intersected with view frustum so in the end 
        // view frustum will be used as bounds approximation.
        // But if light is nondirectional and bounds come out too large 
        // they may bring the effect of almost 180 deg perspective set 
        // up for shadow camera. Such projection will significantly impact 
        // precision of further math.

        return osg::BoundingBox();
    }

    if( accuracy == MinimalShadowMap::BOUNDING_SPHERE )
    {
        // faster but less precise rough scene bound computation
        // however if compute near far is active it may bring quite good result
        osg::Camera * camera = _cv->getRenderStage()->getCamera();
        osg::Matrix m = camera->getViewMatrix() * _clampedProjection;

        ConvexPolyhedron frustum;
        frustum.setToUnitFrustum();
        frustum.transform( osg::Matrix::inverse( m ), m );

        osg::BoundingSphere bs =_st->getShadowedScene()->getBound();
        osg::BoundingBox bb;
        bb.expandBy( bs );
        osg::Polytope box;
        box.setToBoundingBox( bb );

        frustum.cut( box );

        // approximate sphere with octahedron. Ie first cut by box then 
        // additionaly cut with the same box rotated 45, 45, 45 deg.
        box.transform( // rotate box around its center
            osg::Matrix::translate( -bs.center() ) *
            osg::Matrix::rotate( osg::PI_4, 0, 0, 1 ) * 
            osg::Matrix::rotate( osg::PI_4, 1, 1, 0 ) * 
            osg::Matrix::translate( bs.center() ) );
        frustum.cut( box );
    
        return frustum.computeBoundingBox( );    
    }
    
    if( accuracy == MinimalShadowMap::BOUNDING_BOX ) // Default
    {
        // more precise method but slower method 
        // bound visitor traversal takes lot of time for complex scenes 
        // (note that this adds to cull time)

        osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
        cbbv.setTraversalMask(_st->getShadowedScene()->getCastsShadowTraversalMask());
        _st->getShadowedScene()->osg::Group::traverse(cbbv);

        return cbbv.getBoundingBox();
    }

    return osg::BoundingBox();
}

void MinimalShadowMap::ViewData::aimShadowCastingCamera
 ( const osg::Light *light, const osg::Vec4 &lightPos,
   const osg::Vec3 &lightDir, const osg::Vec3 &lightUp )
{    
    osg::BoundingBox bb = computeScenePolytopeBounds();
    if( !bb.valid() ) { // empty scene or looking at the sky - substitute something
        bb.expandBy( osg::BoundingSphere( _cv->getEyePoint(), 1 ) );
    }

    osg::Vec3 up = lightUp;

    if( up.length2() <= 0 ) 
    {
    // This is extra step (not really needed but helpful in debuging) 
    // Compute such lightUp vector that shadow cam is intuitively aligned with eye
    // We compute this vector on -ZY view plane, perpendicular to light direction
    // Matrix m = ViewToWorld
#if 0
        osg::Matrix m = osg::Matrix::inverse( *cv.getModelViewMatrix() );
        osg::Vec3 camFw( -m( 2, 0 ), -m( 2, 1 ), -m( 2, 2 ) );
        camFw.normalize();

        osg::Vec3 camUp( m( 1, 0 ), m( 1, 1 ), m( 1, 2 ) );
        camUp.normalize();

        up = camUp * ( camFw * lightDir ) - camFw * ( camUp * lightDir );
        up.normalize();
#else
        osg::Matrix m = osg::Matrix::inverse( *_cv->getModelViewMatrix() );
        // OpenGL std cam looks along -Z axis so Cam Fw = [ 0  0  -1  0 ] * m
        up.set( -m( 2, 0 ), -m( 2, 1 ), -m( 2, 2 ) );
#endif
    } 

    BaseClass::ViewData::aimShadowCastingCamera
                                    ( bb, light, lightPos, lightDir, up );

    // Intersect scene Receiving Shadow Polytope with shadow camera frustum
    // Important for cases where Scene extend beyond shadow camera frustum
    // From this moment shadowed scene portion is fully contained by both
    // main camera frustum and shadow camera frustum
    osg::Matrix mvp = _camera->getViewMatrix() * _camera->getProjectionMatrix();
    cutScenePolytope( osg::Matrix::inverse( mvp ),  mvp );

    MinimalShadowMap::ViewData::frameShadowCastingCamera
            ( _cv->getRenderStage()->getCamera(), _camera.get(), 0 );
}

void MinimalShadowMap::ViewData::frameShadowCastingCamera
     ( const osg::Camera* cameraMain, osg::Camera* cameraShadow, int pass )
{
    osg::Matrix mvp = 
        cameraShadow->getViewMatrix() * cameraShadow->getProjectionMatrix();

    ConvexPolyhedron polytope = _sceneReceivingShadowPolytope;
    std::vector<osg::Vec3d> points = _sceneReceivingShadowPolytopePoints;

    osg::BoundingBox bb = computeScenePolytopeBounds( mvp );

    // projection was trimmed above, need to recompute mvp
    if( bb.valid() && *_minLightMarginPtr > 0 ) {
        //        bb._max += osg::Vec3( 1, 1, 1 );
        //        bb._min -= osg::Vec3( 1, 1, 1 );

        osg::Matrix transform = osg::Matrix::inverse( mvp );

        osg::Vec3d normal = osg::Matrix::transform3x3( osg::Vec3d(0,0,-1), transform );
        normal.normalize();
        _sceneReceivingShadowPolytope.extrude( normal * *_minLightMarginPtr );

        // Zero pass does crude shadowed scene hull approximation.
        // Its important to cut it to coarse light frustum properly 
        // at this stage.
        // If not cut and polytope extends beyond shadow projection clip 
        // space (-1..1), it may get "twisted" by precisely adjusted shadow cam 
        // projection in second pass. 

        if ( pass == 0 ) 
        { // Make sure extruded polytope does not extend beyond light frustum
            osg::Polytope lightFrustum;
            lightFrustum.setToUnitFrustum();
            lightFrustum.transformProvidingInverse( mvp );
            _sceneReceivingShadowPolytope.cut( lightFrustum );
        }

        _sceneReceivingShadowPolytopePoints.clear();
        _sceneReceivingShadowPolytope.getPoints
            ( _sceneReceivingShadowPolytopePoints );

        bb = computeScenePolytopeBounds( mvp );
    }

    setDebugPolytope( "extended", 
        _sceneReceivingShadowPolytope, osg::Vec4( 1, 0.5, 0, 1 ), osg::Vec4( 1, 0.5, 0, 0.1 ) );

    _sceneReceivingShadowPolytope = polytope;
    _sceneReceivingShadowPolytopePoints = points;
    
    // Warning: Trim light projection at near plane may remove shadowing 
    // from objects outside of view space but still casting shadows into it.
    // I have not noticed this issue so I left mask at default: all bits set.
    if( bb.valid() )
        trimProjection( cameraShadow->getProjectionMatrix(), bb, 1|2|4|8|16|32 );

    ///// Debuging stuff //////////////////////////////////////////////////////////
    setDebugPolytope( "scene", _sceneReceivingShadowPolytope, osg::Vec4(0,1,0,1) );    


#if PRINT_SHADOW_TEXEL_TO_PIXEL_ERROR
    if( pass == 1 )
        displayShadowTexelToPixelErrors
            ( cameraMain, cameraShadow, &_sceneReceivingShadowPolytope );
#endif

}

void MinimalShadowMap::ViewData::cullShadowReceivingScene( )
{
    BaseClass::ViewData::cullShadowReceivingScene( );

    _clampedProjection = *_cv->getProjectionMatrix();

    if( _cv->getComputeNearFarMode() ) {

        // Redo steps from CullVisitor::popProjectionMatrix()
        // which clamps projection matrix when Camera & Projection 
        // completes traversal of their children

        // We have to do this now manually 
        // because we did not complete camera traversal yet but 
        // we need to know how this clamped projection matrix will be

        _cv->computeNearPlane();
        
        osgUtil::CullVisitor::value_type n = _cv->getCalculatedNearPlane();
        osgUtil::CullVisitor::value_type f = _cv->getCalculatedFarPlane();

        if( n < f )
            _cv->clampProjectionMatrix( _clampedProjection, n, f );
    } 

    // Aditionally clamp far plane if shadows are don't need to be cast as 
    // far as main projection far plane 
    if( 0 < *_maxFarPlanePtr )
        clampProjection( _clampedProjection, 0.f, *_maxFarPlanePtr );

    // Give derived classes chance to initialize _sceneReceivingShadowPolytope     
    osg::BoundingBox bb = computeShadowReceivingCoarseBounds( );
    if( bb.valid() )
        _sceneReceivingShadowPolytope.setToBoundingBox( bb );
    else 
        _sceneReceivingShadowPolytope.clear();

    // Cut initial scene using main camera frustum. 
    // Cutting will work correctly on empty polytope too. 
    // Take into consideration near far calculation and _maxFarPlane variable


    osg::Matrix mvp = *_cv->getModelViewMatrix() * _clampedProjection;

    cutScenePolytope( osg::Matrix::inverse( mvp ), mvp );

    setDebugPolytope
        ( "frustum", _sceneReceivingShadowPolytope, osg::Vec4(1,0,1,1));

}

void MinimalShadowMap::ViewData::init( ThisClass *st, osgUtil::CullVisitor *cv )
{
    BaseClass::ViewData::init( st, cv );

    _modellingSpaceToWorldPtr = &st->_modellingSpaceToWorld;
    _minLightMarginPtr        = &st->_minLightMargin;
    _maxFarPlanePtr           = &st->_maxFarPlane;
}

void MinimalShadowMap::ViewData::cutScenePolytope
    ( const osg::Matrix & transform, 
      const osg::Matrix & inverse, 
      const osg::BoundingBox & bb )
{   
    _sceneReceivingShadowPolytopePoints.clear();

    if( bb.valid() ) {
        osg::Polytope polytope;
        polytope.setToBoundingBox( bb );
        polytope.transformProvidingInverse( inverse );
        _sceneReceivingShadowPolytope.cut( polytope );        
        _sceneReceivingShadowPolytope.getPoints
                                ( _sceneReceivingShadowPolytopePoints );
    } else
        _sceneReceivingShadowPolytope.clear();
}

osg::BoundingBox 
    MinimalShadowMap::ViewData::computeScenePolytopeBounds( const osg::Matrix & m )
{
    osg::BoundingBox bb;

    if( &m )
        for( unsigned i = 0; i < _sceneReceivingShadowPolytopePoints.size(); ++i )
            bb.expandBy( _sceneReceivingShadowPolytopePoints[i] * m );
    else 
        for( unsigned i = 0; i < _sceneReceivingShadowPolytopePoints.size(); ++i )
            bb.expandBy( _sceneReceivingShadowPolytopePoints[i] );

    return bb;
}



// Utility methods for adjusting projection matrices

void MinimalShadowMap::ViewData::trimProjection
    ( osg::Matrixd & projectionMatrix, osg::BoundingBox bb, unsigned int trimMask )
{
#if 1
    if( !bb.valid() || !( trimMask & (1|2|4|8|16|32) ) ) return;
    double l = -1, r = 1, b = -1, t = 1, n = 1, f = -1;

#if 0
    // make sure bounding box does not extend beyond unit frustum clip range
    for( int i = 0; i < 3; i ++ ) {
        if( bb._min[i] < -1 ) bb._min[i] = -1;
        if( bb._max[i] >  1 ) bb._max[i] =  1;
    }
#endif

    if( trimMask & 1 ) l = bb._min[0];
    if( trimMask & 2 ) r = bb._max[0];
    if( trimMask & 4 ) b = bb._min[1];
    if( trimMask & 8 ) t = bb._max[1];
    if( trimMask & 16 ) n = -bb._min[2];
    if( trimMask & 32 ) f = -bb._max[2];

    projectionMatrix.postMult( osg::Matrix::ortho( l,r,b,t,n,f ) );
#else
    if( !bb.valid() || !( trimMask & (1|2|4|8|16|32) ) ) return;
    double l, r, t, b, n, f;
    bool ortho = projectionMatrix.getOrtho( l, r, b, t, n, f ); 
    if( !ortho && !projectionMatrix.getFrustum( l, r, b, t, n, f ) )
        return; // rotated or skewed or other crooked projection - give up

    // make sure bounding box does not extend beyond unit frustum clip range
    for( int i = 0; i < 3; i ++ ) {
        if( bb._min[i] < -1 ) bb._min[i] = -1;
        if( bb._max[i] >  1 ) bb._max[i] =  1;
    }

    osg::Matrix projectionToView = osg::Matrix::inverse( projectionMatrix );
    
    osg::Vec3 min =
        osg::Vec3( bb._min[0], bb._min[1], bb._min[2] ) * projectionToView;

    osg::Vec3 max =
        osg::Vec3( bb._max[0], bb._max[1], bb._max[2] ) * projectionToView;

    if( trimMask & 16 ) { // trim near
        if( !ortho ) { // recalc frustum corners on new near plane
            l *= -min[2] / n;
            r *= -min[2] / n;
            b *= -min[2] / n;
            t *= -min[2] / n;
        }
        n = -min[2];
    }

    if( trimMask & 32 ) // trim far
        f = -max[2];

    if( !ortho ) {
        min[0] *=  -n / min[2];
        min[1] *=  -n / min[2];
        max[0] *=  -n / max[2];
        max[1] *=  -n / max[2];
    }

    if( l < r ) { // check for inverted X range
        if( l < min[0] && ( trimMask & 1 ) ) l = min[0];
        if( r > max[0] && ( trimMask & 2 ) ) r = max[0];
    } else {
        if( l > min[0] && ( trimMask & 1 ) ) l = min[0];
        if( r < max[0] && ( trimMask & 2 ) ) r = max[0];
    }

    if( b < t ) { // check for inverted Y range
        if( b < min[1] && ( trimMask & 4 ) ) b = min[1];
        if( t > max[1] && ( trimMask & 8 ) ) t = max[1];
    } else {
        if( b > min[1] && ( trimMask & 4 ) ) b = min[1];
        if( t < max[1] && ( trimMask & 8 ) ) t = max[1];
    }

    if( ortho ) 
        projectionMatrix.makeOrtho( l, r, b, t, n, f );
    else 
        projectionMatrix.makeFrustum( l, r, b, t, n, f );
#endif
}

void MinimalShadowMap::ViewData::clampProjection
                    ( osg::Matrixd & projection, float new_near, float new_far )
{
    double r, l, t, b, n, f;
    bool perspective = projection.getFrustum( l, r, b, t, n, f );
    if( !perspective && !projection.getOrtho( l, r, b, t, n, f ) )
    {
        // What to do here ?
        osg::notify( osg::WARN ) 
            << "MinimalShadowMap::clampProjectionFarPlane failed - non standard matrix"
            << std::endl;

    } else if( n < new_near || new_far < f ) {

        if( n < new_near && new_near < f ) {
            if( perspective ) {
                l *= new_near / n;
                r *= new_near / n;
                b *= new_near / n;
                t *= new_near / n;
            }
            n = new_near;
        } 

        if( n < new_far && new_far < f ) {
            f = new_far;             
        }

        if( perspective )
            projection.makeFrustum( l, r, b, t, n, f );
        else
            projection.makeOrtho( l, r, b, t, n, f );                     
    }
}

// Imagine following scenario: 
// We stand in the room and look through the window.
// How should our view change if we were looking through larger window ?
// In other words how should projection be adjusted if 
// window had grown by some margin ? 
// Method computes such new projection which maintains perpective/world ratio

void MinimalShadowMap::ViewData::extendProjection
    ( osg::Matrixd & projection, osg::Viewport * viewport, const osg::Vec2& margin )
{
  double l,r,b,t,n,f;

  //osg::Matrix projection = camera.getProjectionMatrix();

  bool frustum = projection.getFrustum( l,r,b,t,n,f );

  if( !frustum && !projection.getOrtho( l,r,b,t,n,f ) ) {
    osg::notify( osg::WARN )
        << " Awkward projection matrix. ComputeExtendedProjection failed"
        << std::endl;
    return;
  }

  osg::Matrix window = viewport->computeWindowMatrix();
 
  osg::Vec3 vMin( viewport->x() - margin.x(), 
                 viewport->y() - margin.y(), 
                 0.0 );

  osg::Vec3 vMax( viewport->width() + margin.x() * 2  + vMin.x(), 
                  viewport->height() + margin.y() * 2  + vMin.y(), 
                  0.0 );
  
  osg::Matrix inversePW = osg::Matrix::inverse( projection * window );

  vMin = vMin * inversePW;
  vMax = vMax * inversePW;
  
  l = vMin.x(); 
  r = vMax.x();
  b = vMin.y(); 
  t = vMax.y();

  if( frustum )
    projection.makeFrustum( l,r,b,t,n,f );
  else 
    projection.makeOrtho( l,r,b,t,n,f );
}

