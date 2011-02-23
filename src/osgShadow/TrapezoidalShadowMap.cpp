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


#include <osgShadow/TrapezoidalShadowMap>
#include <cassert>
#include <iostream>
#include <iomanip>


namespace ConvexHull2D {
//===================================================================
// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.
//===================================================================
/////////////////////////////////////////////////////////////////////
// Wojtek Lewandowski:
// Original Algorithm code adjusted to templates and indices.
// .x, .y, .z fields access changed to indices for usage with OSG
/////////////////////////////////////////////////////////////////////
//===================================================================
// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
//    See: the January 2001 Algorithm on Area of Triangles
//===================================================================
template< typename Point >
inline typename Point::value_type isLeft( Point P0, Point P1, Point P2 )
{
    return (P1[0] - P0[0])*(P2[1] - P0[1]) -
           (P2[0] - P0[0])*(P1[1] - P0[1]);
}
//===================================================================
// chainHull_2D(): Andrew's monotone chain 2D convex hull algorithm
//     Input:  P[] = an array of 2D points
//                   presorted by increasing x- and y-coordinates
//             n = the number of points in P[]
//     Output: H[] = an array of the convex hull vertices (max is n)
//     Return: the number of points in H[]
//===================================================================
template< typename Point >
int chainConvexHull2D( Point* P, int n, Point* H )
{
    // the output array H[] will be used as the stack
    int    bot=0, top=(-1);  // indices for bottom and top of the stack
    int    i;                // array scan index

    // Get the indices of points with min x-coord and min|max y-coord
    int minmin = 0, minmax;
    typename Point::value_type xmin = P[0][0];
    for (i=1; i<n; i++)
        if (P[i][0] != xmin) break;
    minmax = i-1;
    if (minmax == n-1) {       // degenerate case: all x-coords == xmin
        H[++top] = P[minmin];
        if (P[minmax][1] != P[minmin][1]) // a nontrivial segment
            H[++top] = P[minmax];
        H[++top] = P[minmin];           // add polygon endpoint
        return top+1;
    }

    // Get the indices of points with max x-coord and min|max y-coord
    int maxmin, maxmax = n-1;
    typename Point::value_type xmax = P[n-1][0];
    for (i=n-2; i>=0; i--)
        if (P[i][0] != xmax) break;
    maxmin = i+1;

    // Compute the lower hull on the stack H
    H[++top] = P[minmin];      // push minmin point onto stack
    i = minmax;
    while (++i <= maxmin)
    {
        // the lower line joins P[minmin] with P[maxmin]
        if (isLeft( P[minmin], P[maxmin], P[i]) >= 0 && i < maxmin)
            continue;          // ignore P[i] above or on the lower line

        while (top > 0)        // there are at least 2 points on the stack
        {
            // test if P[i] is left of the line at the stack top
            if (isLeft( H[top-1], H[top], P[i]) > 0)
                break;         // P[i] is a new hull vertex
            else
                top--;         // pop top point off stack
        }
        H[++top] = P[i];       // push P[i] onto stack
    }

    // Next, compute the upper hull on the stack H above the bottom hull
    if (maxmax != maxmin)      // if distinct xmax points
        H[++top] = P[maxmax];  // push maxmax point onto stack
    bot = top;                 // the bottom point of the upper hull stack
    i = maxmin;
    while (--i >= minmax)
    {
        // the upper line joins P[maxmax] with P[minmax]
        if (isLeft( P[maxmax], P[minmax], P[i]) >= 0 && i > minmax)
            continue;          // ignore P[i] below or on the upper line

        while (top > bot)    // at least 2 points on the upper stack
        {
            // test if P[i] is left of the line at the stack top
            if (isLeft( H[top-1], H[top], P[i]) > 0)
                break;         // P[i] is a new hull vertex
            else
                top--;         // pop top point off stack
        }
        H[++top] = P[i];       // push P[i] onto stack
    }
    if (minmax != minmin)
        H[++top] = P[minmin];  // push joining endpoint onto stack

    return top+1;
}

} // namespace ConvexHull2D

