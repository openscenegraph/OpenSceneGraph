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
#include <osg/TriangleFunctor>
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


class PrintVisitor : public NodeVisitor
{

   public:
   
        PrintVisitor(std::ostream& out):
            NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _out(out)
        {
            _indent = 0;
            _step = 4;
        }
        
        inline void moveIn() { _indent += _step; }
        inline void moveOut() { _indent -= _step; }
        inline void writeIndent() 
        {
            for(int i=0;i<_indent;++i) _out << " ";
        }
                
        virtual void apply(Node& node)
        {
            moveIn();
            writeIndent(); _out << node.className() <<std::endl;
            traverse(node);
            moveOut();
        }

        virtual void apply(Geode& node)         { apply((Node&)node); }
        virtual void apply(Billboard& node)     { apply((Geode&)node); }
        virtual void apply(LightSource& node)   { apply((Group&)node); }
        virtual void apply(ClipNode& node)      { apply((Group&)node); }
        
        virtual void apply(Group& node)         { apply((Node&)node); }
        virtual void apply(Transform& node)     { apply((Group&)node); }
        virtual void apply(Projection& node)    { apply((Group&)node); }
        virtual void apply(Switch& node)        { apply((Group&)node); }
        virtual void apply(LOD& node)           { apply((Group&)node); }

   protected:
   
        PrintVisitor& operator = (const PrintVisitor&) { return *this; }
    
        std::ostream& _out;
        int _indent;
        int _step;
};

CullVisitor::CullVisitor():
    NodeVisitor(CULL_VISITOR,TRAVERSE_ACTIVE_CHILDREN),
    _currentStateGraph(NULL),
    _currentRenderBin(NULL),
    _computed_znear(FLT_MAX),
    _computed_zfar(-FLT_MAX),
    _currentReuseRenderLeafIndex(0),
    _numberOfEncloseOverrideRenderBinDetails(0)
{
}

CullVisitor::CullVisitor(const CullVisitor& rhs):
    NodeVisitor(rhs),
    CullStack(rhs),
    _currentStateGraph(NULL),
    _currentRenderBin(NULL),
    _computed_znear(FLT_MAX),
    _computed_zfar(-FLT_MAX),
    _currentReuseRenderLeafIndex(0),
    _numberOfEncloseOverrideRenderBinDetails(0)
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
    if (!_nearPlaneCandidateMap.empty())
    { 
    
        // osg::Timer_t start_t = osg::Timer::instance()->tick();
        
        // update near from defferred list of drawables
        unsigned int numTests = 0;
        for(DistanceMatrixDrawableMap::iterator itr=_nearPlaneCandidateMap.begin();
            itr!=_nearPlaneCandidateMap.end() && itr->first<_computed_znear;
            ++itr)
        {
            ++numTests;
            // osg::notify(osg::WARN)<<"testing computeNearestPointInFrustum with d_near = "<<itr->first<<std::endl;
            value_type d_near = computeNearestPointInFrustum(itr->second._matrix, itr->second._planes,*(itr->second._drawable));
            if (d_near<_computed_znear)
            {
                _computed_znear = d_near;
                // osg::notify(osg::WARN)<<"updating znear to "<<_computed_znear<<std::endl;
            }
        } 

        // osg::Timer_t end_t = osg::Timer::instance()->tick();
        // osg::notify(osg::NOTICE)<<"Took "<<osg::Timer::instance()->delta_m(start_t,end_t)<<"ms to test "<<numTests<<" out of "<<_nearPlaneCandidateMap.size()<<std::endl;

        _nearPlaneCandidateMap.clear();
    }
}

void CullVisitor::popProjectionMatrix()
{
    computeNearPlane();

    if (_computeNearFar && _computed_zfar>=_computed_znear)
    {

        //osg::notify(osg::INFO)<<"clamping "<< "znear="<<_computed_znear << " zfar="<<_computed_zfar<<std::endl;


        // adjust the projection matrix so that it encompases the local coords.
        // so it doesn't cull them out.
        osg::Matrix& projection = *_projectionStack.back();
        
        value_type tmp_znear = _computed_znear;
        value_type tmp_zfar = _computed_zfar;
        
        clampProjectionMatrix(projection, tmp_znear, tmp_zfar);
    }
    else
    {
        //osg::notify(osg::INFO)<<"Not clamping "<< "znear="<<_computed_znear << " zfar="<<_computed_zfar<<std::endl;
    }

    CullStack::popProjectionMatrix();
}

