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
#include <osg/Transform>
#include <osg/Projection>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/LightSource>
#include <osg/ClipNode>
#include <osg/TexGenNode>
#include <osg/OccluderNode>
#include <osg/OcclusionQueryNode>
#include <osg/Notify>
#include <osg/TexEnv>
#include <osg/AlphaFunc>
#include <osg/LineSegment>
#include <osg/TemplatePrimitiveFunctor>
#include <osg/Geometry>
#include <osg/io_utils>

#include <osgUtil/CullVisitor>

#include <float.h>
#include <algorithm>

#include <osg/Timer>

using namespace osg;
using namespace osgUtil;

inline float MAX_F(float a, float b)
    { return a>b?a:b; }
inline int EQUAL_F(float a, float b)
    { return a == b || fabsf(a-b) <= MAX_F(fabsf(a),fabsf(b))*1e-3f; }


CullVisitor::CullVisitor():
    osg::NodeVisitor(CULL_VISITOR,TRAVERSE_ACTIVE_CHILDREN),
    _currentStateGraph(NULL),
    _currentRenderBin(NULL),
    _computed_znear(FLT_MAX),
    _computed_zfar(-FLT_MAX),
    _traversalOrderNumber(0),
    _currentReuseRenderLeafIndex(0),
    _numberOfEncloseOverrideRenderBinDetails(0)
{
    _identifier = new Identifier;
}

CullVisitor::CullVisitor(const CullVisitor& rhs):
    osg::Object(rhs),
    NodeVisitor(rhs),
    CullStack(rhs),
    _currentStateGraph(NULL),
    _currentRenderBin(NULL),
    _computed_znear(FLT_MAX),
    _computed_zfar(-FLT_MAX),
    _traversalOrderNumber(0),
    _currentReuseRenderLeafIndex(0),
    _numberOfEncloseOverrideRenderBinDetails(0),
    _identifier(rhs._identifier)
{
}

CullVisitor::~CullVisitor()
{
    reset();
}

osg::ref_ptr<CullVisitor>& CullVisitor::prototype()
{
    static osg::ref_ptr<CullVisitor> s_CullVisitor = new CullVisitor;
    return s_CullVisitor;
}

CullVisitor* CullVisitor::create()
{
    return CullVisitor::prototype().valid() ?
           CullVisitor::prototype()->clone() :
           new CullVisitor;
}


void CullVisitor::reset()
{
    //
    // first unref all referenced objects and then empty the containers.
    //

    CullStack::reset();

    _renderBinStack.clear();

    _numberOfEncloseOverrideRenderBinDetails = 0;

    // reset the traversal order number
    _traversalOrderNumber = 0;

    // reset the calculated near far planes.
    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;


    osg::Vec3 lookVector(0.0,0.0,-1.0);

    _bbCornerFar = (lookVector.x()>=0?1:0) |
                   (lookVector.y()>=0?2:0) |
                   (lookVector.z()>=0?4:0);

    _bbCornerNear = (~_bbCornerFar)&7;

    // Only reset the RenderLeaf objects used last frame.
    for(RenderLeafList::iterator itr=_reuseRenderLeafList.begin(),
        iter_end=_reuseRenderLeafList.begin()+_currentReuseRenderLeafIndex;
        itr!=iter_end;
        ++itr)
    {
        (*itr)->reset();
    }

    // reset the resuse lists.
    _currentReuseRenderLeafIndex = 0;

    _nearPlaneCandidateMap.clear();
    _farPlaneCandidateMap.clear();
}

float CullVisitor::getDistanceToEyePoint(const Vec3& pos, bool withLODScale) const
{
    if (withLODScale) return (pos-getEyeLocal()).length()*getLODScale();
    else return (pos-getEyeLocal()).length();
}

float CullVisitor::getDistanceToViewPoint(const Vec3& pos, bool withLODScale) const
{
    if (withLODScale) return (pos-getViewPointLocal()).length()*getLODScale();
    else return (pos-getViewPointLocal()).length();
}

inline CullVisitor::value_type distance(const osg::Vec3& coord,const osg::Matrix& matrix)
{
    return -((CullVisitor::value_type)coord[0]*(CullVisitor::value_type)matrix(0,2)+(CullVisitor::value_type)coord[1]*(CullVisitor::value_type)matrix(1,2)+(CullVisitor::value_type)coord[2]*(CullVisitor::value_type)matrix(2,2)+matrix(3,2));
}

float CullVisitor::getDistanceFromEyePoint(const osg::Vec3& pos, bool withLODScale) const
{
    const Matrix& matrix = *_modelviewStack.back();
    float dist = distance(pos,matrix);

    if (withLODScale) return dist*getLODScale();
    else return dist;
}

void CullVisitor::computeNearPlane()
{
    //OSG_NOTICE<<"CullVisitor::computeNearPlane() _computed_znear="<<_computed_znear<<", _computed_zfar="<<_computed_zfar<<std::endl;
    if (!_nearPlaneCandidateMap.empty())
    {
#if 0
        osg::Timer_t start_t = osg::Timer::instance()->tick();
#endif
        // update near from defferred list of drawables
        unsigned int numTests = 0;
        for(DistanceMatrixDrawableMap::iterator itr=_nearPlaneCandidateMap.begin();
            itr!=_nearPlaneCandidateMap.end() && itr->first<_computed_znear;
            ++itr)
        {
            ++numTests;
            value_type d_near = computeNearestPointInFrustum(itr->second._matrix, itr->second._planes,*(itr->second._drawable));
            //OSG_NOTICE<<"  testing computeNearestPointInFrustum with, drawable"<<itr->second._drawable<<", "<<itr->first<<", d_near = "<<d_near<<std::endl;
            if (d_near<_computed_znear)
            {
                _computed_znear = d_near;
#if 0
                OSG_NOTICE<<"   updating znear to "<<d_near<<std::endl;
#endif
            }
        }

#if 0
        osg::Timer_t end_t = osg::Timer::instance()->tick();
        OSG_NOTICE<<"Took "<<osg::Timer::instance()->delta_m(start_t,end_t)<<"ms to test "<<numTests<<" out of "<<_nearPlaneCandidateMap.size()<<std::endl;
#endif
        _nearPlaneCandidateMap.clear();
    }

    if (!_farPlaneCandidateMap.empty())
    {

        //osg::Timer_t start_t = osg::Timer::instance()->tick();

        // update near from defferred list of drawables
        unsigned int numTests = 0;
        for(DistanceMatrixDrawableMap::reverse_iterator itr=_farPlaneCandidateMap.rbegin();
            itr!=_farPlaneCandidateMap.rend() && itr->first>_computed_zfar;
            ++itr)
        {
            ++numTests;
            // OSG_WARN<<"testing computeFurthestPointInFrustum with d_far = "<<itr->first<<std::endl;
            value_type d_far = computeFurthestPointInFrustum(itr->second._matrix, itr->second._planes,*(itr->second._drawable));
            if (d_far>_computed_zfar)
            {
                _computed_zfar = d_far;
                // OSG_WARN<<"updating zfar to "<<_computed_zfar<<std::endl;
            }
        }

        //osg::Timer_t end_t = osg::Timer::instance()->tick();
        //OSG_NOTICE<<"Took "<<osg::Timer::instance()->delta_m(start_t,end_t)<<"ms to test "<<numTests<<" out of "<<_farPlaneCandidateMap.size()<<std::endl;

        _farPlaneCandidateMap.clear();
    }
#if 0
    OSG_NOTICE<<"  result _computed_znear="<<_computed_znear<<", _computed_zfar="<<_computed_zfar<<std::endl;
#endif
}

