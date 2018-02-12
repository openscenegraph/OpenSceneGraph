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


#include <osgShadow/LightSpacePerspectiveShadowMap>
#include <osg/io_utils>
#include <iostream>
#include <iomanip>

#define DIRECTIONAL_ONLY     0
#define DIRECTIONAL_ADAPTED  1
#define DIRECTIONAL_AND_SPOT 2

//#define LISPSM_ALGO DIRECTIONAL_ONLY
#define LISPSM_ALGO DIRECTIONAL_ADAPTED
//#define LISPSM_ALGO DIRECTIONAL_AND_SPOT

#define ROBERTS_TEST_CHANGES 1

#define PRINT_COMPUTED_N_OPT 0

using namespace osgShadow;

////////////////////////////////////////////////////////////////////////////////
// There are two slightly differing implemetations available on
// "Light Space Perspective Shadow Maps" page. One from 2004 and other from 2006.
// Our implementation is written in two versions based on these solutions.
////////////////////////////////////////////////////////////////////////////////
// Original LisPSM authors 2004 implementation excerpt. Kept here for reference.
// DIRECTIONAL AND DIRECTIONAL_ADAPTED versions are based on this code.
// DIRECTIONAL_AND_SPOT version is based on later 2006 code.
////////////////////////////////////////////////////////////////////////////////
 #if 0
////////////////////////////////////////////////////////////////////////////////
// This code is copyright Vienna University of Technology, 2004.
//
// Please feel FREE to COPY and USE the code to include it in your own work,
// provided you include this copyright notice.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Authors Code:
// Daniel Scherzer (scherzer@cg.tuwien.ac.at)
//
// Authors Paper:
// Michael Wimmer (wimmer@cg.tuwien.ac.at)
// Daniel Scherzer (scherzer@cg.tuwien.ac.at)
// Werner Purgathofer
////////////////////////////////////////////////////////////////////////////////
void calcLispSMMtx(struct VecPoint* B) {
    Vector3 min, max;
    Vector3 up;
    Matrix4x4 lispMtx;
    struct VecPoint Bcopy = VECPOINT_NULL;
    double dotProd = dot(viewDir,lightDir);
    double sinGamma;

    sinGamma = sqrt(1.0-dotProd*dotProd);

    copyMatrix(lispMtx,IDENTITY);

    copyVecPoint(&Bcopy,*B);

    //CHANGED
    if(useBodyVec) {
        Vector3 newDir;
        calcNewDir(newDir,B);
        calcUpVec(up,newDir,lightDir);
    }
    else {
        calcUpVec(up,viewDir,lightDir);
    }

    //temporal light View
    //look from position(eyePos)
    //into direction(lightDir)
    //with up vector(up)
    look(lightView,eyePos,lightDir,up);

    //transform the light volume points from world into light space
    transformVecPoint(B,lightView);

    //calculate the cubic hull (an AABB)
    //of the light space extents of the intersection body B
    //and save the two extreme points min and max
    calcCubicHull(min,max,B->points,B->size);

    {
        //use the formulas of the paper to get n (and f)
        const double factor = 1.0/sinGamma;
        const double z_n = factor*nearDist; //often 1
        const double d = absDouble(max[1]-min[1]); //perspective transform depth //light space y extents
        const double z_f = z_n + d*sinGamma;
        const double n = (z_n+sqrt(z_f*z_n))/sinGamma;
        const double f = n+d;
        Vector3 pos;

        //new observer point n-1 behind eye position
        //pos = eyePos-up*(n-nearDist)
        linCombVector3(pos,eyePos,up,-(n-nearDist));

        look(lightView,pos,lightDir,up);

        //one possibility for a simple perspective transformation matrix
        //with the two parameters n(near) and f(far) in y direction
        copyMatrix(lispMtx,IDENTITY);    // a = (f+n)/(f-n); b = -2*f*n/(f-n);
        lispMtx[ 5] = (f+n)/(f-n);        // [ 1 0 0 0]
        lispMtx[13] = -2*f*n/(f-n);        // [ 0 a 0 b]
        lispMtx[ 7] = 1;                // [ 0 0 1 0]
        lispMtx[15] = 0;                // [ 0 1 0 0]

        //temporal arrangement for the transformation of the points to post-perspective space
        mult(lightProjection,lispMtx,lightView); // ligthProjection = lispMtx*lightView

        //transform the light volume points from world into the distorted light space
        transformVecPoint(&Bcopy,lightProjection);

        //calculate the cubic hull (an AABB)
        //of the light space extents of the intersection body B
        //and save the two extreme points min and max
        calcCubicHull(min,max,Bcopy.points,Bcopy.size);
    }

    //refit to unit cube
    //this operation calculates a scale translate matrix that
    //maps the two extreme points min and max into (-1,-1,-1) and (1,1,1)
    scaleTranslateToFit(lightProjection,min,max);

    //together
    mult(lightProjection,lightProjection,lispMtx); // ligthProjection = scaleTranslate*lispMtx
}
#endif