template<class matrix_type, class value_type>
bool _clampProjectionMatrix(matrix_type& projection, double& znear, double& zfar, value_type nearFarRatio)
{
    double epsilon = 1e-6;
    if (zfar<znear-epsilon)
    {
        osg::notify(osg::INFO)<<"_clampProjectionMatrix not applied, invalid depth range, znear = "<<znear<<"  zfar = "<<zfar<<std::endl;
        return false;
    }
    
    if (zfar<znear+epsilon)
    {
        // znear and zfar are too close together and could cause divide by zero problems
        // late on in the clamping code, so move the znear and zfar apart.
        double average = (znear+zfar)*0.5;
        znear = average-epsilon;
        zfar = average+epsilon;
        // osg::notify(osg::INFO) << "_clampProjectionMatrix widening znear and zfar to "<<znear<<" "<<zfar<<std::endl;
    }

    if (fabs(projection(0,3))<epsilon  && fabs(projection(1,3))<epsilon  && fabs(projection(2,3))<epsilon )
    {
        // osg::notify(osg::INFO) << "Orthographic matrix before clamping"<<projection<<std::endl;

        value_type delta_span = (zfar-znear)*0.02;
        if (delta_span<1.0) delta_span = 1.0;
        value_type desired_znear = znear - delta_span;
        value_type desired_zfar = zfar + delta_span;

        // assign the clamped values back to the computed values.
        znear = desired_znear;
        zfar = desired_zfar;

        projection(2,2)=-2.0f/(desired_zfar-desired_znear);
        projection(3,2)=-(desired_zfar+desired_znear)/(desired_zfar-desired_znear);

        // osg::notify(osg::INFO) << "Orthographic matrix after clamping "<<projection<<std::endl;
    }
    else
    {

        // osg::notify(osg::INFO) << "Persepective matrix before clamping"<<projection<<std::endl;

        //std::cout << "_computed_znear"<<_computed_znear<<std::endl;
        //std::cout << "_computed_zfar"<<_computed_zfar<<std::endl;

        value_type zfarPushRatio = 1.02;
        value_type znearPullRatio = 0.98;

        //znearPullRatio = 0.99; 

        value_type desired_znear = znear * znearPullRatio;
        value_type desired_zfar = zfar * zfarPushRatio;

        // near plane clamping.
        double min_near_plane = zfar*nearFarRatio;
        if (desired_znear<min_near_plane) desired_znear=min_near_plane;

        // assign the clamped values back to the computed values.
        znear = desired_znear;
        zfar = desired_zfar;

        value_type trans_near_plane = (-desired_znear*projection(2,2)+projection(3,2))/(-desired_znear*projection(2,3)+projection(3,3));
        value_type trans_far_plane = (-desired_zfar*projection(2,2)+projection(3,2))/(-desired_zfar*projection(2,3)+projection(3,3));

        value_type ratio = fabs(2.0/(trans_near_plane-trans_far_plane));
        value_type center = -(trans_near_plane+trans_far_plane)/2.0;

        projection.postMult(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                        0.0f,1.0f,0.0f,0.0f,
                                        0.0f,0.0f,ratio,0.0f,
                                        0.0f,0.0f,center*ratio,1.0f));

        // osg::notify(osg::INFO) << "Persepective matrix after clamping"<<projection<<std::endl;
    }
    return true;
}


bool CullVisitor::clampProjectionMatrixImplementation(osg::Matrixf& projection, double& znear, double& zfar) const
{
    return _clampProjectionMatrix( projection, znear, zfar, _nearFarRatio );
}

