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
#include <osgUtil/IntersectVisitor>
#include <osg/Transform>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/Notify>
#include <osg/TriangleFunctor>
#include <osg/Geometry>
#include <osg/Projection>
#include <osg/Camera>
#include <osg/io_utils>

#include <float.h>
#include <algorithm>
#include <map>

using namespace osg;
using namespace osgUtil;



Hit::Hit():
        _ratio(0.0f),
        _primitiveIndex(0)
{
}


Hit::Hit(const Hit& hit)
{
    // copy data across.
    _ratio = hit._ratio;
    _originalLineSegment = hit._originalLineSegment;
    _localLineSegment = hit._localLineSegment;
    _nodePath = hit._nodePath;
    _geode = hit._geode;
    _drawable = hit._drawable;
    _matrix = hit._matrix;
    _inverse = hit._inverse;

    _vecIndexList = hit._vecIndexList;
    _primitiveIndex = hit._primitiveIndex;
    _intersectPoint = hit._intersectPoint;
    _intersectNormal = hit._intersectNormal;
}


Hit::~Hit()
{
}


Hit& Hit::operator = (const Hit& hit)
{
    if (&hit==this) return *this;

    _matrix = hit._matrix;
    _inverse = hit._inverse;
    _originalLineSegment = hit._originalLineSegment;
    _localLineSegment = hit._localLineSegment;

    // copy data across.
    _ratio = hit._ratio;
    _nodePath = hit._nodePath;
    _geode = hit._geode;
    _drawable = hit._drawable;

    _vecIndexList = hit._vecIndexList;
    _primitiveIndex = hit._primitiveIndex;
    _intersectPoint = hit._intersectPoint;
    _intersectNormal = hit._intersectNormal;

    return *this;
}

const osg::Vec3 Hit::getWorldIntersectNormal() const
{
    if (_inverse.valid())
    {
        osg::Vec3 norm = osg::Matrix::transform3x3(*_inverse,_intersectNormal);
        norm.normalize();
        return norm;
    }
    else return _intersectNormal;
}



IntersectVisitor::IntersectState::IntersectState()
{
    _segmentMaskStack.push_back(0xffffffff);
}


IntersectVisitor::IntersectState::~IntersectState()
{
}


bool IntersectVisitor::IntersectState::isCulled(const BoundingSphere& bs,LineSegmentMask& segMaskOut)
{
    bool hit = false;
    LineSegmentMask mask = 0x00000001;
    segMaskOut = 0x00000000;
    LineSegmentMask segMaskIn = _segmentMaskStack.back();
    for(IntersectState::LineSegmentList::iterator sitr=_segList.begin();
        sitr!=_segList.end();
        ++sitr)
    {
        if ((segMaskIn & mask) && (sitr->second)->intersect(bs))
        {
            segMaskOut = segMaskOut| mask;
            hit = true;
        }
        mask = mask << 1;
    }
    return !hit;
}