#if ( LISPSM_ALGO == DIRECTIONAL_ONLY )


LightSpacePerspectiveShadowMapAlgorithm::LightSpacePerspectiveShadowMapAlgorithm()
{
    lispsm = NULL;
}

LightSpacePerspectiveShadowMapAlgorithm::~LightSpacePerspectiveShadowMapAlgorithm()
{
}

void LightSpacePerspectiveShadowMapAlgorithm::operator()
    ( const osgShadow::ConvexPolyhedron* hullShadowedView,
      const osg::Camera* cameraMain,
      osg::Camera* cameraShadow ) const
{
    osg::BoundingBox bb = hullShadowedView->computeBoundingBox( cameraMain->getViewMatrix() );
    double nearDist = -bb._max[2];

    const osg::Matrix & eyeViewToWorld = cameraMain->getInverseViewMatrix();

    osg::Matrix lightViewToWorld = cameraShadow->getInverseViewMatrix();

    osg::Vec3d eyePos = osg::Vec3d( 0, 0, 0 ) * eyeViewToWorld;

    osg::Vec3d viewDir( osg::Matrix::transform3x3( osg::Vec3d(0,0,-1), eyeViewToWorld ) );

    osg::Vec3d lightDir( osg::Matrix::transform3x3( osg::Vec3d( 0,0,-1), lightViewToWorld ) );
    osg::Vec3d up( osg::Matrix::transform3x3( osg::Vec3d(0,1,0), lightViewToWorld ) );

    osg::Matrix lightView; // compute coarse light view matrix
    lightView.makeLookAt( eyePos, eyePos + lightDir, up );
    bb = hullShadowedView->computeBoundingBox( lightView );

    const double dotProd = viewDir * lightDir;
    const double sinGamma = sqrt(1.0- dotProd*dotProd);
    const double factor = 1.0/sinGamma;
    const double z_n = factor*nearDist; //often 1
    //use the formulas of the paper to get n (and f)
    const double d = fabs( bb._max[1]-bb._min[1]); //perspective transform depth //light space y extents
    const double z_f = z_n + d*sinGamma;
    const double n = (z_n+sqrt(z_f*z_n))/sinGamma;
    const double f = n+d;
    osg::Vec3d pos = eyePos-up*(n-nearDist);

#if PRINT_COMPUTED_N_OPT
    std::cout
       << " N=" << std::setw(8) << n
       << " n=" << std::setw(8) << z_n
       << " f=" << std::setw(8) << z_f
       << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
       << std::flush;
#endif

    lightView.makeLookAt( pos, pos + lightDir, up );

    //one possibility for a simple perspective transformation matrix
    //with the two parameters n(near) and f(far) in y direction
    double a = (f+n)/(f-n);
    double b = -2*f*n/(f-n);

    osg::Matrix lispProjection( 1, 0, 0, 0,
                                0, a, 0, 1,
                                0, 0,-1, 0,
                                0, b, 0, 0 );

//    lispProjection.makeIdentity( );
#if 0
    {
        osg::Matrix mvp = _camera->getViewMatrix() *
                      _camera->getProjectionMatrix();

        extendScenePolytope( mvp, osg::Matrix::inverse( mvp ) );
    }
#endif

    bb = hullShadowedView->computeBoundingBox( lightView * lispProjection );

    osg::Matrix fitToUnitFrustum;
    fitToUnitFrustum.makeOrtho( bb._min[0],  bb._max[0],
                                bb._min[1],  bb._max[1],
                               -(bb._min[2]-1), -bb._max[2] );

    cameraShadow->setProjectionMatrix
        ( lightViewToWorld * lightView * lispProjection * fitToUnitFrustum );


#if 0 // DOUBLE CHECK!
    bb = computeScenePolytopeBounds
        ( cameraShadow->getViewMatrix() * cameraShadow->getProjectionMatrix() );

    if( !osg::equivalent( 0.f, (bb._min - osg::Vec3d(-1,-1,-1)).length2() ) ||
        !osg::equivalent( 0.f, (bb._max - osg::Vec3d( 1, 1, 1)).length2() ) )
    {
        bb = computeScenePolytopeBounds
            ( cameraShadow->getViewMatrix() * cameraShadow->getProjectionMatrix() );
    }
#endif
}