bool CullVisitor::clampProjectionMatrixImplementation(osg::Matrixd& projection, double& znear, double& zfar) const
{
    return _clampProjectionMatrix( projection, znear, zfar, _nearFarRatio );
}

struct ComputeNearestPointFunctor
{

    ComputeNearestPointFunctor():
        _planes(0) {}

    void set(CullVisitor::value_type znear, const osg::Matrix& matrix, const osg::Polytope::PlaneList* planes)
    {
        _znear = znear;
        _matrix = matrix;
        _planes = planes;
    }

    typedef std::pair<float, osg::Vec3>  DistancePoint;
    typedef std::vector<DistancePoint>  Polygon;

    CullVisitor::value_type         _znear;
    osg::Matrix                     _matrix;
    const osg::Polytope::PlaneList* _planes;
    Polygon                         _polygonOriginal;
    Polygon                         _polygonNew;
    
    Polygon                         _pointCache;

    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool)
    {
        CullVisitor::value_type n1 = distance(v1,_matrix);
        CullVisitor::value_type n2 = distance(v2,_matrix);
        CullVisitor::value_type n3 = distance(v3,_matrix);

        // check if triangle is total behind znear, if so disguard
        if (n1 >= _znear &&
            n2 >= _znear &&
            n3 >= _znear)
        {
            //osg::notify(osg::NOTICE)<<"Triangle totally beyond znear"<<std::endl;
            return;
        }
      
        
        if (n1 < 0.0 &&
            n2 < 0.0 &&
            n3 < 0.0)
        {
            // osg::notify(osg::NOTICE)<<"Triangle totally behind eye point"<<std::endl;
            return;
        }

        // check which planes the points
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
                //osg::notify(osg::NOTICE)<<"Triangle totally outside frustum "<<d1<<"\t"<<d2<<"\t"<<d3<<std::endl;
                return;
            }
            unsigned int numInside = ((d1>=0.0)?1:0) + ((d2>=0.0)?1:0) + ((d3>=0.0)?1:0);
            if (numInside<3)
            {
                active_mask = active_mask | selector_mask;
            }
            
            //osg::notify(osg::NOTICE)<<"Triangle ok w.r.t plane "<<d1<<"\t"<<d2<<"\t"<<d3<<std::endl;

            selector_mask <<= 1; 
        }        
    
        if (active_mask==0)
        {
            _znear = osg::minimum(_znear,n1);
            _znear = osg::minimum(_znear,n2);
            _znear = osg::minimum(_znear,n3);
            // osg::notify(osg::NOTICE)<<"Triangle all inside frustum "<<n1<<"\t"<<n2<<"\t"<<n3<<" number of plane="<<_planes->size()<<std::endl;
            return;
        }
        
        //return;
    
        // numPartiallyInside>0) so we have a triangle cutting an frustum wall,
        // this means that use brute force methods for deviding up triangle. 
        
        //osg::notify(osg::NOTICE)<<"Using brute force method of triangle cutting frustum walls"<<std::endl;
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
            if (dist < _znear) 
            {
                _znear = dist;
                //osg::notify(osg::NOTICE)<<"Near plane updated "<<_znear<<std::endl;
            }
        }
    }
};

CullVisitor::value_type CullVisitor::computeNearestPointInFrustum(const osg::Matrix& matrix, const osg::Polytope::PlaneList& planes,const osg::Drawable& drawable)
{
    // osg::notify(osg::WARN)<<"CullVisitor::computeNearestPointInFrustum("<<getTraversalNumber()<<"\t"<<planes.size()<<std::endl;

    osg::TriangleFunctor<ComputeNearestPointFunctor> cnpf;
    cnpf.set(_computed_znear, matrix, &planes);
    
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
            osg::notify(osg::WARN)<<"Warning: CullVisitor::updateCalculatedNearFar(.) near>far in range calculation,"<< std::endl;
            osg::notify(osg::WARN)<<"         correcting by swapping values d_near="<<d_near<<" dfar="<<d_far<< std::endl;
        }
    }

    if (d_far<0.0)
    {
        // whole object behind the eye point so disguard
        return false;
    }

    if (d_near<_computed_znear) _computed_znear = d_near;
    if (d_far>_computed_zfar) _computed_zfar = d_far;

    return true;
}