void CullVisitor::popProjectionMatrix()
{
    computeNearPlane();

    if (_computeNearFar && _computed_zfar>=_computed_znear)
    {

        //OSG_INFO<<"clamping "<< "znear="<<_computed_znear << " zfar="<<_computed_zfar<<std::endl;


        // adjust the projection matrix so that it encompases the local coords.
        // so it doesn't cull them out.
        osg::Matrix& projection = *_projectionStack.back();

        value_type tmp_znear = _computed_znear;
        value_type tmp_zfar = _computed_zfar;

        clampProjectionMatrix(projection, tmp_znear, tmp_zfar);
    }
    else
    {
        //OSG_INFO<<"Not clamping "<< "znear="<<_computed_znear << " zfar="<<_computed_zfar<<std::endl;
    }

    CullStack::popProjectionMatrix();
}



bool CullVisitor::clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const
{
    return osg::clampProjectionMatrix( projection, znear, zfar, _nearFarRatio );
}

bool CullVisitor::clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const
{
    return osg::clampProjectionMatrix( projection, znear, zfar, _nearFarRatio );
}

template<typename Comparator>
struct ComputeNearFarFunctor
{

    ComputeNearFarFunctor():
        _znear(CullVisitor::value_type(0.0)),
        _planes(0) {}

    void set(CullVisitor::value_type znear, const osg::Matrix& matrix, const osg::Polytope::PlaneList* planes)
    {
        _znear = znear;
        _matrix = matrix;
        _planes = planes;
    }

    typedef std::pair<float, osg::Vec3>  DistancePoint;
    typedef std::vector<DistancePoint>  Polygon;

    Comparator                      _comparator;

    CullVisitor::value_type         _znear;
    osg::Matrix                     _matrix;
    const osg::Polytope::PlaneList* _planes;
    Polygon                         _polygonOriginal;
    Polygon                         _polygonNew;

    Polygon                         _pointCache;

    // Handle Points
    inline void operator() ( const osg::Vec3 &v1, bool)
    {
        CullVisitor::value_type n1 = distance(v1,_matrix);

        // check if point is behind znear, if so discard
        if (_comparator.greaterEqual(n1,_znear))
        {
            //OSG_NOTICE<<"Point beyond znear"<<std::endl;
            return;
        }

        if (n1 < 0.0)
        {
            // OSG_NOTICE<<"Point behind eye point"<<std::endl;
            return;
        }

        // If point is outside any of frustum planes, discard.
        osg::Polytope::PlaneList::const_iterator pitr;
        for(pitr = _planes->begin();
            pitr != _planes->end();
            ++pitr)
        {
            const osg::Plane& plane = *pitr;
            float d1 = plane.distance(v1);

            if (d1<0.0)
            {
                //OSG_NOTICE<<"Point outside frustum "<<d1<<std::endl;
                return;
            }
            //OSG_NOTICE<<"Point ok w.r.t plane "<<d1<<std::endl;
        }

        _znear = n1;
        //OSG_NOTICE<<"Near plane updated "<<_znear<<std::endl;
    }

