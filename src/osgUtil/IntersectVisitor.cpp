#include <osgUtil/IntersectVisitor>
#include <osg/Transform>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/Notify>

#include <float.h>
#include <algorithm>
#include <map>

#ifndef OSG_USE_IO_DOT_H
#include <iostream>
using namespace std;
#endif

using namespace osg;
using namespace osgUtil;

IntersectState::IntersectState()
{
    _matrix = NULL;
    _inverse = NULL;
    _segmentMaskStack.push_back(0xffffffff);
}


IntersectState::~IntersectState()
{
    if (_matrix) _matrix->unref();
    if (_inverse) _inverse->unref();
    for(LineSegmentList::iterator itr=_segList.begin();
        itr!=_segList.end();
        ++itr)
    {
        itr->first->unref();
        itr->second->unref();
    }

    _matrix = (osg::Matrix *)0xffffffff;
    _inverse = (osg::Matrix *)0xffffffff;
}


bool IntersectState::isCulled(const BoundingSphere& bs,LineSegmentmentMask& segMaskOut)
{
    bool hit = false;
    LineSegmentmentMask mask = 0x00000001;
    segMaskOut = 0x00000000;
    LineSegmentmentMask segMaskIn = _segmentMaskStack.back();
    //    notify(INFO) << << "IntersectState::isCulled() mask in "<<segMaskIn<<"  ";
    for(IntersectState::LineSegmentList::iterator sitr=_segList.begin();
        sitr!=_segList.end();
        ++sitr)
    {
        if ((segMaskIn & mask) && (sitr->second)->intersect(bs))
        {
            //            notify(INFO) << << "Hit ";
            segMaskOut = segMaskOut| mask;
            hit = true;
        }
        mask = mask << 1;
    }
    //    notify(INFO) << << "mask = "<<segMaskOut<<endl;
    return !hit;
}


bool IntersectState::isCulled(const BoundingBox& bb,LineSegmentmentMask& segMaskOut)
{
    bool hit = false;
    LineSegmentmentMask mask = 0x00000001;
    segMaskOut = 0x00000000;
    LineSegmentmentMask segMaskIn = _segmentMaskStack.back();
    for(IntersectState::LineSegmentList::iterator sitr=_segList.begin();
        sitr!=_segList.end();
        ++sitr)
    {
        if ((segMaskIn & mask) && (sitr->second)->intersect(bb))
        {
            segMaskOut = segMaskOut| mask;
            hit = true;
        }
        mask = mask << 1;
    }
    return !hit;
}


Hit::Hit()
{
    _originalLineSegment=NULL;
    _localLineSegment=NULL;
    _geode=NULL;
    _geoset=NULL;
    _matrix=NULL;
}


Hit::Hit(const Hit& hit):Referenced()
{
    // copy data across.
    _ratio = hit._ratio;
    _originalLineSegment = hit._originalLineSegment;
    _localLineSegment = hit._localLineSegment;
    _nodePath = hit._nodePath;
    _geode = hit._geode;
    _geoset = hit._geoset;
    _matrix = hit._matrix;

    _vecIndexList = hit._vecIndexList;
    _primitiveIndex = hit._primitiveIndex;
    _intersectPoint = hit._intersectPoint;
    _intersectNormal = hit._intersectNormal;

    if (_matrix) _matrix->ref();
    if (_originalLineSegment) _originalLineSegment->ref();
    if (_localLineSegment) _localLineSegment->ref();
}


Hit::~Hit()
{
    if (_matrix) _matrix->unref();
    if (_originalLineSegment) _originalLineSegment->unref();
    if (_localLineSegment) _localLineSegment->unref();
    _matrix = (osg::Matrix*)0xffffffff;
    _localLineSegment = (osg::LineSegment*)0xffffffff;
    _localLineSegment = (osg::LineSegment*)0xffffffff;
    _geode = (osg::Geode*)0xffffffff;
}