bool CullVisitor::updateCalculatedNearFar(const osg::Matrix& matrix,const osg::Drawable& drawable, bool isBillboard)
{
    const osg::BoundingBox& bb = drawable.getBound();

    value_type d_near, d_far;

    if (isBillboard)
    {
    
#ifdef TIME_BILLBOARD_NEAR_FAR_CALCULATION    
        static unsigned int lastFrameNumber = getTraversalNumber();
        static unsigned int numBillboards = 0;
        static double elapsed_time = 0.0;
        if (lastFrameNumber != getTraversalNumber())
        {
            osg::notify(osg::NOTICE)<<"Took "<<elapsed_time<<"ms to test "<<numBillboards<<" billboards"<<std::endl;
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

        osg::notify(osg::NOTICE).precision(15);

        if (false)
        {

            osg::notify(osg::NOTICE)<<"TESTING Billboard near/far computation"<<std::endl;

             // osg::notify(osg::WARN)<<"Checking corners of billboard "<<std::endl;
            // deprecated brute force way, use all corners of the bounding box.
            value_type nd_near, nd_far;
            nd_near = nd_far = distance(bb.corner(0),matrix);
            for(unsigned int i=0;i<8;++i)
            {
                value_type d = distance(bb.corner(i),matrix);
                if (d<nd_near) nd_near = d;
                if (d>nd_far) nd_far = d;
                osg::notify(osg::NOTICE)<<"\ti="<<i<<"\td="<<d<<std::endl;
            }

            if (nd_near==d_near && nd_far==d_far)
            {
                osg::notify(osg::NOTICE)<<"\tBillboard near/far computation correct "<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"\tBillboard near/far computation ERROR\n\t\t"<<d_near<<"\t"<<nd_near
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
            osg::notify(osg::WARN)<<"Warning: CullVisitor::updateCalculatedNearFar(.) near>far in range calculation,"<< std::endl;
            osg::notify(osg::WARN)<<"         correcting by swapping values d_near="<<d_near<<" dfar="<<d_far<< std::endl;
        }
    }

    if (d_far<0.0)
    {
        // whole object behind the eye point so discard
        return false;
    }

    if (d_near<_computed_znear) 
    {
        if (_computeNearFar==COMPUTE_NEAR_FAR_USING_PRIMITIVES)
        {
            osg::Polytope& frustum = getCurrentCullingSet().getFrustum();
            if (frustum.getCurrentMask() && frustum.getResultMask())
            {
                if (isBillboard)
                {
                    // osg::notify(osg::WARN)<<"Adding billboard into deffered list"<<std::endl;
                
                    osg::Polytope transformed_frustum;
                    transformed_frustum.setAndTransformProvidingInverse(getProjectionCullingStack().back().getFrustum(),matrix);
                
                    // insert drawable into the deferred list of drawables which will be handled at the popProjectionMatrix().
                    _nearPlaneCandidateMap.insert(
                        DistanceMatrixDrawableMap::value_type(d_near,MatrixPlanesDrawables(matrix,&drawable,transformed_frustum)) );
                } 
                else
                {
                    // insert drawable into the deferred list of drawables which will be handled at the popProjectionMatrix().
                    _nearPlaneCandidateMap.insert(
                        DistanceMatrixDrawableMap::value_type(d_near,MatrixPlanesDrawables(matrix,&drawable,frustum)) );
                }
            
      
                                    
                // use the far point if its nearer than current znear as this is a conservative estimate of the znear
                // while the final computation for this drawable is deferred.
                if (d_far<_computed_znear)
                {
                    if (d_far<0.0) osg::notify(osg::WARN)<<"       1)  sett near with dnear="<<d_near<<"  dfar="<<d_far<< std::endl;
                    else _computed_znear = d_far;
                }
                
            }
            else
            {
                
                if (d_near<0.0) osg::notify(osg::WARN)<<"        2) sett near with d_near="<<d_near<< std::endl;
                else _computed_znear = d_near;
            }
            
        }
        else
        {
            //if (d_near<0.0) osg::notify(osg::WARN)<<"         3) set near with d_near="<<d_near<< std::endl;
            _computed_znear = d_near;
        }
    }    

    if (d_far>_computed_zfar) _computed_zfar = d_far;


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
       if (d<0.0) osg::notify(osg::WARN)<<"Alerting billboard ="<<d<< std::endl;
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

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    // traverse any call callbacks and traverse any children.
    handle_cull_callbacks_and_traverse(node);

    RefMatrix& matrix = *getModelViewMatrix();
    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        Drawable* drawable = node.getDrawable(i);
        const BoundingBox &bb =drawable->getBound();

        if( drawable->getCullCallback() )
        {
            if( drawable->getCullCallback()->cull( this, drawable, &_renderInfo ) == true )
            continue;
        }
        
        //else
        {
            if (node.isCullingActive() && isCulled(bb)) continue;
        }


        if (_computeNearFar && bb.valid()) 
        {
            if (!updateCalculatedNearFar(matrix,*drawable,false)) continue;
        }

        // need to track how push/pops there are, so we can unravel the stack correctly.
        unsigned int numPopStateSetRequired = 0;

        // push the geoset's state on the geostate stack.    
        StateSet* stateset = drawable->getStateSet();
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
            osg::notify(osg::NOTICE)<<"CullVisitor::apply(Geode&) detected NaN,"<<std::endl
                                    <<"    depth="<<depth<<", center=("<<bb.center()<<"),"<<std::endl
                                    <<"    matrix="<<matrix<<std::endl;
        }
        else
        {        
            addDrawableAndDepth(drawable,&matrix,depth);
        }

        for(unsigned int i=0;i< numPopStateSetRequired; ++i)
        {
            popStateSet();
        }

    }

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();

}