    // Handle Lines
    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, bool)
    {
        CullVisitor::value_type n1 = distance(v1,_matrix);
        CullVisitor::value_type n2 = distance(v2,_matrix);

        // check if line is totally behind znear, if so discard
        if (_comparator.greaterEqual(n1,_znear) &&
            _comparator.greaterEqual(n2,_znear))
        {
            //OSG_NOTICE<<"Line totally beyond znear"<<std::endl;
            return;
        }

        if (n1 < 0.0 &&
            n2 < 0.0)
        {
            // OSG_NOTICE<<"Line totally behind eye point"<<std::endl;
            return;
        }

        // Check each vertex to each frustum plane.
        osg::Polytope::ClippingMask selector_mask = 0x1;
        osg::Polytope::ClippingMask active_mask = 0x0;

        osg::Polytope::PlaneList::const_iterator pitr;
        for(pitr = _planes->begin();
            pitr != _planes->end();
            ++pitr)
        {
            const osg::Plane& plane = *pitr;
            float d1 = plane.distance(v1);
            float d2 = plane.distance(v2);

            unsigned int numOutside = ((d1<0.0)?1:0) + ((d2<0.0)?1:0);
            if (numOutside==2)
            {
                //OSG_NOTICE<<"Line totally outside frustum "<<d1<<"\t"<<d2<<std::endl;
                return;
            }
            unsigned int numInside = ((d1>=0.0)?1:0) + ((d2>=0.0)?1:0);
            if (numInside<2)
            {
                active_mask = active_mask | selector_mask;
            }

            //OSG_NOTICE<<"Line ok w.r.t plane "<<d1<<"\t"<<d2<<std::endl;

            selector_mask <<= 1;
        }

        if (active_mask==0)
        {
            _znear = minimum(_znear,n1);
            _znear = minimum(_znear,n2);
            // OSG_NOTICE<<"Line all inside frustum "<<n1<<"\t"<<n2<<" number of plane="<<_planes->size()<<std::endl;
            return;
        }

        //OSG_NOTICE<<"Using brute force method of line cutting frustum walls"<<std::endl;
        DistancePoint p1(0, v1);
        DistancePoint p2(0, v2);

        selector_mask = 0x1;

        for(pitr = _planes->begin();
            pitr != _planes->end();
            ++pitr)
        {
            if (active_mask & selector_mask)
            {
                // clip line to plane
                const osg::Plane& plane = *pitr;

                // assign the distance from the current plane.
                p1.first = plane.distance(p1.second);
                p2.first = plane.distance(p2.second);

                if (p1.first >= 0.0f)
                {
                    // p1 is in.
                    if (p2.first < 0.0)
                    {
                        // p2 is out.
                        // replace p2 with intersection
                        float r = p1.first/(p1.first-p2.first);
                        p2 = DistancePoint(0.0f, p1.second*(1.0f-r) + p2.second*r);

                    }
                }
                else if (p2.first >= 0.0f)
                {
                    // p1 is out and p2 is in.
                    // replace p1 with intersection
                    float r = p1.first/(p1.first-p2.first);
                    p1 = DistancePoint(0.0f, p1.second*(1.0f-r) + p2.second*r);
                }
                // The case where both are out was handled above.
            }
            selector_mask <<= 1;
        }

        n1 = distance(p1.second,_matrix);
        n2 = distance(p2.second,_matrix);
        _znear = _comparator.minimum(_znear, n1);
        _znear = _comparator.minimum(_znear, n2);
        //OSG_NOTICE<<"Near plane updated "<<_znear<<std::endl;
    }

    // Handle Triangles
    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool)
    {
        CullVisitor::value_type n1 = distance(v1,_matrix);
        CullVisitor::value_type n2 = distance(v2,_matrix);
        CullVisitor::value_type n3 = distance(v3,_matrix);

        // check if triangle is total behind znear, if so discard
        if (_comparator.greaterEqual(n1,_znear) &&
            _comparator.greaterEqual(n2,_znear) &&
            _comparator.greaterEqual(n3,_znear))
        {
            //OSG_NOTICE<<"Triangle totally beyond znear"<<std::endl;
            return;
        }


        if (n1 < 0.0 &&
            n2 < 0.0 &&
            n3 < 0.0)
        {
            // OSG_NOTICE<<"Triangle totally behind eye point"<<std::endl;
            return;
        }

        // Check each vertex to each frustum plane.
        osg::Polytope::ClippingMask selector_mask = 0x1;
        osg::Polytope::ClippingMask active_mask = 0x0;

        osg::Polytope::PlaneList::const_iterator pitr;
        for(pitr = _planes->begin();
            pitr != _planes->end();
            ++pitr)
        {
            const osg::Plane& plane = *pitr;
            float d1 = plane.distance(v1);
            float d2 = plane.distance(v2);
            float d3 = plane.distance(v3);

            unsigned int numOutside = ((d1<0.0)?1:0) + ((d2<0.0)?1:0) + ((d3<0.0)?1:0);
            if (numOutside==3)
            {
                //OSG_NOTICE<<"Triangle totally outside frustum "<<d1<<"\t"<<d2<<"\t"<<d3<<std::endl;
                return;
            }
            unsigned int numInside = ((d1>=0.0)?1:0) + ((d2>=0.0)?1:0) + ((d3>=0.0)?1:0);
            if (numInside<3)
            {
                active_mask = active_mask | selector_mask;
            }

            //OSG_NOTICE<<"Triangle ok w.r.t plane "<<d1<<"\t"<<d2<<"\t"<<d3<<std::endl;

            selector_mask <<= 1;
        }

        if (active_mask==0)
        {
            _znear = _comparator.minimum(_znear,n1);
            _znear = _comparator.minimum(_znear,n2);
            _znear = _comparator.minimum(_znear,n3);
            // OSG_NOTICE<<"Triangle all inside frustum "<<n1<<"\t"<<n2<<"\t"<<n3<<" number of plane="<<_planes->size()<<std::endl;
            return;
        }

        //return;

        // numPartiallyInside>0) so we have a triangle cutting an frustum wall,
        // this means that use brute force methods for dividing up triangle.

        //OSG_NOTICE<<"Using brute force method of triangle cutting frustum walls"<<std::endl;
        _polygonOriginal.clear();
        _polygonOriginal.push_back(DistancePoint(0,v1));
        _polygonOriginal.push_back(DistancePoint(0,v2));
        _polygonOriginal.push_back(DistancePoint(0,v3));

        selector_mask = 0x1;

        for(pitr = _planes->begin();
            pitr != _planes->end() && !_polygonOriginal.empty();
            ++pitr)
        {
            if (active_mask & selector_mask)
            {
                // polygon bisects plane so need to divide it up.
                const osg::Plane& plane = *pitr;
                _polygonNew.clear();

                // assign the distance from the current plane.
                for(Polygon::iterator polyItr = _polygonOriginal.begin();
                    polyItr != _polygonOriginal.end();
                    ++polyItr)
                {
                    polyItr->first = plane.distance(polyItr->second);
                }

                // create the new polygon by clamping against the
                unsigned int psize = _polygonOriginal.size();

                for(unsigned int ci = 0; ci < psize; ++ci)
                {
                    unsigned int ni = (ci+1)%psize;
                    bool computeIntersection = false;
                    if (_polygonOriginal[ci].first>=0.0f)
                    {
                        _polygonNew.push_back(_polygonOriginal[ci]);

                        if (_polygonOriginal[ni].first<0.0f) computeIntersection = true;
                    }
                    else if (_polygonOriginal[ni].first>0.0f) computeIntersection = true;


                    if (computeIntersection)
                    {
                        // segment intersects with the plane, compute new position.
                        float r = _polygonOriginal[ci].first/(_polygonOriginal[ci].first-_polygonOriginal[ni].first);
                        _polygonNew.push_back(DistancePoint(0.0f,_polygonOriginal[ci].second*(1.0f-r) + _polygonOriginal[ni].second*r));
                    }
                }
                _polygonOriginal.swap(_polygonNew);
            }
            selector_mask <<= 1;
        }

        // now take the nearst points to the eye point.
        for(Polygon::iterator polyItr = _polygonOriginal.begin();
            polyItr != _polygonOriginal.end();
            ++polyItr)
        {
            CullVisitor::value_type dist = distance(polyItr->second,_matrix);
            if (_comparator.less(dist,_znear))
            {
                _znear = dist;
                //OSG_NOTICE<<"Near plane updated "<<_znear<<std::endl;
            }
        }
    }

    // Handle Quadrilaterals
    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, const osg::Vec3 &v4, bool treatVertexDataAsTemporary)
    {
        this->operator()(v1, v2, v3, treatVertexDataAsTemporary);
        this->operator()(v1, v3, v4, treatVertexDataAsTemporary);
    }
};