namespace osgShadow
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TrapezoidalShadowMapAlgorithm
//

////////////////////////////////////////////////////////////////////////////////
struct TrapezoidalMapping : public osg::Referenced
{
    TrapezoidalMapping( unsigned resolution = 2048,
                  float focusDistance  = 1000.f,
                  float focusStart = 0.8f,
                  float focusEnd = 0.0f,
                  float maxNearFarDist = 10000.f,
                  float maxShadowSize  = 1000.f,
                  float scale = 1.f,
                  float duellingFrustaAngle = 15.f,
                  float focusDistanceMin = 50.f,
                  float focusDistanceMax = 10000.f,
                  float focusDistanceStep = 10.f,
                  float focusBase = 1000.f,
                  float frustumNear = -1.f,
                  float frustumFar = 1.f ) :
                        resolution( resolution ),
                        handler( false ),
                        maxNearFarDist( maxNearFarDist ),
                        maxShadowSize( maxShadowSize ),
                        focusDistance( focusDistance ),
                        focusStart( focusStart ),
                        focusEnd( focusEnd ),
                        scale( scale ),
                        duellingFrustaAngle( duellingFrustaAngle ),
                        focusDistanceMin( focusDistanceMin ),
                        focusDistanceMax( focusDistanceMax ),
                        focusDistanceStep( focusDistanceStep ),
                        focusBase( focusBase ),
                        frustumNear( frustumNear ),
                        frustumFar( frustumFar )
    {
    }

    bool AdjustFocus( float distance )
    {
        if( handler ) return false;

        float scaledFocusDistance = FocusDistance();
        float minStepDistance = scaledFocusDistance - focusDistanceStep;
        float maxStepDistance = scaledFocusDistance + focusDistanceStep;

        if( distance < minStepDistance ) distance = minStepDistance;
        if( distance > maxStepDistance ) distance = maxStepDistance;
        if( distance < focusDistanceMin ) distance = focusDistanceMin;
        if( distance > focusDistanceMax ) distance = focusDistanceMax;

        scale = distance / focusBase;
        return true;
    }

    unsigned resolution;
    bool  handler;

    float maxNearFarDist;
    float maxShadowSize;
    float focusDistance;
    float focusStart;
    float focusEnd;
    float scale;
    float duellingFrustaAngle;
    float focusDistanceMin;
    float focusDistanceMax;
    float focusDistanceStep;
    float focusBase;
    float frustumNear;
    float frustumFar;

    float FocusStart() { return focusStart; }
    float FocusEnd() { return focusEnd; }
    float FocusDistance() { return focusDistance * scale; }
    float ShadowRange()  { return maxNearFarDist * scale; }
    float ShadowLength() { return maxShadowSize * scale; }
    unsigned Resolution() { return resolution; }

    void ComputeTrapezoidMapping
            ( osg::Matrix camPostPerspectiveToLightViewSpaceTransform,
              osg::Matrix &projection,
              osg::Matrix &trapezoidalMapping );