#endif



#if ( LISPSM_ALGO == DIRECTIONAL_ADAPTED )

LightSpacePerspectiveShadowMapAlgorithm::LightSpacePerspectiveShadowMapAlgorithm()
{
    lispsm = NULL;
}

LightSpacePerspectiveShadowMapAlgorithm::~LightSpacePerspectiveShadowMapAlgorithm()
{
}

void LightSpacePerspectiveShadowMapAlgorithm::operator()
    ( const osgShadow::ConvexPolyhedron* hullShadowedView,
      const osg::Camera* cameraMain,
      osg::Camera* cameraShadow ) const
{

    // all computations are done in post projection light space
    // which means we are in left handed coordinate system
    osg::Matrix mvpLight =
        cameraShadow->getViewMatrix() * cameraShadow->getProjectionMatrix();

    osg::Matrix m = cameraMain->getInverseViewMatrix() * mvpLight;
    osg::Vec3 eye = osg::Vec3( 0, 0, 0 ) * m;
    osg::Vec3 center = osg::Vec3( 0, 0, -1 ) * m;
    osg::Vec3 up(0,1,0);
    osg::Vec3 viewDir( center - eye );
    viewDir.normalize();

    m.makeLookAt( eye, center, up );

    osg::BoundingBox bb = hullShadowedView->computeBoundingBox( mvpLight * m );
    if( !bb.valid() )
    {
#if ROBERTS_TEST_CHANGES
        // OSG_NOTICE<<"LightSpacePerspectiveShadowMapAlgorithm::operator() invalid bb A"<<std::endl;
#endif
        return;
    }

    double nearDist = -bb._max[2];

#if 1
    // Original LiSPSM Paper suggests that algorithm should work for all light types:
    // infinite directional, omnidirectional and spot types may be treated as directional
    // as all computations are performed in post projection light space.
    // Frankly, I have my doubts if their error analysis and methodology still works
    // in non directional lights post projective space. But since I can't prove it doesn't,
    // I assume it does ;-). So I made an effort to modify their original directional algo
    // to work in true light post perspective space and compute all params in this space.
    // And here is a snag. Although shadowed hull fits completely into light space,
    // camera position may not, and after projective transform it may land outside
    // light frustum and even on/or below infinity plane. I need camera pos to compute
    // minimal distance to shadowed hull. If its not right rest of the computation may
    // be completely off. So in the end this approach is not singulartity free.
    // I guess this problem is solvable in other way but "this other
    // way" looks like a topic for other scientific paper and I am definitely not that
    // ambitious ;-). So for the time being I simply try to discover when this happens and
    // apply workaround, I found works. This workaround may mean that adjusted projection
    // may not be optimal in original LisPSM Lmax norm sense. But as I wrote above,
    // I doubt they are optimal when Light is not directional, anyway.

    // Seems that most nasty case when algorithm fails is when cam pos is
    // below light frustum near plane but above infinity plane - when this occurs
    // shadows simply disappear. My workaround is to find this case by
    // checking light postperspective transform camera z ( negative value means this )
    // and make sure min distance to shadow hull is clamped to positive value.

    if( eye[2] < 0 && nearDist <= 0 )
    {
        float clampedNearDist = 0.0001;
        eye += viewDir * ( clampedNearDist - nearDist );
        nearDist = clampedNearDist;
#if ROBERTS_TEST_CHANGES
        // OSG_NOTICE<<"LightSpacePerspectiveShadowMapAlgorithm::operator() adjusting eye"<<std::endl;
#endif

    }
#endif


#if ROBERTS_TEST_CHANGES
    if (nearDist<0.0)
    {
        nearDist = 0.0;
        // OSG_NOTICE<<"LightSpacePerspectiveShadowMapAlgorithm::operator() nearDist<0.0, resetting to 0.0."<<std::endl;
    }
#endif

    // Beware!!! Dirty Tricks:
    // Light direction in light post proj space is actually (0,0,1)
    // But since we want to pass it to std OpenGL right handed coordinate
    // makeLookAt function we compensate the effects by also using right
    // handed view forward vector (ie 0,0,-1) instead.
    // So in the end we get left handed makeLookAt behaviour (D3D like)...
    // I agree this method is bizarre. But it works so I left it as is.
    // It sort of came out by itself through trial and error.
    // I later understoood why it works.

    osg::Vec3 lightDir(0,0,-1);
    osg::Matrix lightView; // compute coarse light view matrix
    lightView.makeLookAt( eye, eye + lightDir, up );
    bb = hullShadowedView->computeBoundingBox( mvpLight * lightView );
    if( !bb.valid() )
    {
#if ROBERTS_TEST_CHANGES
        // OSG_NOTICE<<"LightSpacePerspectiveShadowMapAlgorithm::operator() invalid bb B"<<std::endl;
#endif
        return;
    }

    //use the formulas from the LiSPSM paper to get n (and f)
    const double dotProd = viewDir * lightDir;
    const double sinGamma = sqrt(1.0- dotProd*dotProd);
    const double factor = 1.0/sinGamma;
    const double z_n = factor*nearDist;
    //perspective transform depth light space y extents
    const double d = fabs( bb._max[1]-bb._min[1]);
    const double z_f = z_n + d*sinGamma;
    double n = (z_n+sqrt(z_f*z_n))/sinGamma;

#if ROBERTS_TEST_CHANGES
    // clamp the localtion of p so that it isn't too close to the eye as to cause problems
    float minRatio=0.02;
    if (n<d*minRatio)
    {
        n=d*minRatio;
        // OSG_NOTICE<<"LightSpacePerspectiveShadowMapAlgorithm::operator() n too small, clamping n to "<<minRatio<<"*d."<<std::endl;
    }
#endif
    const double f = n+d;
    osg::Vec3d pos = eye-up*(n-nearDist);
    //pos = eye;
    lightView.makeLookAt( pos, pos + lightDir, up );

    //one possibility for a simple perspective transformation matrix
    //with the two parameters n(near) and f(far) in y direction
    double a = (f+n)/(f-n);
    double b = -2*f*n/(f-n);

    osg::Matrix lispProjection( 1, 0, 0, 0,
                                0, a, 0, 1,
                                0, 0, 1, 0,
                                0, b, 0, 0 );

    cameraShadow->setProjectionMatrix
        ( cameraShadow->getProjectionMatrix() * lightView * lispProjection );

    //OSG_NOTICE<<"LightSpacePerspectiveShadowMapAlgorithm::operator() normal case dotProd="<<dotProd<<", sinGamma="<<sinGamma<<" d="<<d<<", pos=("<<pos<<"), (n-nearDist)="<<(n-nearDist)<<std::endl;
    //OSG_NOTICE<<"    eye=("<<eye<<"), up=("<<up<<"), n="<<n<<", nearDist="<<nearDist<<", z_n="<<z_n<<", z_f="<<z_f<<std::endl;
}