struct LessComparator
{
    inline bool less(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs<rhs; }
    inline bool lessEqual(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs<=rhs; }
    inline bool greaterEqual(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs>=rhs; }
    inline CullVisitor::value_type minimum(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs<rhs?lhs:rhs; }
};
typedef ComputeNearFarFunctor<LessComparator> ComputeNearestPointFunctor;

struct GreaterComparator
{
    inline bool less(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs>rhs; }
    inline bool lessEqual(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs>=rhs; }
    inline bool greaterEqual(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs<=rhs; }
    inline CullVisitor::value_type minimum(CullVisitor::value_type lhs, CullVisitor::value_type rhs) const { return lhs>rhs?lhs:rhs; }
};
typedef ComputeNearFarFunctor<GreaterComparator> ComputeFurthestPointFunctor;


CullVisitor::value_type CullVisitor::computeNearestPointInFrustum(const osg::Matrix& matrix, const osg::Polytope::PlaneList& planes,const osg::Drawable& drawable)
{
    // OSG_NOTICE<<"CullVisitor::computeNearestPointInFrustum("<<getTraversalNumber()<<"\t"<<planes.size()<<std::endl;

    osg::TemplatePrimitiveFunctor<ComputeNearestPointFunctor> cnpf;
    cnpf.set(FLT_MAX, matrix, &planes);

    drawable.accept(cnpf);

    return cnpf._znear;
}

CullVisitor::value_type CullVisitor::computeFurthestPointInFrustum(const osg::Matrix& matrix, const osg::Polytope::PlaneList& planes,const osg::Drawable& drawable)
{
    //OSG_NOTICE<<"CullVisitor::computeFurthestPointInFrustum("<<getTraversalNumber()<<"\t"<<planes.size()<<")"<<std::endl;

    osg::TemplatePrimitiveFunctor<ComputeFurthestPointFunctor> cnpf;
    cnpf.set(-FLT_MAX, matrix, &planes);

    drawable.accept(cnpf);

    return cnpf._znear;
}

bool CullVisitor::updateCalculatedNearFar(const osg::Matrix& matrix,const osg::BoundingBox& bb)
{
    // efficient computation of near and far, only taking into account the nearest and furthest
    // corners of the bounding box.
    value_type d_near = distance(bb.corner(_bbCornerNear),matrix);
    value_type d_far = distance(bb.corner(_bbCornerFar),matrix);

    if (d_near>d_far)
    {
        std::swap(d_near,d_far);
        if ( !EQUAL_F(d_near, d_far) )
        {
            OSG_WARN<<"Warning: CullVisitor::updateCalculatedNearFar(.) near>far in range calculation,"<< std::endl;
            OSG_WARN<<"         correcting by swapping values d_near="<<d_near<<" dfar="<<d_far<< std::endl;
        }
    }

    if (d_far<0.0)
    {
        // whole object behind the eye point so discard
        return false;
    }

    if (d_near<_computed_znear) _computed_znear = d_near;
    if (d_far>_computed_zfar) _computed_zfar = d_far;

    return true;
}