    bool ComputeViewFrustumProjection
            ( osg::Matrix camPostPerspectiveToLightViewSpaceTransform,
              osg::Matrix & projection,
              osg::Vec3 polygon[8], int & count,
              osg::Vec3 &top, osg::Vec3 &bottom,
              float &dotLightViewDir );

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TrapezoidalShadowMapAlgorithm
//
bool useIdentity = false;
float fraction = 1.f / 10240;
///////////////////////////////////////////////////////////////////////////////
using namespace osg;
using namespace osgShadow;
///////////////////////////////////////////////////////////////////////////////
// Auxilliary functions
///////////////////////////////////////////////////////////////////////////////
static double
ComputeConvexPolygonArea( Vec3 points[], int count, Matrix * matrix )
{
    double area = 0.0;

    Vec3 av[9];

    if( matrix )
        for( int i = 0; i < count; i++ ) av[i] = points[i] * *matrix;
    else
        for( int i = 0; i < count; i++ ) av[i] = points[i];

    for( Vec3 *p0 = av + count - 1, *p1 = av; count--; p0 = p1, p1++ )
        area += p0->x() * p1->y() - p0->y() * p1->x();

    return abs( area ) * 0.5;
}
////////////////////////////////////////////////////////////////////////////////
// Code taken from TSM Recipe
///////////////////////////////////////////////////////////////////////////////
#define ASSIGN_MAT(M, u0, u3, u6, u1, u4, u7, u2, u5, u8) { \
    M[0][0] = u0; M[0][1] = u3; M[0][2] = u6; \
    M[1][0] = u1; M[1][1] = u4; M[1][2] = u7; \
    M[2][0] = u2; M[2][1] = u5; M[2][2] = u8; \
}
////////////////////////////////////////////////////////////////////////////////
#define DET2(a, b, c, d) ((a) * (d) - (b) * (c))