#endif

#if ( LISPSM_ALGO == DIRECTIONAL_AND_SPOT )


// Adapted Modified version of LispSM authors implementation from 2006
// Nopt formula differs from the paper. I adopted original authors class to
// use with OSG



//we search the point in the LVS volume that is nearest to the camera
#include <limits.h>
static const float OSG_INFINITY = FLT_MAX;

namespace osgShadow {

class LispSM {
public:
    typedef std::vector<osg::Vec3d> Vertices;

    void setProjectionMatrix( const osg::Matrix & projectionMatrix )
        { _projectionMatrix = projectionMatrix; }

    void setViewMatrix( const osg::Matrix & viewMatrix )
        { _viewMatrix = viewMatrix; }

    void setHull( const ConvexPolyhedron & hull )
        { _hull = hull; }

    const ConvexPolyhedron & getHull( ) const
        { return _hull; }

    const osg::Matrix & getProjectionMatrix( void ) const
        { return _projectionMatrix; }

    const osg::Matrix & getViewMatrix( void ) const
        { return _viewMatrix; }

    bool getUseLiSPSM() const
        { return _useLiSPSM; }

    void setUseLiSPSM( bool use )
        { _useLiSPSM = use; }

    bool getUseFormula() const
        { return _useFormula; }

    void setUseFormula( bool use )
        { _useFormula = use; }