Hit& Hit::operator = (const Hit& hit)
{
    if (&hit==this) return *this;

    // free old memory.
    if (_matrix!=hit._matrix)
    {
        if (_matrix) _matrix->unref();
        _matrix = hit._matrix;
        if (_matrix) _matrix->ref();
    }
    if (_originalLineSegment!=hit._originalLineSegment)
    {
        if (_originalLineSegment) _originalLineSegment->unref();
        _originalLineSegment = hit._originalLineSegment;
        if (_originalLineSegment) _originalLineSegment->ref();
    }
    if (_localLineSegment!=hit._localLineSegment)
    {
        if (_localLineSegment) _localLineSegment->unref();
        _localLineSegment = hit._localLineSegment;
        if (_localLineSegment) _localLineSegment->ref();
    }

    // copy data across.
    _ratio = hit._ratio;
    _originalLineSegment = hit._originalLineSegment;
    _localLineSegment = hit._localLineSegment;
    _nodePath = hit._nodePath;
    _geode = hit._geode;
    _geoset = hit._geoset;

    _vecIndexList = hit._vecIndexList;
    _primitiveIndex = hit._primitiveIndex;
    _intersectPoint = hit._intersectPoint;
    _intersectNormal = hit._intersectNormal;

    return *this;
}


IntersectVisitor::IntersectVisitor()
{
    // overide the default node visitor mode.
    setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
    reset();
}


IntersectVisitor::~IntersectVisitor()
{
}


void IntersectVisitor::reset()
{

    //
    // first unref all referenced objects and then empty the containers.
    //
    _intersectStateStack.clear();

    // create a empty IntersectState on the the intersectStateStack.
    IntersectState* nis = new IntersectState;
    nis->_matrix = NULL;
    nis->_inverse = NULL;

    _intersectStateStack.push_back(nis);

}


bool IntersectVisitor::hits()
{
    for(LineSegmentHitListMap::iterator itr = _segHitList.begin();
        itr != _segHitList.end();
        ++itr)
    {
        if (!(itr->second.empty())) return true;
    }
    return false;
}


void IntersectVisitor::addLineSegment(LineSegment* seg)
{
    if (!seg) return;
    
    if (!seg->valid())
    {
        notify(WARN)<<"Warning: invalid line segment passed to IntersectVisitor::addLineSegment(..), segment ignored.."<<endl;
        return;
    }

    // first check to see if segment has already been added.
    for(LineSegmentHitListMap::iterator itr = _segHitList.begin();
        itr != _segHitList.end();
        ++itr)
    {
        if (itr->first == seg) return;
    }

    // create a new segment transformed to local coordintes.
    IntersectState* cis = _intersectStateStack.back().get();
    LineSegment* ns = new LineSegment;
    if (cis->_inverse) ns->mult(*seg,*(cis->_inverse));
    else *ns = *seg;
    cis->_segList.push_back(std::pair<LineSegment*,LineSegment*>(seg,ns));
    seg->ref();
    ns->ref();

}


void IntersectVisitor::pushMatrix(const Matrix& matrix)
{
    IntersectState* nis = new IntersectState;

    IntersectState* cis = _intersectStateStack.back().get();

    if (cis->_matrix)
    {
        nis->_matrix = new Matrix;
        nis->_matrix->mult(matrix,*(cis->_matrix));
    }
    else
    {
        nis->_matrix = new Matrix(matrix);
    }
    nis->_matrix->ref();

    Matrix* inverse_world = new Matrix;
    inverse_world->ref();
    inverse_world->invert(*(nis->_matrix));
    nis->_inverse = inverse_world;

    IntersectState::LineSegmentmentMask segMaskIn = cis->_segmentMaskStack.back();
    IntersectState::LineSegmentmentMask mask = 0x00000001;
    for(IntersectState::LineSegmentList::iterator sitr=cis->_segList.begin();
        sitr!=cis->_segList.end();
        ++sitr)
    {
        if ((segMaskIn & mask))
        {
            LineSegment* seg = new LineSegment;
            seg->mult(*(sitr->first),*inverse_world);
            nis->_segList.push_back(std::pair<LineSegment*,LineSegment*>(sitr->first,seg));
            seg->ref();
            sitr->first->ref();
        }
        mask = mask << 1;
    }

    _intersectStateStack.push_back(nis);

    //    notify(INFO) << << "IntersectVisitor::pushMatrix()"<<endl;
}