bool IntersectVisitor::IntersectState::isCulled(const BoundingBox& bb,LineSegmentMask& segMaskOut)
{
    bool hit = false;
    LineSegmentMask mask = 0x00000001;
    segMaskOut = 0x00000000;
    LineSegmentMask segMaskIn = _segmentMaskStack.back();
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

void IntersectVisitor::IntersectState::addLineSegment(osg::LineSegment* seg)
{
    // create a new segment transformed to local coordintes.
    LineSegment* ns = new LineSegment;

    if (_model_inverse.valid())
    {
        if (_view_inverse.valid())
        {
            osg::Matrix matrix = (*(_view_inverse)) * (*(_model_inverse));
            ns->mult(*seg,matrix);
        }
        else
        {
            ns->mult(*seg,*(_model_inverse));
        }
    }
    else if (_view_inverse.valid())
    {
        ns->mult(*seg,*(_view_inverse));
    }
    else
    {
        *ns = *seg;
    }

    _segList.push_back(LineSegmentPair(seg,ns));
}


IntersectVisitor::IntersectVisitor():
    osg::NodeVisitor(osg::NodeVisitor::INTERSECTION_VISITOR, osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
{

    // override the default node visitor mode.
    setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

    // Initialize eyepoint to 0,0,0
    setEyePoint(Vec3(0.0f,0.0f,0.0f));

    setLODSelectionMode(USE_HIGHEST_LEVEL_OF_DETAIL); // original IntersectVisitor behavior
    //setLODSelectionMode(USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION);

    reset();
}


IntersectVisitor::~IntersectVisitor()
{
}


void IntersectVisitor::reset()
{
    _intersectStateStack.clear();

    // create a empty IntersectState on the intersectStateStack.
    _intersectStateStack.push_back(new IntersectState);

    _segHitList.clear();

}

float IntersectVisitor::getDistanceToEyePoint(const Vec3& pos, bool /*withLODScale*/) const
{
    if (_lodSelectionMode==USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION)
    {
        // OSG_NOTICE<<"IntersectVisitor::getDistanceToEyePoint)"<<(pos-getEyePoint()).length()<<std::endl;
        // LODScale is not available to IntersectVisitor, so we ignore the withLODScale argument
        //if (withLODScale) return (pos-getEyePoint()).length()*getLODScale();
        //else return (pos-getEyePoint()).length();
        return (pos-getEyePoint()).length();
    }
    else
    {
        return 0.0f;
    }
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

osg::Vec3 IntersectVisitor::getEyePoint() const
{
    const IntersectState* cis = _intersectStateStack.empty() ? 0 : _intersectStateStack.back().get();
    if (cis && (cis->_model_inverse.valid() || cis->_view_inverse.valid()))
    {

        osg::Vec3 eyePoint = _pseudoEyePoint;
        if (cis->_view_inverse.valid()) eyePoint = eyePoint * (*(cis->_view_inverse));
        if (cis->_model_inverse.valid()) eyePoint = eyePoint * (*(cis->_model_inverse));

        //OSG_NOTICE<<"IntersectVisitor::getEyePoint()"<<eyePoint<<std::endl;

        return eyePoint;
    }
    else
    {
        return _pseudoEyePoint;
    }
}

void IntersectVisitor::addLineSegment(LineSegment* seg)
{
    if (!seg) return;

    if (!seg->valid())
    {
        OSG_WARN<<"Warning: invalid line segment passed to IntersectVisitor::addLineSegment(..)"<<std::endl;
        OSG_WARN<<"         "<<seg->start()<<" "<<seg->end()<<" segment ignored.."<< std::endl;
        return;
    }

    IntersectState* cis = _intersectStateStack.back().get();

    if (cis->_segList.size()>=32)
    {
        OSG_WARN<<"Warning: excessive number of line segmenets passed to IntersectVisitor::addLineSegment(..), maximum permitted is 32 line segments."<<std::endl;
        OSG_WARN<<"         "<<seg->start()<<" "<<seg->end()<<" segment ignored.."<< std::endl;
        return;
    }

    setEyePoint(seg->start()); // set start of line segment to be pseudo EyePoint for billboarding and LOD purposes

    // first check to see if segment has already been added.
    for(IntersectState::LineSegmentList::iterator itr = cis->_segList.begin();
        itr != cis->_segList.end();
        ++itr)
    {
        if (itr->first == seg) return;
    }

    cis->addLineSegment(seg);

}


void IntersectVisitor::pushMatrix(RefMatrix* matrix, osg::Transform::ReferenceFrame rf)
{
    IntersectState* nis = new IntersectState;

    IntersectState* cis = _intersectStateStack.back().get();

    if (rf == osg::Transform::RELATIVE_RF)
    {
        // share the original view matrix
        nis->_view_matrix = cis->_view_matrix;
        nis->_view_inverse = cis->_view_inverse;

        // set up new model matrix
        nis->_model_matrix = matrix;
        if (cis->_model_matrix.valid())
        {
            nis->_model_matrix->postMult(*(cis->_model_matrix));
        }

        RefMatrix* inverse_world = new RefMatrix;
        inverse_world->invert(*(nis->_model_matrix));
        nis->_model_inverse = inverse_world;
    }
    else
    {
        // set a new view matrix
        nis->_view_matrix = matrix;

        RefMatrix* inverse_world = new RefMatrix;
        inverse_world->invert(*(nis->_view_matrix));
        nis->_view_inverse = inverse_world;

        // model matrix now blank.
        nis->_model_matrix = 0;
        nis->_model_inverse = 0;
    }


    IntersectState::LineSegmentMask segMaskIn = cis->_segmentMaskStack.back();
    IntersectState::LineSegmentMask mask = 0x00000001;
    for(IntersectState::LineSegmentList::iterator sitr=cis->_segList.begin();
        sitr!=cis->_segList.end();
        ++sitr)
    {
        if ((segMaskIn & mask))
        {
            nis->addLineSegment(sitr->first.get());
        }
        mask = mask << 1;
    }

    _intersectStateStack.push_back(nis);
}


void IntersectVisitor::popMatrix()
{
    if (!_intersectStateStack.empty())
    {
        _intersectStateStack.pop_back();
    }
}


bool IntersectVisitor::enterNode(Node& node)
{
    const BoundingSphere& bs = node.getBound();
    if (bs.valid() && node.isCullingActive())
    {
        IntersectState* cis = _intersectStateStack.back().get();
        IntersectState::LineSegmentMask sm=0xffffffff;
        if (cis->isCulled(bs,sm)) return false;
        cis->_segmentMaskStack.push_back(sm);
        return true;
    }
    else
    {
        IntersectState* cis = _intersectStateStack.back().get();
        if (!cis->_segmentMaskStack.empty())
            cis->_segmentMaskStack.push_back(cis->_segmentMaskStack.back());
        else
            cis->_segmentMaskStack.push_back(0xffffffff);

        return true;
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

struct TriangleHit
{
    TriangleHit(unsigned int index, const osg::Vec3& normal, float r1, const osg::Vec3* v1, float r2, const osg::Vec3* v2, float r3, const osg::Vec3* v3):
        _index(index),
        _normal(normal),
        _r1(r1),
        _v1(v1),
        _r2(r2),
        _v2(v2),
        _r3(r3),
        _v3(v3) {}

    unsigned int        _index;
    const osg::Vec3     _normal;
    float               _r1;
    const osg::Vec3*    _v1;
    float               _r2;
    const osg::Vec3*    _v2;
    float               _r3;
    const osg::Vec3*    _v3;

protected:

    TriangleHit& operator = (const TriangleHit&) { return *this; }

};


struct TriangleIntersect
{
    osg::ref_ptr<LineSegment> _seg;

    Vec3    _s;
    Vec3    _d;
    float   _length;

    int _index;
    float _ratio;
    bool _hit;



    typedef std::multimap<float,TriangleHit> TriangleHitList;

    TriangleHitList _thl;

    TriangleIntersect():
        _length(0.0f),
        _index(0),
        _ratio(0.0f),
        _hit(false)
    {
    }

    TriangleIntersect(const LineSegment& seg,float ratio=FLT_MAX)
    {
        set(seg,ratio);
    }

    void set(const LineSegment& seg,float ratio=FLT_MAX)
    {
        _seg=new LineSegment(seg);
        _hit=false;
        _index = 0;
        _ratio = ratio;

        _s = _seg->start();
        _d = _seg->end()-_seg->start();
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
        if (!in.valid())
        {
            OSG_WARN<<"Warning:: Picked up error in TriangleIntersect"<<std::endl;
            OSG_WARN<<"   ("<<v1<<",\t"<<v2<<",\t"<<v3<<")"<<std::endl;
            OSG_WARN<<"   ("<<r1<<",\t"<<r2<<",\t"<<r3<<")"<<std::endl;
            return;
        }

        float d = (in-_s)*_d;

        if (d<0.0f) return;
        if (d>_length) return;

        osg::Vec3 normal = v12^v23;
        normal.normalize();

        float r = d/_length;


        _thl.insert(std::pair<const float,TriangleHit>(r,TriangleHit(_index-1,normal,r1,&v1,r2,&v2,r3,&v3)));
        _hit = true;

    }

};

bool IntersectVisitor::intersect(Drawable& drawable)
{
    bool hitFlag = false;

    IntersectState* cis = _intersectStateStack.back().get();

    const BoundingBox& bb = drawable.getBoundingBox();

    for(IntersectState::LineSegmentList::iterator sitr=cis->_segList.begin();
        sitr!=cis->_segList.end();
        ++sitr)
    {
        if (sitr->second->intersect(bb))
        {

            TriangleFunctor<TriangleIntersect> ti;
            ti.set(*sitr->second);
            drawable.accept(ti);
            if (ti._hit)
            {

                osg::Geometry* geometry = drawable.asGeometry();


                for(TriangleIntersect::TriangleHitList::iterator thitr=ti._thl.begin();
                    thitr!=ti._thl.end();
                    ++thitr)
                {

                    Hit hit;
                    hit._nodePath = _nodePath;
                    hit._matrix = cis->_model_matrix;
                    hit._inverse = cis->_model_inverse;
                    hit._drawable = &drawable;
                    if (_nodePath.empty()) hit._geode = NULL;
                    else hit._geode = dynamic_cast<Geode*>(_nodePath.back());

                    TriangleHit& triHit = thitr->second;

                    hit._ratio = thitr->first;
                    hit._primitiveIndex = triHit._index;
                    hit._originalLineSegment = sitr->first;
                    hit._localLineSegment = sitr->second;

                    hit._intersectPoint = sitr->second->start()*(1.0f-hit._ratio)+
                        sitr->second->end()*hit._ratio;

                    hit._intersectNormal = triHit._normal;

                    if (geometry)
                    {
                        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
                        if (vertices)
                        {
                            osg::Vec3* first = &(vertices->front());
                            if (triHit._v1) hit._vecIndexList.push_back(triHit._v1-first);
                            if (triHit._v2) hit._vecIndexList.push_back(triHit._v2-first);
                            if (triHit._v3) hit._vecIndexList.push_back(triHit._v3-first);
                        }
                    }


                    _segHitList[sitr->first.get()].push_back(hit);

                    std::sort(_segHitList[sitr->first.get()].begin(),_segHitList[sitr->first.get()].end());

                    hitFlag = true;

                }
            }
        }
    }

    return hitFlag;

}


void IntersectVisitor::apply(Drawable& drawable)
{
    intersect(drawable);
}

void IntersectVisitor::apply(Geode& geode)
{
    if (!enterNode(geode)) return;

    for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
        intersect(*geode.getDrawable(i));
    }

    leaveNode();
}


void IntersectVisitor::apply(Billboard& node)
{
    if (!enterNode(node)) return;

    // IntersectVisitor doesn't have getEyeLocal(), can we use NodeVisitor::getEyePoint()?
    const Vec3& eye_local = getEyePoint();

    for(unsigned int i = 0; i < node.getNumDrawables(); i++ )
    {
        const Vec3& pos = node.getPosition(i);
        osg::ref_ptr<RefMatrix> billboard_matrix = new RefMatrix;
        node.computeMatrix(*billboard_matrix,eye_local,pos);

        pushMatrix(billboard_matrix.get(), osg::Transform::RELATIVE_RF);

        intersect(*node.getDrawable(i));

        popMatrix();

    }

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

    osg::ref_ptr<RefMatrix> matrix = new RefMatrix;
    node.computeLocalToWorldMatrix(*matrix,this);

    pushMatrix(matrix.get(), node.getReferenceFrame());

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

PickVisitor::PickVisitor(const osg::Viewport* viewport, const osg::Matrixd& proj, const osg::Matrixd& view, float mx, float my):
    _mx(mx),
    _my(my),
    _lastViewport(viewport),
    _lastProjectionMatrix(proj),
    _lastViewMatrix(view)
{
    setLODSelectionMode(USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION);

    if (viewport &&
        mx >= static_cast<float>(viewport->x()) &&
        my >= static_cast<float>(viewport->y()) &&
        mx < static_cast<float>(viewport->x()+viewport->width()) &&
        my < static_cast<float>(viewport->y()+viewport->height()))
    {

        // mouse pointer intersect viewport so we can proceed to set up a line segment
        osg::Matrix MVPW = proj * viewport->computeWindowMatrix();
        osg::Matrixd inverseMVPW;
        inverseMVPW.invert(MVPW);

        osg::Vec3 nearPoint = osg::Vec3(mx,my,0.0f) * inverseMVPW;
        osg::Vec3 farPoint = osg::Vec3(mx,my,1.0f) * inverseMVPW;
        osg::LineSegment* lineSegment = new osg::LineSegment;
        lineSegment->set(nearPoint, farPoint);

        IntersectState* cis = !_intersectStateStack.empty() ? _intersectStateStack.back().get() : 0;
        if (cis)
        {
            cis->_view_matrix = new RefMatrix(view);
            cis->_view_inverse = new RefMatrix;
            cis->_view_inverse->invert(*(cis->_view_matrix));

            cis->_model_matrix = 0;
            cis->_model_inverse = 0;
        }
        else
        {
            OSG_NOTICE<<"Warning: PickVisitor not set up correctly, picking errors likely"<<std::endl;
        }


        addLineSegment(lineSegment);
    }
}

void PickVisitor::runNestedPickVisitor(osg::Node& node, const osg::Viewport* viewport, const osg::Matrix& proj, const osg::Matrix& view, float mx, float my)
{

    PickVisitor newPickVisitor( viewport, proj, view, mx, my );
    newPickVisitor.setTraversalMask(getTraversalMask());

    newPickVisitor.getNodePath() = getNodePath();

    // the new pickvisitor over the nodes children.
    node.traverse( newPickVisitor );

    for(LineSegmentHitListMap::iterator itr = newPickVisitor._segHitList.begin();
        itr != newPickVisitor._segHitList.end();
        ++itr)
    {
        _segHitList.insert(*itr);
    }
}

void PickVisitor::apply(osg::Projection& projection)
{
    runNestedPickVisitor( projection,
                          _lastViewport.get(),
                          projection.getMatrix(),
                          _lastViewMatrix,
                          _mx, _my );
}

void PickVisitor::apply(osg::Camera& camera)
{
    if (!camera.isRenderToTextureCamera())
    {
        if (camera.getReferenceFrame()==osg::Camera::RELATIVE_RF)
        {
            if (camera.getTransformOrder()==osg::Camera::POST_MULTIPLY)
            {
                runNestedPickVisitor( camera,
                                      camera.getViewport() ? camera.getViewport() : _lastViewport.get(),
                                      _lastProjectionMatrix * camera.getProjectionMatrix(),
                                      _lastViewMatrix * camera.getViewMatrix(),
                                      _mx, _my );
            }
            else // PRE_MULTIPLY
            {
                runNestedPickVisitor( camera,
                                      camera.getViewport() ? camera.getViewport() : _lastViewport.get(),
                                      camera.getProjectionMatrix() * _lastProjectionMatrix,
                                      camera.getViewMatrix() * _lastViewMatrix,
                                      _mx, _my );
            }
        }
        else
        {
            runNestedPickVisitor( camera,
                                  camera.getViewport() ? camera.getViewport() : _lastViewport.get(),
                                  camera.getProjectionMatrix(),
                                  camera.getViewMatrix(),
                                  _mx, _my );
        }
    }
}