#define DOT2(u, v) (u[0] * v[0] + u[1] * v[1])
////////////////////////////////////////////////////////////////////////////////
static void intersect
(double i[2], double g0[3], double g1[3], double h0[3], double h1[3])
{
    double a, b;

    i[0] = i[1] =
        1.0f / DET2(g0[0] - g1[0], g0[1] - g1[1], h0[0] - h1[0], h0[1] - h1[1]);

    a = DET2(g0[0], g0[1], g1[0], g1[1]);
    b = DET2(h0[0], h0[1], h1[0], h1[1]);

    i[0] *=    DET2(a, g0[0] - g1[0], b, h0[0] - h1[0]);
    i[1] *=    DET2(a, g0[1] - g1[1], b, h0[1] - h1[1]);
}
////////////////////////////////////////////////////////////////////////////////
static void map_Trapezoid_To_Square
(double TR[3][3], double t0[2], double t1[2], double t2[2], double t3[2])
{
    double i[2], a, b, c, d;

    //M1 = R * T1
    a = 0.5f * (t2[0] - t3[0]);
    b = 0.5f * (t2[1] - t3[1]);

    ASSIGN_MAT(TR, a  ,  b  , a * a + b * b,
        b  , -a  , a * b - b * a,
        0.0f, 0.0f, 1.0f);

    //M2 = T2 * M1 = T2 * R * T1
    intersect(i, t0, t3, t1, t2);

    TR[0][2] = -DOT2(TR[0], i);
    TR[1][2] = -DOT2(TR[1], i);

    //M1 = H * M2 = H * T2 * R * T1
    a = DOT2(TR[0], t2) + TR[0][2];
    b = DOT2(TR[1], t2) + TR[1][2];
    c = DOT2(TR[0], t3) + TR[0][2];
    d = DOT2(TR[1], t3) + TR[1][2];

    a = -(a + c) / (b + d);

    TR[0][0] += TR[1][0] * a;
    TR[0][1] += TR[1][1] * a;
    TR[0][2] += TR[1][2] * a;

    //M2 = S1 * M1 = S1 * H * T2 * R * T1
    a = 1.0f / (DOT2(TR[0], t2) + TR[0][2]);
    b = 1.0f / (DOT2(TR[1], t2) + TR[1][2]);

    TR[0][0] *= a; TR[0][1] *= a; TR[0][2] *= a;
    TR[1][0] *= b; TR[1][1] *= b; TR[1][2] *= b;

    //M1 = N * M2 = N * S1 * H * T2 * R * T1
    TR[2][0] = TR[1][0]; TR[2][1] = TR[1][1]; TR[2][2] = TR[1][2];
    TR[1][2] += 1.0f;

    //M2 = T3 * M1 = T3 * N * S1 * H * T2 * R * T1
    a = DOT2(TR[1], t0) + TR[1][2];
    b = DOT2(TR[2], t0) + TR[2][2];
    c = DOT2(TR[1], t2) + TR[1][2];
    d = DOT2(TR[2], t2) + TR[2][2];

    a = -0.5f * (a / b + c / d);

    TR[1][0] += TR[2][0] * a;
    TR[1][1] += TR[2][1] * a;
    TR[1][2] += TR[2][2] * a;

    //M1 = S2 * M2 = S2 * T3 * N * S1 * H * T2 * R * T1
    a = DOT2(TR[1], t0) + TR[1][2];
    b = DOT2(TR[2], t0) + TR[2][2];

    c = -b / a;

    TR[1][0] *= c; TR[1][1] *= c; TR[1][2] *= c;
}
////////////////////////////////////////////////////////////////////////////////
static osg::Matrix TrapezoidToSquare
( const Vec2 & t0, const Vec2 & t1, const Vec2 &t2, const Vec2 &t3 )
{
    double at[4][3] =
    { {t0[0],t0[1],0},  {t1[0],t1[1],0}, {t2[0],t2[1],0}, {t3[0],t3[1],0} };

    double TR[3][3];
    map_Trapezoid_To_Square( TR, at[0], at[1], at[2], at[3] );

    double N_T[16];
    N_T[0] = TR[0][0]; N_T[4] = TR[0][1]; N_T[ 8] =  0.0f; N_T[12] = TR[0][2];
    N_T[1] = TR[1][0]; N_T[5] = TR[1][1]; N_T[ 9] =  0.0f; N_T[13] = TR[1][2];
    N_T[2] =     0.0f; N_T[6] =     0.0f; N_T[10] =  1.0f; N_T[14] =     0.0f;
    N_T[3] = TR[2][0]; N_T[7] = TR[2][1]; N_T[11] =  0.0f; N_T[15] = TR[2][2];

    osg::Matrix trapezoidalMapping( N_T );

#if 0
    Vec3 check[] = {

        ( Vec3( t0, 0 ) * trapezoidalMapping ),
        ( Vec3( t1, 0 ) * trapezoidalMapping ),
        ( Vec3( t2, 0 ) * trapezoidalMapping ),
        ( Vec3( t3, 0 ) * trapezoidalMapping ),

        ( Vec3( ( t0 + t2 ) * 0.5f, 0 ) * trapezoidalMapping ),
        ( Vec3( ( t1 + t3 ) * 0.5f, 0 ) * trapezoidalMapping )
    };
#endif

    return trapezoidalMapping;
}
////////////////////////////////////////////////////////////////////////////////
static inline bool SortByXY( const Vec3& p1, const Vec3& p2 )
{
    return (p1[0] < p2[0]) || ((p1[0] == p2[0]) && (p1[1] < p2[1]));
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void AlignProjection( Matrix & projection, Vec3 polygon[8],
                      int & count, Vec3 & top, Vec3 & bottom )
{
    Vec3 origin = top * projection;
    Vec3 vertVec = -( bottom * projection - top * projection );

    assert( 0.0 < vertVec.length() );
    vertVec.normalize();
    Vec2 horzVec = Vec2( vertVec[1], -vertVec[0] );

    // Rotation matrix to reorient projection to aligned with vertical axis
    Matrix m( horzVec[0], vertVec[0], 0, 0,
              horzVec[1], vertVec[1], 0, 0,
              0,          0,          1, 0,
              0,          0,          0, 1 );

    osg::BoundingBox bb;
    for( int i = 0; i < count; i++ ) {
        polygon[i] = polygon[i] * m;
        bb.expandBy( polygon[i] );
    }

    projection = projection * m;

    m = Matrix::ortho( bb.xMin(), bb.xMax(), bb.yMin(), bb.yMax(), 1, -1 );

    for( int i = 0; i < count; i++ )
         polygon[i] = polygon[i] * m;

    projection = projection * m;
}
////////////////////////////////////////////////////////////////////////////////
bool TrapezoidalMapping::ComputeViewFrustumProjection
    ( osg::Matrix camPostPerspectiveToLightViewSpaceTransform,
      Matrix & projection, Vec3 polygon[8], int & count, Vec3 & top, Vec3 & bottom,
      float &dotLightViewDir )
{
#if 1 // Shortened frustum
    //float limit = cos( osg::inDegrees( this->duellingFrustaAngle ) );

    Vec3 vecNear = Vec3(0, 0,-1) * camPostPerspectiveToLightViewSpaceTransform;
    Vec3 vecFar  = Vec3(0, 0, 1) * camPostPerspectiveToLightViewSpaceTransform;
    Vec3 vecNearFar = vecFar - vecNear;

    float nearDist = vecNear.length();
    float farDist  = vecFar.length();

    float currNearFarDist = farDist - nearDist;

    // dot( vecNearFar.normal, lighVector ) can be used to check angle between
    // camera dir and light vectors hence find duelling frusta case.
    // Note that light vector in light space is 0,0,1 so
    // dot( vecNearFar.normal, lighVector ) == vecNearFar.z / vecNearFar.length()
    dotLightViewDir = vecNearFar[2] / currNearFarDist;

    // compute new shortened far distance
    vecFar = vecNear + vecNearFar * this->ShadowRange() / currNearFarDist;

    // This is correct because light origin is the same as camera origin

    float n = -1.f, f = n + 2.f * this->ShadowRange() / currNearFarDist;
    f = (vecFar * Matrix::inverse( camPostPerspectiveToLightViewSpaceTransform ))[2];

    this->frustumNear = n;
    this->frustumFar = f;

#if 0
    float dist = ( Vec3( 0, 0, f ) * camPostPerspectiveToLightViewSpaceTransform -
          Vec3( 0, 0, n ) * camPostPerspectiveToLightViewSpaceTransform ).length();
#endif

    Vec3 frustum[8], cube[8] = {
        Vec3( -1,-1,n ), Vec3( -1,1,n ), Vec3( 1,-1,n ), Vec3( 1,1,n ),
        Vec3( -1,-1,f ), Vec3( -1,1,f ), Vec3( 1,-1,f ), Vec3( 1,1,f ),
    };

    BoundingBox bb;
    for( int i = 0; i < 8; i++ ) {
        frustum[i] = cube[i] * camPostPerspectiveToLightViewSpaceTransform;
        bb.expandBy( frustum[i] );
    }

    // Note negative z range - its because when we set positive n & f
    // both perspective or ortho matrix is constructed such that it looks into
    // -n ... -f range

    projection.makeOrtho( bb.xMin(), bb.xMax(),
                          bb.yMin(), bb.yMax(),
                          -( bb.zMax() + this->ShadowLength() ), -bb.zMin() );

    for( int i = 0; i < 8; i++ ) {
        frustum[i] = frustum[i] * projection;
        frustum[i][2] = cube[i][2]; //used later to identify duelling frusta
    }

    std::sort( frustum, frustum+8, SortByXY );
    count = ConvexHull2D::chainConvexHull2D( frustum, 8, cube );

    assert( 0 < count && count <= 9 );

    // We don't need last vertex being the same as first one
    count--;


    // Projected near plane rect fits completely within far rect or opposite
    bool bNoDuellingFrusta = ( count > 4 ||
        cube[0][2] != cube[1][2] ||
        cube[1][2] != cube[2][2] ||
        cube[2][2] != cube[3][2] );
//        fabs( dotLightViewDir ) > limit /*obey degrees limit*/ );

    if( bNoDuellingFrusta ) { // Compute center line through near and far center
        top = Vec3( 0,0,n ) * camPostPerspectiveToLightViewSpaceTransform;
        bottom = Vec3( 0,0,f ) * camPostPerspectiveToLightViewSpaceTransform;
    } else { // Compute center line through far bottom and top center
        float f = cube[0][2]; // near or far plane corresponding to convex hull pts
        top = Vec3( 0,-1, f ) * camPostPerspectiveToLightViewSpaceTransform;
        bottom = Vec3( 0,1, f ) * camPostPerspectiveToLightViewSpaceTransform;
    }

    for( int i = 0; i < count; i++ )
        polygon[i] = cube[i];

    AlignProjection( projection, polygon, count, top, bottom );

    return bNoDuellingFrusta;
#else


#endif
}
////////////////////////////////////////////////////////////////////////////////
void TrapezoidalMapping::ComputeTrapezoidMapping
( osg::Matrix camPostPerspectiveToLightViewSpaceTransform,
  osg::Matrix &projection,
  osg::Matrix &trapezoidalMapping )
{
    float dotLightViewDir;
    Vec3 poly[8], top, bottom;
    int count = 0;

    bool bNoDuellingFrusta = ComputeViewFrustumProjection
        ( camPostPerspectiveToLightViewSpaceTransform,
          projection, poly, count, top, bottom, dotLightViewDir );

    double dfWorldTopBottomDistance = ( bottom - top ).length();

    top = top * projection;
    bottom = bottom * projection;

    Vec3 vertVec3 =  bottom - top;
    Vec2 vertVec( vertVec3.x(), vertVec3.y());

    assert( 0.0 < vertVec.length() );

    double dfLightProjectionSpaceToWorldRatio =
        vertVec.length() / dfWorldTopBottomDistance;

    vertVec.normalize();
    Vec2 horzVec = Vec2( -vertVec[1], vertVec[0] );

    // Compute trapezoid
    int iBottom = 0, iTop = 0, iLeft = 0, iRight = 0;
    Vec2 av[8];

    for( int i = 0; i < count; i++ )
    {
        Vec3 v3 = ( poly[i] - top );
        Vec2 v( v3.x(), v3.y() );
        av[i] = Vec2( horzVec * v, vertVec * v );
        if( av[iTop][1] > av[i][1] ) iTop = i;
        if( av[iBottom][1] < av[i][1] ) iBottom = i;
    }

    if( bNoDuellingFrusta ) {

        // Distance from lowest to furthest point
        double lambda = av[ iBottom ][1] - av[ iTop ][1];
        double stdnear = 1, stdfar = -1;

        // Distance of focus in world units
        double delta = this->FocusDistance() * dfLightProjectionSpaceToWorldRatio;

        // 0..lambda temporary set to 20% of distance from near

        double lastArea = 0.0; // Area of focus
        float focusEnd = dotLightViewDir < 0 ? this->FocusEnd() : this->FocusStart();
         for( double focus = this->FocusStart(), step = 1.0 / this->Resolution();
                                  focus >= focusEnd; focus -= step ) {

            // Point of focus on std unitary frustum (near = 1 > sigma > -1 = far)
            double sigma = ( stdfar - stdnear ) * focus + stdnear; // Eighty percent rule
            // Focal length neccesary to compute trapezoid
            double eta = lambda * delta * ( 1 + sigma ) /
                ( lambda - 2 * delta - lambda * sigma );

#if 0 // Check if computed eta(near) gives projection which returns focus distance (d=sigma)
            double d = -( lambda + 2 * eta ) * ( eta + delta ) + 2 * ( lambda + eta ) * eta;
            d /= lambda * (eta + delta);
#endif

            double dfLeft = av[iTop][0] / eta, dfRight = dfLeft;
            double vertOffset = eta - av[iTop][1];
            for( int i = 0; i < count; i++ ) { // Projective transform uses tangents
                double df = av[i][0] / ( av[i][1] + vertOffset );
                if( dfLeft > df ) dfLeft = df;
                if( dfRight < df ) dfRight = df;
            }

            Vec2 centerTop = Vec2(top.x(),top.y()) + vertVec * av[iTop][1];
            Vec2 centerBottom = Vec2(top.x(),top.y()) + vertVec * av[iBottom][1];

            osg::Matrix m = TrapezoidToSquare
                ( /*lower left*/ centerBottom + horzVec * ( dfLeft * ( eta + lambda ) ),
                /* lower right*/ centerBottom + horzVec * ( dfRight * ( eta + lambda ) ),
                /* upper right */ centerTop + horzVec * ( dfRight * eta ),
                /* upper left */ centerTop + horzVec * ( dfLeft * eta ) );

            double area = ComputeConvexPolygonArea( poly, count, &m );

            // Stop condition for matrix optimization
            // Ignore if Trapezoidal matrix reverts orientation
            // this happens sometimes TrapezoidToSquare does some funky stuff
            if( area <= lastArea || eta <= 0 )
                break;

            trapezoidalMapping = m;
            lastArea = area;
        }

    } else {

        for( int i = 0; i < count; i++ ) {
            if( av[iLeft][0] > av[i][0] ) iLeft = i;
            if( av[iRight][0] < av[i][0] ) iRight = i;
        }

        Vec2 center = Vec2(top.x(),top.y()) + vertVec * ( av[iBottom][1] + av[iTop][1] ) * 0.5f;
        Vec2 vertvec = vertVec * ( av[iTop][1] - av[iBottom][1] ) * 0.5f;
        Vec2 horzvec = horzVec * ( av[iRight][0] - av[iLeft][0] ) * 0.5f;

        trapezoidalMapping.set( horzvec[0], horzvec[1], 0, 0,
            vertvec[0], vertvec[1], 0, 0,
            0,          0,          1, 0,
            center[0],  center[1],  0, 1 );

        trapezoidalMapping.invert( trapezoidalMapping );
#if 0
        {
            float f = fraction, nf = 0.5, n = nf * f;
            Vec3 c =
                Vec3( 0,0,-1 ) * camPostPerspectiveToLightViewSpaceTransform * projection;

            trapezoidalMapping =
                osg::Matrix::scale( osg::Vec3( 1, 1, nf * fraction ) ) *
                osg::Matrix::translate( osg::Vec3( -c[0],-c[1], -f ) ) *
                osg::Matrix::frustum( -( c[0] - -1 )*nf, ( 1 - c[0] )*nf,
                                      -( c[1] - -1 )*nf, ( 1 - c[1] )*nf, n, 2 * f );
        }
#endif
    }

    if( useIdentity )
        trapezoidalMapping.makeIdentity();

    return;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TrapezoidalShadowMapAlgorithm
//
TrapezoidalShadowMapAlgorithm::TrapezoidalShadowMapAlgorithm()
{
    tm = new TrapezoidalMapping;
}

TrapezoidalShadowMapAlgorithm::~TrapezoidalShadowMapAlgorithm()
{
    delete tm;
}

void TrapezoidalShadowMapAlgorithm::operator()
    ( const osgShadow::ConvexPolyhedron* hullShadowedView, 
      const osg::Camera* cameraMain, 
      osg::Camera* cameraShadow ) const
{

    osg::Matrix invCamView = cameraMain->getInverseViewMatrix();
    osg::Matrix view = cameraShadow->getViewMatrix();

    osg::Matrix proj, camInvViewProj, trapezoidMapping;

    camInvViewProj = 
        osg::Matrix::inverse( cameraMain->getProjectionMatrix() ) * invCamView;

    tm->ComputeTrapezoidMapping( camInvViewProj * view, proj, trapezoidMapping );

//    TrapezoidMappingUniform->set( trapezoidMapping );

    cameraShadow->setProjectionMatrix( proj * trapezoidMapping );
}

} // end of osgShadow namespace