void IntersectVisitor::popMatrix()
{
    if (!_intersectStateStack.empty())
    {
        //        IntersectState* pvs = _intersectStateStack.back().get();
        //        pvs->unref();
        _intersectStateStack.pop_back();
    }
    //    notify(INFO) << << "IntersectVisitor::popMatrix()"<<endl;
}


bool IntersectVisitor::enterNode(Node& node)
{
    const BoundingSphere& bs = node.getBound();
    if (bs.isValid())
    {
        IntersectState* cis = _intersectStateStack.back().get();
        IntersectState::LineSegmentmentMask sm=0xffffffff;
        if (cis->isCulled(bs,sm)) return false;
        cis->_segmentMaskStack.push_back(sm);
        _nodePath.push_back(&node);
        return true;
    }
    else
    {
        return false;
    }
}


void IntersectVisitor::leaveNode()
{
    IntersectState* cis = _intersectStateStack.back().get();
    cis->_segmentMaskStack.pop_back();
}


void IntersectVisitor::apply(Node& node)
{
    if (!enterNode(node)) return;

    traverse(node);

    leaveNode();
}


struct TriangleIntersect
{
    LineSegment _seg;

    Vec3    _s;
    Vec3    _d;
    float   _length;

    int _index;
    float _ratio;
    bool _hit;

    typedef std::multimap<float,std::pair<int,osg::Vec3> > TriangleHitList;
    TriangleHitList _thl;

    TriangleIntersect(const LineSegment& seg,float ratio=FLT_MAX)
    {
        _seg=seg;
        _hit=false;
        _index = 0;
        _ratio = ratio;

        _s = _seg.start();
        _d = _seg.end()-_seg.start();
        _length = _d.length();
        _d /= _length;

    }

    //   bool intersect(const Vec3& v1,const Vec3& v2,const Vec3& v3,float& r)
    inline void operator () (const Vec3& v1,const Vec3& v2,const Vec3& v3)
    {
        ++_index;

        if (v1==v2 || v2==v3 || v1==v3) return;

        Vec3 v12 = v2-v1;
        Vec3 n12 = v12^_d;
        float ds12 = (_s-v1)*n12;
        float d312 = (v3-v1)*n12;
        if (d312>=0.0f)
        {
            if (ds12<0.0f) return;
            if (ds12>d312) return;
        }
        else                     // d312 < 0
        {
            if (ds12>0.0f) return;
            if (ds12<d312) return;
        }

        Vec3 v23 = v3-v2;
        Vec3 n23 = v23^_d;
        float ds23 = (_s-v2)*n23;
        float d123 = (v1-v2)*n23;
        if (d123>=0.0f)
        {
            if (ds23<0.0f) return;
            if (ds23>d123) return;
        }
        else                     // d123 < 0
        {
            if (ds23>0.0f) return;
            if (ds23<d123) return;
        }

        Vec3 v31 = v1-v3;
        Vec3 n31 = v31^_d;
        float ds31 = (_s-v3)*n31;
        float d231 = (v2-v3)*n31;
        if (d231>=0.0f)
        {
            if (ds31<0.0f) return;
            if (ds31>d231) return;
        }
        else                     // d231 < 0
        {
            if (ds31>0.0f) return;
            if (ds31<d231) return;
        }
        

        float r3;
        if (ds12==0.0f) r3=0.0f;
        else if (d312!=0.0f) r3 = ds12/d312;
        else return; // the triangle and the line must be parallel intersection.
        
        float r1;
        if (ds23==0.0f) r1=0.0f;
        else if (d123!=0.0f) r1 = ds23/d123;
        else return; // the triangle and the line must be parallel intersection.
        
        float r2;
        if (ds31==0.0f) r2=0.0f;
        else if (d231!=0.0f) r2 = ds31/d231;
        else return; // the triangle and the line must be parallel intersection.

        float total_r = (r1+r2+r3);
        if (total_r!=1.0f)
        {
            if (total_r==0.0f) return; // the triangle and the line must be parallel intersection.
            float inv_total_r = 1.0f/total_r;
            r1 *= inv_total_r;
            r2 *= inv_total_r;
            r3 *= inv_total_r;
        }
        
        Vec3 in = v1*r1+v2*r2+v3*r3;

        float d = (in-_s)*_d;

        if (d<0.0f) return;
        if (d>_length) return;

        osg::Vec3 normal = v12^v23;
        normal.normalize();

        float r = d/_length;

        _thl.insert(std::pair<const float,std::pair<int,osg::Vec3> >  (r,std::pair<int,osg::Vec3>(_index-1,normal)));
        _hit = true;

    }

};