bool CullVisitor::updateCalculatedNearFar(const osg::Matrix& matrix,const osg::Drawable& drawable, bool isBillboard)
{
    const osg::BoundingBox& bb = drawable.getBoundingBox();

    value_type d_near, d_far;

    if (isBillboard)
    {

#ifdef TIME_BILLBOARD_NEAR_FAR_CALCULATION
        static unsigned int lastFrameNumber = getTraversalNumber();
        static unsigned int numBillboards = 0;
        static double elapsed_time = 0.0;
        if (lastFrameNumber != getTraversalNumber())
        {
            OSG_NOTICE<<"Took "<<elapsed_time<<"ms to test "<<numBillboards<<" billboards"<<std::endl;
            numBillboards = 0;
            elapsed_time = 0.0;
            lastFrameNumber = getTraversalNumber();
        }
        osg::Timer_t start_t = osg::Timer::instance()->tick();
#endif

        osg::Vec3 lookVector(-matrix(0,2),-matrix(1,2),-matrix(2,2));

        unsigned int bbCornerFar = (lookVector.x()>=0?1:0) +
                       (lookVector.y()>=0?2:0) +
                       (lookVector.z()>=0?4:0);

        unsigned int bbCornerNear = (~bbCornerFar)&7;

        d_near = distance(bb.corner(bbCornerNear),matrix);
        d_far = distance(bb.corner(bbCornerFar),matrix);

        OSG_NOTICE.precision(15);

        if (false)
        {

            OSG_NOTICE<<"TESTING Billboard near/far computation"<<std::endl;

             // OSG_WARN<<"Checking corners of billboard "<<std::endl;
            // deprecated brute force way, use all corners of the bounding box.
            value_type nd_near, nd_far;
            nd_near = nd_far = distance(bb.corner(0),matrix);
            for(unsigned int i=0;i<8;++i)
            {
                value_type d = distance(bb.corner(i),matrix);
                if (d<nd_near) nd_near = d;
                if (d>nd_far) nd_far = d;
                OSG_NOTICE<<"\ti="<<i<<"\td="<<d<<std::endl;
            }

            if (nd_near==d_near && nd_far==d_far)
            {
                OSG_NOTICE<<"\tBillboard near/far computation correct "<<std::endl;
            }
            else
            {
                OSG_NOTICE<<"\tBillboard near/far computation ERROR\n\t\t"<<d_near<<"\t"<<nd_near
                                        <<"\n\t\t"<<d_far<<"\t"<<nd_far<<std::endl;
            }

        }

#ifdef TIME_BILLBOARD_NEAR_FAR_CALCULATION
        osg::Timer_t end_t = osg::Timer::instance()->tick();

        elapsed_time += osg::Timer::instance()->delta_m(start_t,end_t);
        ++numBillboards;
#endif
    }
    else
    {
        // efficient computation of near and far, only taking into account the nearest and furthest
        // corners of the bounding box.
        d_near = distance(bb.corner(_bbCornerNear),matrix);
        d_far = distance(bb.corner(_bbCornerFar),matrix);
    }

    if (d_near>d_far)
    {
        std::swap(d_near,d_far);
        if ( !EQUAL_F(d_near, d_far) )
        {
            OSG_WARN<<"Warning: CullVisitor::updateCalculatedNearFar(.) near>far in range calculation,"<< std::endl;
            OSG_WARN<<"         correcting by swapping values d_near="<<d_near<<" dfar="<<d_far<< std::endl;
        }
    }

    if (d_far<0.0)
    {
        // whole object behind the eye point so discard
        return false;
    }

    if (_computeNearFar==COMPUTE_NEAR_FAR_USING_PRIMITIVES || _computeNearFar==COMPUTE_NEAR_USING_PRIMITIVES)
    {
        if (d_near<_computed_znear || d_far>_computed_zfar)
        {
            osg::Polytope& frustum = getCurrentCullingSet().getFrustum();
            if (frustum.getResultMask())
            {
                MatrixPlanesDrawables mpd;
                if (isBillboard)
                {
                    // OSG_WARN<<"Adding billboard into deffered list"<<std::endl;
                    osg::Polytope transformed_frustum;
                    transformed_frustum.setAndTransformProvidingInverse(getProjectionCullingStack().back().getFrustum(),matrix);
                    mpd.set(matrix,&drawable,transformed_frustum);
                }
                else
                {
                    mpd.set(matrix,&drawable,frustum);
                }

                if (d_near<_computed_znear)
                {
                    _nearPlaneCandidateMap.insert(DistanceMatrixDrawableMap::value_type(d_near,mpd) );
                }

                if (_computeNearFar==COMPUTE_NEAR_FAR_USING_PRIMITIVES)
                {
                    if (d_far>_computed_zfar)
                    {
                        _farPlaneCandidateMap.insert(DistanceMatrixDrawableMap::value_type(d_far,mpd) );
                    }
                }

                // use the far point if its nearer than current znear as this is a conservative estimate of the znear
                // while the final computation for this drawable is deferred.
                if (d_far>=0.0 && d_far<_computed_znear)
                {
                    //_computed_znear = d_far;
                }

                if (_computeNearFar==COMPUTE_NEAR_FAR_USING_PRIMITIVES)
                {
                    // use the near point if its further than current zfar as this is a conservative estimate of the zfar
                    // while the final computation for this drawable is deferred.
                    if (d_near>=0.0 && d_near>_computed_zfar)
                    {
                        // _computed_zfar = d_near;
                    }
                }
                else // computing zfar using bounding sphere
                {
                    if (d_far>_computed_zfar) _computed_zfar = d_far;
                }
            }
            else
            {
                if (d_near<_computed_znear) _computed_znear = d_near;
                if (d_far>_computed_zfar) _computed_zfar = d_far;
            }
        }
    }
    else
    {
        if (d_near<_computed_znear) _computed_znear = d_near;
        if (d_far>_computed_zfar) _computed_zfar = d_far;
    }


/*
    // deprecated brute force way, use all corners of the bounding box.
    updateCalculatedNearFar(bb.corner(0));
    updateCalculatedNearFar(bb.corner(1));
    updateCalculatedNearFar(bb.corner(2));
    updateCalculatedNearFar(bb.corner(3));
    updateCalculatedNearFar(bb.corner(4));
    updateCalculatedNearFar(bb.corner(5));
    updateCalculatedNearFar(bb.corner(6));
    updateCalculatedNearFar(bb.corner(7));
*/

    return true;

}
void CullVisitor::updateCalculatedNearFar(const osg::Vec3& pos)
{
    float d;
    if (!_modelviewStack.empty())
    {
        const osg::Matrix& matrix = *(_modelviewStack.back());
        d = distance(pos,matrix);
    }
    else
    {
        d = -pos.z();
    }

    if (d<_computed_znear)
    {
       _computed_znear = d;
       if (d<0.0) OSG_WARN<<"Alerting billboard ="<<d<< std::endl;
    }
    if (d>_computed_zfar) _computed_zfar = d;
}

void CullVisitor::apply(Node& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}


void CullVisitor::apply(Geode& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(osg::Drawable& drawable)
{
    RefMatrix& matrix = *getModelViewMatrix();

    const BoundingBox &bb =drawable.getBoundingBox();

    if( drawable.getCullCallback() )
    {
        osg::DrawableCullCallback* dcb = drawable.getCullCallback()->asDrawableCullCallback();
        if (dcb)
        {
            if( dcb->cull( this, &drawable, &_renderInfo ) == true ) return;
        }
        else
        {
            drawable.getCullCallback()->run(&drawable,this);
        }
    }

    if (drawable.isCullingActive() && isCulled(bb)) return;


    if (_computeNearFar && bb.valid())
    {
        if (!updateCalculatedNearFar(matrix,drawable,false)) return;
    }

    // need to track how push/pops there are, so we can unravel the stack correctly.
    unsigned int numPopStateSetRequired = 0;

    // push the geoset's state on the geostate stack.
    StateSet* stateset = drawable.getStateSet();
    if (stateset)
    {
        ++numPopStateSetRequired;
        pushStateSet(stateset);
    }

    CullingSet& cs = getCurrentCullingSet();
    if (!cs.getStateFrustumList().empty())
    {
        osg::CullingSet::StateFrustumList& sfl = cs.getStateFrustumList();
        for(osg::CullingSet::StateFrustumList::iterator itr = sfl.begin();
            itr != sfl.end();
            ++itr)
        {
            if (itr->second.contains(bb))
            {
                ++numPopStateSetRequired;
                pushStateSet(itr->first.get());
            }
        }
    }

    float depth = bb.valid() ? distance(bb.center(),matrix) : 0.0f;

    if (osg::isNaN(depth))
    {
        OSG_NOTICE<<"CullVisitor::apply(Geode&) detected NaN,"<<std::endl
                                <<"    depth="<<depth<<", center=("<<bb.center()<<"),"<<std::endl
                                <<"    matrix="<<matrix<<std::endl;
        OSG_DEBUG << "    NodePath:" << std::endl;
        for (NodePath::const_iterator i = getNodePath().begin(); i != getNodePath().end(); ++i)
        {
            OSG_DEBUG << "        \"" << (*i)->getName() << "\"" << std::endl;
        }
    }
    else
    {
        addDrawableAndDepth(&drawable,&matrix,depth);
    }

    for(unsigned int i=0;i< numPopStateSetRequired; ++i)
    {
        popStateSet();
    }
}


void CullVisitor::apply(Billboard& node)
{
    if (isCulled(node)) return;

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    // Don't traverse billboard, since drawables are handled manually below
    //handle_cull_callbacks_and_traverse(node);

    const Vec3& eye_local = getEyeLocal();
    const RefMatrix& modelview = *getModelViewMatrix();

    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        const Vec3& pos = node.getPosition(i);

        Drawable* drawable = node.getDrawable(i);
        // need to modify isCulled to handle the billboard offset.
        // if (isCulled(drawable->getBound())) continue;

        if( drawable->getCullCallback() )
        {
            osg::DrawableCullCallback* dcb = drawable->getCullCallback()->asDrawableCullCallback();
            if (dcb && dcb->cull( this, drawable, &_renderInfo ) == true )
                continue;
        }

        RefMatrix* billboard_matrix = createOrReuseMatrix(modelview);

        node.computeMatrix(*billboard_matrix,eye_local,pos);


        if (_computeNearFar && drawable->getBoundingBox().valid()) updateCalculatedNearFar(*billboard_matrix,*drawable,true);
        float depth = distance(pos,modelview);
/*
        if (_computeNearFar)
        {
            if (d<_computed_znear)
            {
                if (d<0.0) OSG_WARN<<"Alerting billboard handling ="<<d<< std::endl;
                _computed_znear = d;
            }
            if (d>_computed_zfar) _computed_zfar = d;
        }
*/
        StateSet* stateset = drawable->getStateSet();
        if (stateset) pushStateSet(stateset);

        if (osg::isNaN(depth))
        {
            OSG_NOTICE<<"CullVisitor::apply(Billboard&) detected NaN,"<<std::endl
                                    <<"    depth="<<depth<<", pos=("<<pos<<"),"<<std::endl
                                    <<"    *billboard_matrix="<<*billboard_matrix<<std::endl;
            OSG_DEBUG << "    NodePath:" << std::endl;
            for (NodePath::const_iterator itr = getNodePath().begin(); itr != getNodePath().end(); ++itr)
            {
                OSG_DEBUG << "        \"" << (*itr)->getName() << "\"" << std::endl;
            }
        }
        else
        {
            addDrawableAndDepth(drawable,billboard_matrix,depth);
        }

        if (stateset) popStateSet();

    }

    // pop the node's state off the geostate stack.
    if (node_state) popStateSet();

}