void CullVisitor::apply(Billboard& node)
{
    if (isCulled(node)) return;

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    // traverse any call callbacks and traverse any children.
    handle_cull_callbacks_and_traverse(node);

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
            if( drawable->getCullCallback()->cull( this, drawable, &_renderInfo ) == true )
                continue;
        }

        RefMatrix* billboard_matrix = createOrReuseMatrix(modelview);

        node.computeMatrix(*billboard_matrix,eye_local,pos);


        if (_computeNearFar && drawable->getBound().valid()) updateCalculatedNearFar(*billboard_matrix,*drawable,true);
        float depth = distance(pos,modelview);
/*
        if (_computeNearFar)
        {
            if (d<_computed_znear)
            {
                if (d<0.0) osg::notify(osg::WARN)<<"Alerting billboard handling ="<<d<< std::endl;
                _computed_znear = d;
            }
            if (d>_computed_zfar) _computed_zfar = d;
        }
*/
        StateSet* stateset = drawable->getStateSet();
        if (stateset) pushStateSet(stateset);
        
        if (osg::isNaN(depth))
        {
            osg::notify(osg::NOTICE)<<"CullVisitor::apply(Billboard&) detected NaN,"<<std::endl
                                    <<"    depth="<<depth<<", pos=("<<pos<<"),"<<std::endl
                                    <<"    *billboard_matrix="<<*billboard_matrix<<std::endl;
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

    ref_ptr<RefMatrix> matrix = createOrReuseMatrix(*getModelViewMatrix());
    node.computeLocalToWorldMatrix(*matrix,this);
    pushModelViewMatrix(matrix.get(), node.getReferenceFrame());
    
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

    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;


    ref_ptr<RefMatrix> matrix = createOrReuseMatrix(node.getMatrix());
    pushProjectionMatrix(matrix.get());
    
    //osg::notify(osg::INFO)<<"Push projection "<<*matrix<<std::endl;
    
    // note do culling check after the frustum has been updated to ensure
    // that the node is not culled prematurely.
    if (!isCulled(node))
    {
        handle_cull_callbacks_and_traverse(node);
    }

    popProjectionMatrix();

    //osg::notify(osg::INFO)<<"Pop projection "<<*matrix<<std::endl;

    _computed_znear = previous_znear;
    _computed_zfar = previous_zfar;

    // swap back the near plane candidates
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

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
      // we have an earth sky implementation to do the work for use
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

class RenderStageCache : public osg::Object
{
    public:
    
        RenderStageCache() {}
        RenderStageCache(const RenderStageCache&, const osg::CopyOp&) {}
        
        META_Object(osgUtil, RenderStageCache);
        
        void setRenderStage(CullVisitor* cv, RenderStage* rs)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            _renderStageMap[cv] = rs;
        }        

        RenderStage* getRenderStage(osgUtil::CullVisitor* cv)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            return _renderStageMap[cv].get();
        }
        
        typedef std::map<CullVisitor*, osg::ref_ptr<RenderStage> > RenderStageMap;
        
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
    osg::notify(osg::NOTICE)<<std::endl<<std::endl<<"CullVisitor, before : ";
    write(osg::notify(osg::NOTICE));