bool IntersectVisitor::intersect(GeoSet& gset)
{
    bool hitFlag = false;

    IntersectState* cis = _intersectStateStack.back().get();

    const BoundingBox& bb = gset.getBound();

    for(IntersectState::LineSegmentList::iterator sitr=cis->_segList.begin();
        sitr!=cis->_segList.end();
        ++sitr)
    {
        if (sitr->second->intersect(bb))
        {
            TriangleIntersect ti(*sitr->second);
            for_each_triangle(gset,ti);
            if (ti._hit)
            {

                for(TriangleIntersect::TriangleHitList::iterator thitr=ti._thl.begin();
                    thitr!=ti._thl.end();
                    ++thitr)
                {
                    Hit hit;
                    hit._nodePath = _nodePath;
                    hit._matrix = cis->_matrix;
                    if (hit._matrix) hit._matrix->ref();
                    hit._geoset = &gset;
                    if (_nodePath.empty()) hit._geode = NULL;
                    else hit._geode = dynamic_cast<Geode*>(_nodePath.back());

                    hit._ratio = thitr->first;
                    hit._primitiveIndex = thitr->second.first;
                    hit._originalLineSegment = sitr->first;
                    if (hit._originalLineSegment) hit._originalLineSegment->ref();
                    hit._localLineSegment = sitr->second;
                    if (hit._localLineSegment) hit._localLineSegment->ref();

                    hit._intersectPoint = sitr->second->start()*(1.0f-hit._ratio)+
                        sitr->second->end()*hit._ratio;

                    hit._intersectNormal = thitr->second.second;

                    //                    _segHitList[sitr->first].insert(hit);
                    _segHitList[sitr->first].push_back(hit);
                    std::sort(_segHitList[sitr->first].begin(),_segHitList[sitr->first].end());

                    hitFlag = true;

                }
            }
        }
        //        else notify(INFO) << << "no BB hit"<<endl;
    }

    return hitFlag;

}


void IntersectVisitor::apply(Geode& geode)
{
    if (!enterNode(geode)) return;

    for(int i = 0; i < geode.getNumDrawables(); i++ )
    {
        osg::GeoSet* gset = dynamic_cast<osg::GeoSet*>(geode.getDrawable(i));
        if (gset) intersect(*gset);
    }
    
    leaveNode();
}


void IntersectVisitor::apply(Billboard& node)
{
    if (!enterNode(node)) return;
    //     Vec3 eye_local = getEyeLocal();
    //     for(int i=0;i<node.getNumGeosets();++i)
    //     {
    //         Vec3 pos;
    //         node.getPos(i,pos);
    //
    //         GeoSet* gset = node.getGeoSet(i);
    //
    //         Matrix local_mat;
    //         node.calcTransform(eye_local,pos,local_mat);
    //
    //         Matrix* matrix = NULL;
    //         Matrix* currMatrix = getCurrentMatrix();
    //         if (currMatrix)
    //         {
    //             matrix = new Matrix();
    //             matrix->mult(local_mat,*(currMatrix));
    //         }
    //         else
    //         {
    //             matrix = new Matrix(local_mat);
    //         }
    //
    //         matrix->ref();
    //         matrix->unref();
    //
    //     }
    leaveNode();
}


void IntersectVisitor::apply(Group& node)
{
    if (!enterNode(node)) return;

    traverse(node);

    leaveNode();
}


void IntersectVisitor::apply(Transform& node)
{
    if (!enterNode(node)) return;

    pushMatrix(node.getMatrix());

    traverse(node);

    popMatrix();

    leaveNode();
}


void IntersectVisitor::apply(Switch& node)
{
    apply((Group&)node);
}


void IntersectVisitor::apply(LOD& node)
{
    apply((Group&)node);
}