    bool getUseOldFormula() const
        { return _useOldFormula; }

    void setUseOldFormula( bool use )
        { _useOldFormula = use; }

    void setN(const double& n )
        { _N = n; }

    const double getN() const
        { return _N; }

    //for old LispSM formula from paper
    const double getNearDist() const
        { return _nearDist; }

    void setNearDist( const double & nearDist )
        { _nearDist = nearDist; }

    const double getFarDist() const
        { return _farDist; }

    void setFarDist( const double & farDist )
        { _farDist = farDist; }

    const osg::Vec3d & getEyeDir() const
        { return _eyeDir; }

    const osg::Vec3d & getLightDir() const
        { return _lightDir; }

    void setEyeDir( const osg::Vec3d eyeDir )
        { _eyeDir = eyeDir; }

    void setLightDir( const osg::Vec3d lightDir )
        { _lightDir = lightDir; }

protected:

    bool        _useLiSPSM;
    bool        _useFormula;
    bool        _useOldFormula;
    double      _N;
    double      _nearDist;
    double      _farDist;

    mutable osg::Vec3d  _E;
    osg::Vec3d  _eyeDir;
    osg::Vec3d  _lightDir;

    ConvexPolyhedron _hull;

    osg::Matrix _viewMatrix;
    osg::Matrix _projectionMatrix;