#endif

    // Save current cull settings
    CullSettings saved_cull_settings(*this);

#ifdef DEBUG_CULLSETTINGS
    osg::notify(osg::NOTICE)<<"CullVisitor, saved_cull_settings : ";
    saved_cull_settings.write(osg::notify(osg::NOTICE));
#endif

#if 1
    // set cull settings from this Camera
    setCullSettings(camera);

#ifdef DEBUG_CULLSETTINGS
    osg::notify(osg::NOTICE)<<"CullVisitor, after setCullSettings(camera) : ";
    write(osg::notify(osg::NOTICE));
#endif
    // inherit the settings from above
    inheritCullSettings(saved_cull_settings, camera.getInheritanceMask());

#ifdef DEBUG_CULLSETTINGS
    osg::notify(osg::NOTICE)<<"CullVisitor, after inheritCullSettings(saved_cull_settings,"<<camera.getInheritanceMask()<<") : ";
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
    float previous_znear = _computed_znear;
    float previous_zfar = _computed_zfar;
    
    // take a copy of the current near plane candidates
    DistanceMatrixDrawableMap  previousNearPlaneCandidateMap;
    previousNearPlaneCandidateMap.swap(_nearPlaneCandidateMap);

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

//        unsigned int contextID = getState() ? getState()->getContextID() : 0;

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
        
            rtts = new osgUtil::RenderStage;
            rsCache->setRenderStage(this,rtts.get());

            rtts->setCamera(&camera);

            if (camera.getDrawBuffer() != GL_NONE)
            {
                rtts->setDrawBuffer(camera.getDrawBuffer());
            }
            else
            {
                // inherit draw buffer from above.
                rtts->setDrawBuffer(previous_stage->getDrawBuffer());
            }

            if (camera.getReadBuffer() != GL_NONE)
            {
                rtts->setReadBuffer(camera.getReadBuffer());
            }
            else
            {
                // inherit read buffer from above.
                rtts->setReadBuffer(previous_stage->getReadBuffer());
            }

        }
        else
        {
            // reusing render to texture stage, so need to reset it to empty it from previous frames contents.
            rtts->reset();
        }

        // set up clera masks/values        
        rtts->setClearDepth(camera.getClearDepth());
        rtts->setClearAccum(camera.getClearAccum());
        rtts->setClearStencil(camera.getClearStencil());
        rtts->setClearMask(camera.getClearMask());


        // set up the background color and clear mask.
        if (camera.getInheritanceMask() & CLEAR_COLOR)
        {
            rtts->setClearColor(previous_stage->getClearColor());
        }
        else
        {
            rtts->setClearColor(camera.getClearColor());
        }
        
        // set the color mask.
        osg::ColorMask* colorMask = camera.getColorMask()!=0 ? camera.getColorMask() : previous_stage->getColorMask();
        rtts->setColorMask(colorMask);

        // set up the viewport.
        osg::Viewport* viewport = camera.getViewport()!=0 ? camera.getViewport() : previous_stage->getViewport();
        rtts->setViewport( viewport );
        


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


        // and the render to texture stage to the current stages
        // dependancy list.
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