void CullVisitor::apply(LightSource& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    StateAttribute* light = node.getLight();
    if (light)
    {
        if (node.getReferenceFrame()==osg::LightSource::RELATIVE_RF)
        {
            RefMatrix& matrix = *getModelViewMatrix();
            addPositionedAttribute(&matrix,light);
        }
        else
        {
            // relative to absolute.
            addPositionedAttribute(0,light);
        }
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.
    if (node_state) popStateSet();
}

void CullVisitor::apply(ClipNode& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    RefMatrix& matrix = *getModelViewMatrix();

    const ClipNode::ClipPlaneList& planes = node.getClipPlaneList();
    for(ClipNode::ClipPlaneList::const_iterator itr=planes.begin();
        itr!=planes.end();
        ++itr)
    {
        if (node.getReferenceFrame()==osg::ClipNode::RELATIVE_RF)
        {
            addPositionedAttribute(&matrix,itr->get());
        }
        else
        {
            addPositionedAttribute(0,itr->get());
        }
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.
    if (node_state) popStateSet();
}

void CullVisitor::apply(TexGenNode& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);


    if (node.getReferenceFrame()==osg::TexGenNode::RELATIVE_RF)
    {
        RefMatrix& matrix = *getModelViewMatrix();
        addPositionedTextureAttribute(node.getTextureUnit(), &matrix ,node.getTexGen());
    }
    else
    {
        addPositionedTextureAttribute(node.getTextureUnit(), 0 ,node.getTexGen());
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.
    if (node_state) popStateSet();
}

void CullVisitor::apply(Group& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(Transform& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    RefMatrix* matrix = createOrReuseMatrix(*getModelViewMatrix());
    node.computeLocalToWorldMatrix(*matrix,this);
    pushModelViewMatrix(matrix, node.getReferenceFrame());

    handle_cull_callbacks_and_traverse(node);

    popModelViewMatrix();

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(Projection& node)
{

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);


    // record previous near and far values.
    float previous_znear = _computed_znear;
    float previous_zfar = _computed_zfar;

    // take a copy of the current near plane candidates
    DistanceMatrixDrawableMap  previousNearPlaneCandidateMap;
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

    DistanceMatrixDrawableMap  previousFarPlaneCandidateMap;
    previousFarPlaneCandidateMap.swap(_farPlaneCandidateMap);

    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;


    RefMatrix *matrix = createOrReuseMatrix(node.getMatrix());
    pushProjectionMatrix(matrix);

    //OSG_INFO<<"Push projection "<<*matrix<<std::endl;

    // note do culling check after the frustum has been updated to ensure
    // that the node is not culled prematurely.
    if (!isCulled(node))
    {
        handle_cull_callbacks_and_traverse(node);
    }

    popProjectionMatrix();

    //OSG_INFO<<"Pop projection "<<*matrix<<std::endl;

    _computed_znear = previous_znear;
    _computed_zfar = previous_zfar;

    // swap back the near plane candidates
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);
    previousFarPlaneCandidateMap.swap(_farPlaneCandidateMap);

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(Switch& node)
{
    apply((Group&)node);
}


void CullVisitor::apply(LOD& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(osg::ClearNode& node)
{
    // simply override the current earth sky.
    if (node.getRequiresClear())
    {
      getCurrentRenderBin()->getStage()->setClearColor(node.getClearColor());
      getCurrentRenderBin()->getStage()->setClearMask(node.getClearMask());
    }
    else
    {
      // we have an earth sky implementation to do the work for us
      // so we don't need to clear.
      getCurrentRenderBin()->getStage()->setClearMask(0);
    }

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

}

namespace osgUtil
{

class RenderStageCache : public osg::Object, public osg::Observer
{
    public:

        typedef std::map<osg::Referenced*, osg::ref_ptr<RenderStage> > RenderStageMap;

        RenderStageCache() {}
        RenderStageCache(const RenderStageCache&, const osg::CopyOp&) {}
        virtual ~RenderStageCache()
        {
            for(RenderStageMap::iterator itr = _renderStageMap.begin();
                itr != _renderStageMap.end();
                ++itr)
            {
                itr->first->removeObserver(this);
            }
        }

        META_Object(osgUtil, RenderStageCache);

        virtual void objectDeleted(void* object)
        {
            osg::Referenced* ref = reinterpret_cast<osg::Referenced*>(object);

            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

            RenderStageMap::iterator itr = _renderStageMap.find(ref);
            if (itr!=_renderStageMap.end())
            {
                _renderStageMap.erase(itr);
            }
        }

        void setRenderStage(osg::Referenced* cv, RenderStage* rs)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            RenderStageMap::iterator itr = _renderStageMap.find(cv);
            if (itr==_renderStageMap.end())
            {
                _renderStageMap[cv] = rs;
                cv->addObserver(this);
            }
            else
            {
                itr->second = rs;
            }

        }

        RenderStage* getRenderStage(osg::Referenced* cv)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            RenderStageMap::iterator itr = _renderStageMap.find(cv);
            if (itr!=_renderStageMap.end())
            {
                return itr->second.get();
            }
            else
            {
                return 0;
            }
        }


        /** Resize any per context GLObject buffers to specified size. */
        virtual void resizeGLObjectBuffers(unsigned int maxSize)
        {
            for(RenderStageMap::const_iterator itr = _renderStageMap.begin();
                itr != _renderStageMap.end();
                ++itr)
            {
                itr->second->resizeGLObjectBuffers(maxSize);
            }
        }

        /** If State is non-zero, this function releases any associated OpenGL objects for
           * the specified graphics context. Otherwise, releases OpenGL objexts
           * for all graphics contexts. */
        virtual void releaseGLObjects(osg::State* state= 0) const
        {
            for(RenderStageMap::const_iterator itr = _renderStageMap.begin();
                itr != _renderStageMap.end();
                ++itr)
            {
                itr->second->releaseGLObjects(state);
            }
        }

        OpenThreads::Mutex  _mutex;
        RenderStageMap      _renderStageMap;
};

}

void CullVisitor::apply(osg::Camera& camera)
{
    // push the node's state.
    StateSet* node_state = camera.getStateSet();
    if (node_state) pushStateSet(node_state);

//#define DEBUG_CULLSETTINGS

#ifdef DEBUG_CULLSETTINGS
    if (osg::isNotifyEnabled(osg::NOTICE))
    {
        OSG_NOTICE<<std::endl<<std::endl<<"CullVisitor, before : ";
        write(osg::notify(osg::NOTICE));
    }
#endif

    // Save current cull settings
    CullSettings saved_cull_settings(*this);

#ifdef DEBUG_CULLSETTINGS
    if (osg::isNotifyEnabled(osg::NOTICE))
    {
        OSG_NOTICE<<"CullVisitor, saved_cull_settings : ";
        saved_cull_settings.write(osg::notify(osg::NOTICE));
    }
#endif

#if 1
    // set cull settings from this Camera
    setCullSettings(camera);

#ifdef DEBUG_CULLSETTINGS
    OSG_NOTICE<<"CullVisitor, after setCullSettings(camera) : ";
    write(osg::notify(osg::NOTICE));
#endif
    // inherit the settings from above
    inheritCullSettings(saved_cull_settings, camera.getInheritanceMask());

#ifdef DEBUG_CULLSETTINGS
    OSG_NOTICE<<"CullVisitor, after inheritCullSettings(saved_cull_settings,"<<camera.getInheritanceMask()<<") : ";
    write(osg::notify(osg::NOTICE));
#endif

#else
    // activate all active cull settings from this Camera
    inheritCullSettings(camera);
#endif

    // set the cull mask.
    unsigned int savedTraversalMask = getTraversalMask();
    bool mustSetCullMask = (camera.getInheritanceMask() & osg::CullSettings::CULL_MASK) == 0;
    if (mustSetCullMask) setTraversalMask(camera.getCullMask());

    RefMatrix& originalModelView = *getModelViewMatrix();

    osg::RefMatrix* projection = 0;
    osg::RefMatrix* modelview = 0;

    if (camera.getReferenceFrame()==osg::Transform::RELATIVE_RF)
    {
        if (camera.getTransformOrder()==osg::Camera::POST_MULTIPLY)
        {
            projection = createOrReuseMatrix(*getProjectionMatrix()*camera.getProjectionMatrix());
            modelview = createOrReuseMatrix(*getModelViewMatrix()*camera.getViewMatrix());
        }
        else // pre multiply
        {
            projection = createOrReuseMatrix(camera.getProjectionMatrix()*(*getProjectionMatrix()));
            modelview = createOrReuseMatrix(camera.getViewMatrix()*(*getModelViewMatrix()));
        }
    }
    else
    {
        // an absolute reference frame
        projection = createOrReuseMatrix(camera.getProjectionMatrix());
        modelview = createOrReuseMatrix(camera.getViewMatrix());
    }


    if (camera.getViewport()) pushViewport(camera.getViewport());

    // record previous near and far values.
    value_type previous_znear = _computed_znear;
    value_type previous_zfar = _computed_zfar;

    // take a copy of the current near plane candidates
    DistanceMatrixDrawableMap  previousNearPlaneCandidateMap;
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

    DistanceMatrixDrawableMap  previousFarPlaneCandidateMap;
    previousFarPlaneCandidateMap.swap(_farPlaneCandidateMap);

    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;

    pushProjectionMatrix(projection);
    pushModelViewMatrix(modelview, camera.getReferenceFrame());


    if (camera.getRenderOrder()==osg::Camera::NESTED_RENDER)
    {
        handle_cull_callbacks_and_traverse(camera);
    }
    else
    {
        // set up lighting.
        // currently ignore lights in the scene graph itself..
        // will do later.
        osgUtil::RenderStage* previous_stage = getCurrentRenderBin()->getStage();

        // use render to texture stage.
        // create the render to texture stage.
        osg::ref_ptr<osgUtil::RenderStageCache> rsCache = dynamic_cast<osgUtil::RenderStageCache*>(camera.getRenderingCache());
        if (!rsCache)
        {
            rsCache = new osgUtil::RenderStageCache;
            camera.setRenderingCache(rsCache.get());
        }

        osg::ref_ptr<osgUtil::RenderStage> rtts = rsCache->getRenderStage(this);
        if (!rtts)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*(camera.getDataChangeMutex()));

            rtts = _rootRenderStage.valid() ? osg::cloneType(_rootRenderStage.get()) : new osgUtil::RenderStage;
            rsCache->setRenderStage(this,rtts.get());

            rtts->setCamera(&camera);

            if ( camera.getInheritanceMask() & DRAW_BUFFER )
            {
                // inherit draw buffer from above.
                rtts->setDrawBuffer(previous_stage->getDrawBuffer(),previous_stage->getDrawBufferApplyMask());
            }
            else
            {
                rtts->setDrawBuffer(camera.getDrawBuffer());
            }

            if ( camera.getInheritanceMask() & READ_BUFFER )
            {
                // inherit read buffer from above.
                rtts->setReadBuffer(previous_stage->getReadBuffer(), previous_stage->getReadBufferApplyMask());
            }
            else
            {
                rtts->setReadBuffer(camera.getReadBuffer());
            }
        }
        else
        {
            // reusing render to texture stage, so need to reset it to empty it from previous frames contents.
            rtts->reset();
        }

        // cache the StateGraph and replace with a clone of the existing parental chain.
        osg::ref_ptr<StateGraph> previous_rootStateGraph = _rootStateGraph;
        StateGraph* previous_currentStateGraph = _currentStateGraph;

        // replicate the StageGraph parental chain so that state graph and render leaves are kept local to the Camera's RenderStage.
        {
            typedef std::vector< osg::ref_ptr<StateGraph> > StageGraphStack;
            StageGraphStack stateGraphParentalChain;
            StateGraph* sg = _currentStateGraph;
            while(sg)
            {
                stateGraphParentalChain.push_back(sg);
                sg = sg->_parent;
            }

            _rootStateGraph = rtts->getStateGraph();
            if (_rootStateGraph)
            {
                _rootStateGraph->clean();
            }
            else
            {
                _rootStateGraph = new StateGraph;

                // assign the state graph to the RenderStage to ensure it remains in memory for the draw traversal.
                rtts->setStateGraph(_rootStateGraph.get());
            }
            _currentStateGraph = _rootStateGraph.get();

            StageGraphStack::reverse_iterator ritr = stateGraphParentalChain.rbegin();

            if (ritr!=stateGraphParentalChain.rend())
            {
                const osg::StateSet* ss = (*ritr++)->getStateSet();
                _rootStateGraph->setStateSet(ss);

                while(ritr != stateGraphParentalChain.rend())
                {
                    _currentStateGraph = _currentStateGraph->find_or_insert((*ritr++)->getStateSet());
                }
            }
        }


        // set up clear masks/values
        rtts->setClearDepth(camera.getClearDepth());
        rtts->setClearAccum(camera.getClearAccum());
        rtts->setClearStencil(camera.getClearStencil());
        rtts->setClearMask((camera.getInheritanceMask() & CLEAR_MASK) ? previous_stage->getClearMask() : camera.getClearMask());
        rtts->setClearColor((camera.getInheritanceMask() & CLEAR_COLOR) ? previous_stage->getClearColor() : camera.getClearColor());

        // set the color mask.
        osg::ColorMask* colorMask = camera.getColorMask()!=0 ? camera.getColorMask() : previous_stage->getColorMask();
        rtts->setColorMask(colorMask);

        // set up the viewport.
        osg::Viewport* viewport = camera.getViewport()!=0 ? camera.getViewport() : previous_stage->getViewport();
        rtts->setViewport( viewport );

        // set initial view matrix
        rtts->setInitialViewMatrix(modelview);

        // set up to charge the same PositionalStateContainer is the parent previous stage.
        osg::Matrix inheritedMVtolocalMV;
        inheritedMVtolocalMV.invert(originalModelView);
        inheritedMVtolocalMV.postMult(*getModelViewMatrix());
        rtts->setInheritedPositionalStateContainerMatrix(inheritedMVtolocalMV);
        rtts->setInheritedPositionalStateContainer(previous_stage->getPositionalStateContainer());

        // record the render bin, to be restored after creation
        // of the render to text
        osgUtil::RenderBin* previousRenderBin = getCurrentRenderBin();

        // set the current renderbin to be the newly created stage.
        setCurrentRenderBin(rtts.get());

        // traverse the subgraph
        {
            handle_cull_callbacks_and_traverse(camera);
        }

        // restore the previous renderbin.
        setCurrentRenderBin(previousRenderBin);


        if (rtts->getStateGraphList().size()==0 && rtts->getRenderBinList().size()==0)
        {
            // getting to this point means that all the subgraph has been
            // culled by small feature culling or is beyond LOD ranges.
        }


        // restore cache of the StateGraph
        _rootStateGraph->prune();
        _rootStateGraph = previous_rootStateGraph;
        _currentStateGraph = previous_currentStateGraph;


        // and the render to texture stage to the current stages
        // dependency list.
        switch(camera.getRenderOrder())
        {
            case osg::Camera::PRE_RENDER:
                getCurrentRenderBin()->getStage()->addPreRenderStage(rtts.get(),camera.getRenderOrderNum());
                break;
            default:
                getCurrentRenderBin()->getStage()->addPostRenderStage(rtts.get(),camera.getRenderOrderNum());
                break;
        }

    }

    // restore the previous model view matrix.
    popModelViewMatrix();

    // restore the previous model view matrix.
    popProjectionMatrix();


    // restore the original near and far values
    _computed_znear = previous_znear;
    _computed_zfar = previous_zfar;

    // swap back the near plane candidates
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);
    previousFarPlaneCandidateMap.swap(_farPlaneCandidateMap);


    if (camera.getViewport()) popViewport();

    // restore the previous traversal mask settings
    if (mustSetCullMask) setTraversalMask(savedTraversalMask);

    // restore the previous cull settings
    setCullSettings(saved_cull_settings);

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

}

void CullVisitor::apply(osg::OccluderNode& node)
{
    // need to check if occlusion node is in the occluder
    // list, if so disable the appropriate ShadowOccluderVolume
    disableAndPushOccludersCurrentMask(_nodePath);


    if (isCulled(node))
    {
        popOccludersCurrentMask(_nodePath);
        return;
    }

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);



    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();

    // pop the current mask for the disabled occluder
    popOccludersCurrentMask(_nodePath);
}

void CullVisitor::apply(osg::OcclusionQueryNode& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);


    osg::Camera* camera = getCurrentCamera();

    // If previous query indicates visible, then traverse as usual.
    if (node.getPassed( camera, *this ))
        handle_cull_callbacks_and_traverse(node);

    // Traverse the query subtree if OcclusionQueryNode needs to issue another query.
    node.traverseQuery( camera, *this );

    // Traverse the debug bounding geometry, if enabled.
    node.traverseDebug( *this );


    // pop the node's state off the render graph stack.
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