    double      getN(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const;

    osg::Vec3d  getNearCameraPointE() const;

    osg::Vec3d  getZ0_ls
                    (const osg::Matrix& lightSpace, const osg::Vec3d& e, const double& b_lsZmax, const osg::Vec3d& eyeDir) const;

    double      calcNoptGeneral
                    (const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const;

    double      calcNoptOld
                    ( const double gamma_ = 999) const;

    osg::Matrix getLispSmMtx
                    (const osg::Matrix& lightSpace) const;

    osg::Vec3d  getProjViewDir_ls
                    (const osg::Matrix& lightSpace) const;

    void        updateLightMtx
                    (osg::Matrix& lightView, osg::Matrix& lightProj, const std::vector<osg::Vec3d>& B) const;

public:
    LispSM( ) : _useLiSPSM( true ), _useFormula( true ), _useOldFormula( false ), _N( 1 ), _nearDist( 1 ), _farDist( 10 ) { }

    virtual void updateLightMtx( osg::Matrix& lightView, osg::Matrix& lightProj ) const;
};

}

osg::Vec3d LispSM::getNearCameraPointE( ) const
{
    const osg::Matrix& eyeView = getViewMatrix();

    ConvexPolyhedron::Vertices LVS;
    _hull.getPoints( LVS );

    //the LVS volume is always in front of the camera
    //the camera points along the neg z axis.
    //-> so the nearest point is the maximum

    unsigned max = 0;
    for(unsigned i = 0; i < LVS.size(); i++) {

        LVS[i] = LVS[i] * eyeView;

        if( LVS[max].z() < LVS[i].z() ) {
            max = i;
        }
    }
    //transform back to world space
    return LVS[max] * osg::Matrix::inverse( eyeView );
}

//z0 is the point that lies on the plane A parallel to the near plane through e
//and on the near plane of the C frustum (the plane z = bZmax) and on the line x = e.x
osg::Vec3d LispSM::getZ0_ls
    (const osg::Matrix& lightSpace, const osg::Vec3d& e, const double& b_lsZmax, const osg::Vec3d& eyeDir) const
{
    //to calculate the parallel plane to the near plane through e we
    //calculate the plane A with the three points
    osg::Plane A(eyeDir,e);
    //to transform plane A into lightSpace
    A.transform( lightSpace );
    //transform to light space
    const osg::Vec3d e_ls = e * lightSpace;

    //z_0 has the x coordinate of e, the z coord of B_lsZmax
    //and the y coord of the plane A and plane (z==B_lsZmax) intersection
#if 1
    osg::Vec3d v = osg::Vec3d(e_ls.x(),0,b_lsZmax);

    // x & z are given. We compute y from equations:
    // A.distance( x,y,z ) == 0
    // A.distance( x,y,z ) == A.distance( x,0,z ) + A.normal.y * y
    // hence A.distance( x,0,z ) == -A.normal.y * y

    v.y() = -A.distance( v ) / A.getNormal().y();
#else
    //get the parameters of A from the plane equation n dot d = 0
    const double d = A.asVec4()[3];
    const osg::Vec3d n = A.getNormal();
    osg::Vec3d v(e_ls.x(),(-d-n.z()*b_lsZmax-n.x()*e_ls.x())/n.y(),b_lsZmax);
#endif

    return v;

}

double LispSM::calcNoptGeneral(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const
{
    const osg::Matrix& eyeView = getViewMatrix();
    const osg::Matrix invLightSpace = osg::Matrix::inverse( lightSpace );

    const osg::Vec3d z0_ls = getZ0_ls(lightSpace, _E,B_ls.zMax(),getEyeDir());
    const osg::Vec3d z1_ls = osg::Vec3d(z0_ls.x(),z0_ls.y(),B_ls.zMin());

    //to world
    const osg::Vec4d z0_ws = osg::Vec4d( z0_ls, 1 ) * invLightSpace;
    const osg::Vec4d z1_ws = osg::Vec4d( z1_ls, 1 ) * invLightSpace;

    //to eye
    const osg::Vec4d z0_cs = z0_ws * eyeView;
    const osg::Vec4d z1_cs = z1_ws * eyeView;

    double z0 = -z0_cs.z() / z0_cs.w();
    double z1 = -z1_cs.z() / z1_cs.w();

    if( z1 / z0 <= 1.0 ) {

        // solve camera pos singularity in light space problem brutally:
        // if extreme points of B projected to Light space extend beyond
        // camera frustum simply use B extents in camera frustum

        // Its not optimal selection but ceratainly better than negative N
        osg::BoundingBox bb = _hull.computeBoundingBox( eyeView );
        z0 = -bb.zMax();
        if( z0 <= 0 )
            z0 = 0.1;

        z1 = -bb.zMin();
        if( z1 <= z0 )
            z1 = z0 + 0.1;
    }

    const double d = osg::absolute(B_ls.zMax()-B_ls.zMin());

    double N = d/( sqrt( z1 / z0 ) - 1.0 );
#if PRINT_COMPUTED_N_OPT
    std::cout
       << " N=" << std::setw(8) << N
       << " n=" << std::setw(8) << z0
       << " f=" << std::setw(8) << z1
       << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
       << std::flush;
#endif
    return N;
}

double LispSM::calcNoptOld( const double gamma_ ) const
{
    const double& n = getNearDist();
    const double& f = getFarDist();
    const double d = abs(f-n);
    double sinGamma(0);
    if(999 == gamma_) {
        double dot = getEyeDir() * getLightDir();
        sinGamma = sqrt( 1.0 - dot * dot );
    }
    else {
        sinGamma = sin(gamma_);
    }

    double N = (n+sqrt(n*(n+d*sinGamma)))/sinGamma;
#if PRINT_COMPUTED_N_OPT
    std::cout
       << " N=" << std::setw(8) << N
       << " n=" << std::setw(8) << n
       << " f=" << std::setw(8) << f
       << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
       << std::flush;
#endif
    return N;
}

double LispSM::getN(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const
{
    if( getUseFormula()) {
        if( getUseOldFormula() )
            return calcNoptOld();
        else
            return calcNoptGeneral(lightSpace,B_ls);
    }
    else {
        return getN();
    }
}
//this is the algorithm discussed in the article
osg::Matrix LispSM::getLispSmMtx( const osg::Matrix& lightSpace ) const
{
    const osg::BoundingBox B_ls = _hull.computeBoundingBox( lightSpace );

    const double n = getN(lightSpace,B_ls);

    //get the coordinates of the near camera point in light space
    const osg::Vec3d e_ls = _E * lightSpace;
    //c start has the x and y coordinate of e, the z coord of B.min()
    const osg::Vec3d Cstart_lp(e_ls.x(),e_ls.y(),B_ls.zMax());

    if( n >= OSG_INFINITY ) {
        //if n is inf. than we should do uniform shadow mapping
        return osg::Matrix::identity();
    }
    //calc C the projection center
    //new projection center C, n behind the near plane of P
    //we work along a negative axis so we transform +n*<the positive axis> == -n*<neg axis>
    const osg::Vec3d C( Cstart_lp + osg::Vec3d(0,0,1) * n );
    //construct a translation that moves to the projection center
    const osg::Matrix projectionCenter = osg::Matrix::translate( -C );

    //calc d the perspective transform depth or light space y extents
    const double d = osg::absolute(B_ls.zMax()-B_ls.zMin());

    //the lispsm perspective transformation

    //here done with a standard frustum call that maps P onto the unit cube with
    //corner points [-1,-1,-1] and [1,1,1].
    //in directX you can use the same mapping and do a mapping to the directX post-perspective cube
    //with corner points [-1,-1,0] and [1,1,1] as the final step after all the shadow mapping.
    osg::Matrix P = osg::Matrix::frustum( -1.0,1.0,-1.0,1.0, n, n+d );

    //invert the transform from right handed into left handed coordinate system for the ndc
    //done by the openGL style frustumGL call
    //so we stay in a right handed system
    P = P * osg::Matrix::scale( 1.0,1.0,-1.0 );
    //return the lispsm frustum with the projection center
    return projectionCenter * P;
}

osg::Vec3d LispSM::getProjViewDir_ls(const osg::Matrix& lightSpace ) const {
    //get the point in the LVS volume that is nearest to the camera
    const osg::Vec3d e = _E;
    //construct edge to transform into light-space
    const osg::Vec3d b = e+getEyeDir();
    //transform to light-space
    osg::Vec4d e_lp = osg::Vec4d( e, 1.0 ) * lightSpace;
    osg::Vec4d b_lp = osg::Vec4d( b, 1.0 ) * lightSpace;

    if( e_lp[3] <= 0 )
    {
        e_lp[3] = e_lp[3];
    }

    if( b_lp[3] <= 0 )
    {
        osg::Vec4d v = (e_lp - b_lp)/(e_lp[3]-b_lp[3]);

        v = ( e_lp + v  ) * 0.5;

        b_lp = v;
    }

    osg::Vec3d projDir( osg::Vec3( b_lp[0], b_lp[1], b_lp[2] ) / b_lp[3] -
                        osg::Vec3( e_lp[0], e_lp[1], e_lp[2] ) / e_lp[3] );

    projDir.normalize();

    //project the view direction into the shadow map plane
    projDir.y() = 0.0;
    return projDir;
}

void LispSM::updateLightMtx
    ( osg::Matrix& lightView, osg::Matrix& lightProj ) const
{
    //calculate standard light space for spot or directional lights
    //this routine returns two matrices:
    //lightview contains the rotated translated frame
    //lightproj contains in the case of a spot light the spot light perspective transformation
    //in the case of a directional light a identity matrix
    // calcLightSpace(lightView,lightProj);

    if( _hull._faces.empty() ) {
        //debug() << "empty intersection body -> completely inside shadow\n";//debug output
        return;
    }

    _E = getNearCameraPointE();

    lightProj = lightProj * osg::Matrix::scale( 1, 1, -1 );

    //coordinate system change for calculations in the article
    osg::Matrix switchToArticle = osg::Matrix::identity();
    switchToArticle(1,1) = 0.0;
    switchToArticle(1,2) =-1.0; // y -> -z
    switchToArticle(2,1) = 1.0; // z -> y
    switchToArticle(2,2) = 0.0;
    //switch to the lightspace used in the article
    lightProj = lightProj * switchToArticle;

    osg::Matrix L = lightView * lightProj;

    osg::Vec3d projViewDir = getProjViewDir_ls(L);

    if( getUseLiSPSM() /* && projViewDir.z() < 0*/ ) {
        //do Light Space Perspective shadow mapping
        //rotate the lightspace so that the proj light view always points upwards
        //calculate a frame matrix that uses the projViewDir[light-space] as up vector
        //look(from position, into the direction of the projected direction, with unchanged up-vector)
        lightProj = lightProj *
            osg::Matrix::lookAt( osg::Vec3d(0,0,0), projViewDir, osg::Vec3d(0,1,0) );

        osg::Matrix lispsm = getLispSmMtx( lightView * lightProj );
        lightProj = lightProj * lispsm;
    }

    const osg::Matrix PL = lightView * lightProj;

    osg::BoundingBox bb = _hull.computeBoundingBox( PL );

    osg::Matrix fitToUnitFrustum;
    fitToUnitFrustum.makeOrtho( bb._min[0],  bb._max[0],
                                bb._min[1],  bb._max[1],
                               -bb._max[2], -bb._min[2] );

    //map to unit cube
    lightProj = lightProj * fitToUnitFrustum;

    //coordinate system change for calculations in the article
    osg::Matrix switchToGL = osg::Matrix::identity();
    switchToGL(1,1) =  0.0;
    switchToGL(1,2) =  1.0; // y -> z
    switchToGL(2,1) = -1.0; // z -> -y
    switchToGL(2,2) =  0.0;

    //back to open gl coordinate system y <-> z
    lightProj = lightProj * switchToGL;
    //transform from right handed system into left handed ndc
    lightProj = lightProj * osg::Matrix::scale(1.0,1.0,-1.0);
}

void LightSpacePerspectiveShadowMapAlgorithm::operator()
    ( const osgShadow::ConvexPolyhedron* hullShadowedView,
      const osg::Camera* cameraMain,
      osg::Camera* cameraShadow ) const
{
    lispsm->setHull( *hullShadowedView );
    lispsm->setViewMatrix( cameraMain->getViewMatrix() );
    lispsm->setProjectionMatrix( cameraMain->getViewMatrix() );

#if 1
    osg::Vec3d lightDir = osg::Matrix::transform3x3( osg::Vec3d( 0, 0, -1 ), osg::Matrix::inverse( cameraShadow->getViewMatrix() ) );
    osg::Vec3d eyeDir = osg::Matrix::transform3x3( osg::Vec3d( 0, 0, -1 ), osg::Matrix::inverse( cameraMain->getViewMatrix() ) );

#else

    osg::Vec3d lightDir = osg::Matrix::transform3x3( cameraShadow->getViewMatrix(), osg::Vec3d( 0.0, 0.0, -1.0 ) );
    osg::Vec3d eyeDir = osg::Matrix::transform3x3( cameraMain->getViewMatrix(), osg::Vec3d( 0.0, 0.0, -1.0 ) );

#endif

    lightDir.normalize();
    eyeDir.normalize();

    lispsm->setLightDir(lightDir);


    osg::Matrix &proj = cameraShadow->getProjectionMatrix();
    double l,r,b,t,n,f;
    if( proj.getOrtho( l,r,b,t,n,f ) )
    {
        osg::Vec3d camPosInLightSpace =
            osg::Vec3d( 0, 0, 0 ) *
            osg::Matrix::inverse( cameraMain->getViewMatrix() ) *
            cameraShadow->getViewMatrix() *
            cameraShadow->getProjectionMatrix();
    }

    eyeDir.normalize();

    lispsm->setEyeDir( eyeDir );

    osg::BoundingBox bb =
        hullShadowedView->computeBoundingBox( cameraMain->getViewMatrix() );

    lispsm->setNearDist( -bb.zMax() );
    lispsm->setFarDist( -bb.zMin() );

    lispsm->updateLightMtx
        ( cameraShadow->getViewMatrix(), cameraShadow->getProjectionMatrix() );
}

LightSpacePerspectiveShadowMapAlgorithm::LightSpacePerspectiveShadowMapAlgorithm()
{
    lispsm = new LispSM;
}

LightSpacePerspectiveShadowMapAlgorithm::~LightSpacePerspectiveShadowMapAlgorithm()
{
    delete lispsm;
}


#endif
