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


#include <osgUtil/LineSegmentIntersector>

#include <osg/Geometry>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TriangleFunctor>
#include <osg/KdTree>
#include <osg/Timer>
#include <osg/TexMat>

using namespace osgUtil;

namespace LineSegmentIntersectorUtils
{
    struct TriangleIntersection
    {
        TriangleIntersection(unsigned int index, const osg::Vec3& normal, float r1, const osg::Vec3* v1, float r2, const osg::Vec3* v2, float r3, const osg::Vec3* v3):
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

        TriangleIntersection& operator = (const TriangleIntersection&) { return *this; }
    };

    typedef std::multimap<float,TriangleIntersection> TriangleIntersections;


    template<typename Vec3, typename value_type>
    struct TriangleIntersector
    {
        Vec3   _s;
        Vec3   _d;
        value_type       _length;

        int         _index;
        value_type  _ratio;
        bool        _hit;
        bool        _limitOneIntersection;
        TriangleIntersections* _intersections;

        TriangleIntersector()
        {
            _intersections = 0;
            _length = 0.0f;
            _index = 0;
            _ratio = 0.0f;
            _hit = false;
            _limitOneIntersection = false;
        }

        void set(TriangleIntersections* intersections)
        {
            _intersections = intersections;
        }

        void set(const osg::Vec3d& start, const osg::Vec3d& end, value_type ratio=FLT_MAX)
        {
            _hit=false;
            _index = 0;
            _ratio = ratio;

            _s = start;
            _d = end - start;
            _length = _d.length();
            _d /= _length;
        }

        inline void operator () (const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3, bool treatVertexDataAsTemporary)
        {
            ++_index;

            if (_limitOneIntersection && _hit) return;

            if (v1==v2 || v2==v3 || v1==v3) return;

            Vec3 v12 = v2-v1;
            Vec3 n12 = v12^_d;
            value_type ds12 = (_s-v1)*n12;
            value_type d312 = (v3-v1)*n12;
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
            value_type ds23 = (_s-v2)*n23;
            value_type d123 = (v1-v2)*n23;
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
            value_type ds31 = (_s-v3)*n31;
            value_type d231 = (v2-v3)*n31;
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


            value_type r3;
            if (ds12==0.0f) r3=0.0f;
            else if (d312!=0.0f) r3 = ds12/d312;
            else return; // the triangle and the line must be parallel intersection.

            value_type r1;
            if (ds23==0.0f) r1=0.0f;
            else if (d123!=0.0f) r1 = ds23/d123;
            else return; // the triangle and the line must be parallel intersection.

            value_type r2;
            if (ds31==0.0f) r2=0.0f;
            else if (d231!=0.0f) r2 = ds31/d231;
            else return; // the triangle and the line must be parallel intersection.

            value_type total_r = (r1+r2+r3);
            if (total_r!=1.0f)
            {
                if (total_r==0.0f) return; // the triangle and the line must be parallel intersection.
                value_type inv_total_r = 1.0f/total_r;
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

            value_type d = (in-_s)*_d;

            if (d<0.0f) return;
            if (d>_length) return;

            Vec3 normal = v12^v23;
            normal.normalize();

            value_type r = d/_length;


            if (treatVertexDataAsTemporary)
            {
                _intersections->insert(std::pair<const float,TriangleIntersection>(r,TriangleIntersection(_index-1,normal,r1,0,r2,0,r3,0)));
            }
            else
            {
                _intersections->insert(std::pair<const float,TriangleIntersection>(r,TriangleIntersection(_index-1,normal,r1,&v1,r2,&v2,r3,&v3)));
            }
            _hit = true;

        }

    };

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LineSegmentIntersector
//

LineSegmentIntersector::LineSegmentIntersector(const osg::Vec3d& start, const osg::Vec3d& end):
    _parent(0),
    _start(start),
    _end(end)
{
}

LineSegmentIntersector::LineSegmentIntersector(CoordinateFrame cf, const osg::Vec3d& start, const osg::Vec3d& end,
                                               LineSegmentIntersector* parent, osgUtil::Intersector::IntersectionLimit intersectionLimit):
    Intersector(cf, intersectionLimit),
    _parent(0),
    _start(start),
    _end(end)
{
}

LineSegmentIntersector::LineSegmentIntersector(CoordinateFrame cf, double x, double y):
    Intersector(cf),
    _parent(0)
{
    switch(cf)
    {
        case WINDOW : _start.set(x,y,0.0); _end.set(x,y,1.0); break;
        case PROJECTION : _start.set(x,y,-1.0); _end.set(x,y,1.0); break;
        case VIEW : _start.set(x,y,0.0); _end.set(x,y,1.0); break;
        case MODEL : _start.set(x,y,0.0); _end.set(x,y,1.0); break;
    }
}

Intersector* LineSegmentIntersector::clone(osgUtil::IntersectionVisitor& iv)
{
    if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
    {
        osg::ref_ptr<LineSegmentIntersector> lsi = new LineSegmentIntersector(_start, _end);
        lsi->_parent = this;
        lsi->_intersectionLimit = this->_intersectionLimit;
        lsi->setPrecisionHint(getPrecisionHint());
        return lsi.release();
    }

    // compute the matrix that takes this Intersector from its CoordinateFrame into the local MODEL coordinate frame
    // that geometry in the scene graph will always be in.
    osg::Matrix matrix(getTransformation(iv, _coordinateFrame));

    osg::ref_ptr<LineSegmentIntersector> lsi = new LineSegmentIntersector(_start * matrix, _end * matrix);
    lsi->_parent = this;
    lsi->_intersectionLimit = this->_intersectionLimit;
    lsi->setPrecisionHint(getPrecisionHint());
    return lsi.release();
}

osg::Matrix LineSegmentIntersector::getTransformation(IntersectionVisitor& iv, CoordinateFrame cf)
{
    osg::Matrix matrix;
    switch (cf)
    {
        case(WINDOW):
            if (iv.getWindowMatrix()) matrix.preMult( *iv.getWindowMatrix() );
            if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case(PROJECTION):
            if (iv.getProjectionMatrix()) matrix.preMult( *iv.getProjectionMatrix() );
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case(VIEW):
            if (iv.getViewMatrix()) matrix.preMult( *iv.getViewMatrix() );
            if (iv.getModelMatrix()) matrix.preMult( *iv.getModelMatrix() );
            break;
        case(MODEL):
            if (iv.getModelMatrix()) matrix = *iv.getModelMatrix();
            break;
    }

    osg::Matrix inverse;
    inverse.invert(matrix);
    return inverse;
}

bool LineSegmentIntersector::enter(const osg::Node& node)
{
    if (reachedLimit()) return false;
    return !node.isCullingActive() || intersects( node.getBound() );
}

void LineSegmentIntersector::leave()
{
    // do nothing
}

void LineSegmentIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
    if (reachedLimit()) return;

    osg::Vec3d s(_start), e(_end);
    if ( !intersectAndClip( s, e, drawable->getBoundingBox() ) ) return;

    if (iv.getDoDummyTraversal()) return;

    intersect(iv, drawable, s, e);
}

void LineSegmentIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable,
                                       const osg::Vec3d& s, const osg::Vec3d& e)
{
    osg::KdTree* kdTree = iv.getUseKdTreeWhenAvailable() ? dynamic_cast<osg::KdTree*>(drawable->getShape()) : 0;
    if (kdTree)
    {
        osg::KdTree::LineSegmentIntersections intersections;
        intersections.reserve(4);
        if (kdTree->intersect(s,e,intersections))
        {
            // OSG_NOTICE<<"Got KdTree intersections"<<std::endl;
            for(osg::KdTree::LineSegmentIntersections::iterator itr = intersections.begin();
                itr != intersections.end();
                ++itr)
            {
                osg::KdTree::LineSegmentIntersection& lsi = *(itr);

                // get ratio in s,e range
                double ratio = lsi.ratio;

                // remap ratio into _start, _end range
                double remap_ratio = ((s-_start).length() + ratio * (e-s).length() )/(_end-_start).length();


                Intersection hit;
                hit.ratio = remap_ratio;
                hit.matrix = iv.getModelMatrix();
                hit.nodePath = iv.getNodePath();
                hit.drawable = drawable;
                hit.primitiveIndex = lsi.primitiveIndex;

                hit.localIntersectionPoint = _start*(1.0-remap_ratio) + _end*remap_ratio;

                // OSG_NOTICE<<"KdTree: ratio="<<hit.ratio<<" ("<<hit.localIntersectionPoint<<")"<<std::endl;

                hit.localIntersectionNormal = lsi.intersectionNormal;

                hit.indexList.reserve(3);
                hit.ratioList.reserve(3);
                if (lsi.r0!=0.0f)
                {
                    hit.indexList.push_back(lsi.p0);
                    hit.ratioList.push_back(lsi.r0);
                }

                if (lsi.r1!=0.0f)
                {
                    hit.indexList.push_back(lsi.p1);
                    hit.ratioList.push_back(lsi.r1);
                }

                if (lsi.r2!=0.0f)
                {
                    hit.indexList.push_back(lsi.p2);
                    hit.ratioList.push_back(lsi.r2);
                }

                insertIntersection(hit);
            }
        }

        return;
    }

    LineSegmentIntersectorUtils::TriangleIntersections intersections;

    if (getPrecisionHint()==USE_DOUBLE_CALCULATIONS)
    {
        OSG_INFO<<"Using double intersections"<<std::endl;
        typedef LineSegmentIntersectorUtils::TriangleIntersector<osg::Vec3d, osg::Vec3d::value_type> TriangleIntersector;
        osg::TriangleFunctor< TriangleIntersector > ti;

        ti.set(&intersections);
        ti.set(s,e);
        ti._limitOneIntersection = (_intersectionLimit == LIMIT_ONE_PER_DRAWABLE || _intersectionLimit == LIMIT_ONE);
        drawable->accept(ti);
    }
    else
    {
        OSG_INFO<<"Using float intersections"<<std::endl;
        typedef LineSegmentIntersectorUtils::TriangleIntersector<osg::Vec3f, osg::Vec3f::value_type> TriangleIntersector;
        osg::TriangleFunctor< TriangleIntersector > ti;

        ti.set(&intersections);
        ti.set(s,e);
        ti._limitOneIntersection = (_intersectionLimit == LIMIT_ONE_PER_DRAWABLE || _intersectionLimit == LIMIT_ONE);
        drawable->accept(ti);
    }

    if (!intersections.empty())
    {
        osg::Geometry* geometry = drawable->asGeometry();

        for(LineSegmentIntersectorUtils::TriangleIntersections::iterator thitr = intersections.begin();
            thitr != intersections.end();
            ++thitr)
        {

            // get ratio in s,e range
            double ratio = thitr->first;

            // remap ratio into _start, _end range
            double remap_ratio = ((s-_start).length() + ratio * (e-s).length() )/(_end-_start).length();

            if ( _intersectionLimit == LIMIT_NEAREST && !getIntersections().empty() )
            {
                if (remap_ratio >= getIntersections().begin()->ratio )
                    break;
                else
                    getIntersections().clear();
            }

            LineSegmentIntersectorUtils::TriangleIntersection& triHit = thitr->second;

            Intersection hit;
            hit.ratio = remap_ratio;
            hit.matrix = iv.getModelMatrix();
            hit.nodePath = iv.getNodePath();
            hit.drawable = drawable;
            hit.primitiveIndex = triHit._index;

            hit.localIntersectionPoint = _start*(1.0-remap_ratio) + _end*remap_ratio;

            // OSG_NOTICE<<"Conventional: ratio="<<hit.ratio<<" ("<<hit.localIntersectionPoint<<")"<<std::endl;

            hit.localIntersectionNormal = triHit._normal;

            if (geometry)
            {
                osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
                if (vertices)
                {
                    osg::Vec3* first = &(vertices->front());
                    if (triHit._v1)
                    {
                        hit.indexList.push_back(triHit._v1-first);
                        hit.ratioList.push_back(triHit._r1);
                    }
                    if (triHit._v2)
                    {
                        hit.indexList.push_back(triHit._v2-first);
                        hit.ratioList.push_back(triHit._r2);
                    }
                    if (triHit._v3)
                    {
                        hit.indexList.push_back(triHit._v3-first);
                        hit.ratioList.push_back(triHit._r3);
                    }
                }
            }

            insertIntersection(hit);

        }
    }
}

void LineSegmentIntersector::reset()
{
    Intersector::reset();

    _intersections.clear();
}

bool LineSegmentIntersector::intersects(const osg::BoundingSphere& bs)
{
    // if bs not valid then return true based on the assumption that an invalid sphere is yet to be defined.
    if (!bs.valid()) return true;

    osg::Vec3d sm = _start - bs._center;
    double c = sm.length2()-bs._radius*bs._radius;
    if (c<0.0) return true;

    osg::Vec3d se = _end-_start;
    double a = se.length2();
    double b = (sm*se)*2.0;
    double d = b*b-4.0*a*c;

    if (d<0.0) return false;

    d = sqrt(d);

    double div = 1.0/(2.0*a);

    double r1 = (-b-d)*div;
    double r2 = (-b+d)*div;

    if (r1<=0.0 && r2<=0.0) return false;

    if (r1>=1.0 && r2>=1.0) return false;

    if (_intersectionLimit == LIMIT_NEAREST && !getIntersections().empty())
    {
        double ratio = (sm.length() - bs._radius) / sqrt(a);
        if (ratio >= getIntersections().begin()->ratio) return false;
    }

    // passed all the rejection tests so line must intersect bounding sphere, return true.
    return true;
}

bool LineSegmentIntersector::intersectAndClip(osg::Vec3d& s, osg::Vec3d& e,const osg::BoundingBox& bbInput)
{
    osg::Vec3d bb_min(bbInput._min);
    osg::Vec3d bb_max(bbInput._max);

    double epsilon = 1e-5;

    // compate s and e against the xMin to xMax range of bb.
    if (s.x()<=e.x())
    {
        // trivial reject of segment wholely outside.
        if (e.x()<bb_min.x()) return false;
        if (s.x()>bb_max.x()) return false;

        if (s.x()<bb_min.x())
        {
            // clip s to xMin.
            double r = (bb_min.x()-s.x())/(e.x()-s.x()) - epsilon;
            if (r>0.0) s = s + (e-s)*r;
        }

        if (e.x()>bb_max.x())
        {
            // clip e to xMax.
            double r = (bb_max.x()-s.x())/(e.x()-s.x()) + epsilon;
            if (r<1.0) e = s+(e-s)*r;
        }
    }
    else
    {
        if (s.x()<bb_min.x()) return false;
        if (e.x()>bb_max.x()) return false;

        if (e.x()<bb_min.x())
        {
            // clip e to xMin.
            double r = (bb_min.x()-e.x())/(s.x()-e.x()) - epsilon;
            if (r>0.0) e = e + (s-e)*r;
        }

        if (s.x()>bb_max.x())
        {
            // clip s to xMax.
            double r = (bb_max.x()-e.x())/(s.x()-e.x()) + epsilon;
            if (r<1.0) s = e + (s-e)*r;
        }
    }

    // compate s and e against the yMin to yMax range of bb.
    if (s.y()<=e.y())
    {
        // trivial reject of segment wholely outside.
        if (e.y()<bb_min.y()) return false;
        if (s.y()>bb_max.y()) return false;

        if (s.y()<bb_min.y())
        {
            // clip s to yMin.
            double r = (bb_min.y()-s.y())/(e.y()-s.y()) - epsilon;
            if (r>0.0) s = s + (e-s)*r;
        }

        if (e.y()>bb_max.y())
        {
            // clip e to yMax.
            double r = (bb_max.y()-s.y())/(e.y()-s.y()) + epsilon;
            if (r<1.0) e = s+(e-s)*r;
        }
    }
    else
    {
        if (s.y()<bb_min.y()) return false;
        if (e.y()>bb_max.y()) return false;

        if (e.y()<bb_min.y())
        {
            // clip e to yMin.
            double r = (bb_min.y()-e.y())/(s.y()-e.y()) - epsilon;
            if (r>0.0) e = e + (s-e)*r;
        }

        if (s.y()>bb_max.y())
        {
            // clip s to yMax.
            double r = (bb_max.y()-e.y())/(s.y()-e.y()) + epsilon;
            if (r<1.0) s = e + (s-e)*r;
        }
    }

    // compate s and e against the zMin to zMax range of bb.
    if (s.z()<=e.z())
    {
        // trivial reject of segment wholely outside.
        if (e.z()<bb_min.z()) return false;
        if (s.z()>bb_max.z()) return false;

        if (s.z()<bb_min.z())
        {
            // clip s to zMin.
            double r = (bb_min.z()-s.z())/(e.z()-s.z()) - epsilon;
            if (r>0.0) s = s + (e-s)*r;
        }

        if (e.z()>bb_max.z())
        {
            // clip e to zMax.
            double r = (bb_max.z()-s.z())/(e.z()-s.z()) + epsilon;
            if (r<1.0) e = s+(e-s)*r;
        }
    }
    else
    {
        if (s.z()<bb_min.z()) return false;
        if (e.z()>bb_max.z()) return false;

        if (e.z()<bb_min.z())
        {
            // clip e to zMin.
            double r = (bb_min.z()-e.z())/(s.z()-e.z()) - epsilon;
            if (r>0.0) e = e + (s-e)*r;
        }

        if (s.z()>bb_max.z())
        {
            // clip s to zMax.
            double r = (bb_max.z()-e.z())/(s.z()-e.z()) + epsilon;
            if (r<1.0) s = e + (s-e)*r;
        }
    }

    // OSG_NOTICE<<"clampped segment "<<s<<" "<<e<<std::endl;

    // if (s==e) return false;

    return true;
}

osg::Texture* LineSegmentIntersector::Intersection::getTextureLookUp(osg::Vec3& tc) const
{
    osg::Geometry* geometry = drawable.valid() ? drawable->asGeometry() : 0;
    osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;

    if (vertices)
    {
        if (indexList.size()==3 && ratioList.size()==3)
        {
            unsigned int i1 = indexList[0];
            unsigned int i2 = indexList[1];
            unsigned int i3 = indexList[2];

            float r1 = ratioList[0];
            float r2 = ratioList[1];
            float r3 = ratioList[2];

            osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
            osg::FloatArray* texcoords_FloatArray = dynamic_cast<osg::FloatArray*>(texcoords);
            osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
            osg::Vec3Array* texcoords_Vec3Array = dynamic_cast<osg::Vec3Array*>(texcoords);
            if (texcoords_FloatArray)
            {
                // we have tex coord array so now we can compute the final tex coord at the point of intersection.
                float tc1 = (*texcoords_FloatArray)[i1];
                float tc2 = (*texcoords_FloatArray)[i2];
                float tc3 = (*texcoords_FloatArray)[i3];
                tc.x() = tc1*r1 + tc2*r2 + tc3*r3;
            }
            else if (texcoords_Vec2Array)
            {
                // we have tex coord array so now we can compute the final tex coord at the point of intersection.
                const osg::Vec2& tc1 = (*texcoords_Vec2Array)[i1];
                const osg::Vec2& tc2 = (*texcoords_Vec2Array)[i2];
                const osg::Vec2& tc3 = (*texcoords_Vec2Array)[i3];
                tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
                tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
            }
            else if (texcoords_Vec3Array)
            {
                // we have tex coord array so now we can compute the final tex coord at the point of intersection.
                const osg::Vec3& tc1 = (*texcoords_Vec3Array)[i1];
                const osg::Vec3& tc2 = (*texcoords_Vec3Array)[i2];
                const osg::Vec3& tc3 = (*texcoords_Vec3Array)[i3];
                tc.x() = tc1.x()*r1 + tc2.x()*r2 + tc3.x()*r3;
                tc.y() = tc1.y()*r1 + tc2.y()*r2 + tc3.y()*r3;
                tc.z() = tc1.z()*r1 + tc2.z()*r2 + tc3.z()*r3;
            }
            else
            {
                return 0;
            }
        }

        const osg::TexMat* activeTexMat = 0;
        const osg::Texture* activeTexture = 0;

        if (drawable->getStateSet())
        {
            const osg::TexMat* texMat = dynamic_cast<osg::TexMat*>(drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXMAT));
            if (texMat) activeTexMat = texMat;

            const osg::Texture* texture = dynamic_cast<osg::Texture*>(drawable->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
            if (texture) activeTexture = texture;
        }

        for(osg::NodePath::const_reverse_iterator itr = nodePath.rbegin();
            itr != nodePath.rend() && (!activeTexMat || !activeTexture);
            ++itr)
        {
            const osg::Node* node = *itr;
            if (node->getStateSet())
            {
                if (!activeTexMat)
                {
                    const osg::TexMat* texMat = dynamic_cast<const osg::TexMat*>(node->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXMAT));
                    if (texMat) activeTexMat = texMat;
                }

                if (!activeTexture)
                {
                    const osg::Texture* texture = dynamic_cast<const osg::Texture*>(node->getStateSet()->getTextureAttribute(0,osg::StateAttribute::TEXTURE));
                    if (texture) activeTexture = texture;
                }
            }
        }

        if (activeTexMat)
        {
            osg::Vec4 tc_transformed = osg::Vec4(tc.x(), tc.y(), tc.z() ,0.0f) * activeTexMat->getMatrix();
            tc.x() = tc_transformed.x();
            tc.y() = tc_transformed.y();
            tc.z() = tc_transformed.z();

            if (activeTexture && activeTexMat->getScaleByTextureRectangleSize())
            {
                tc.x() *= static_cast<float>(activeTexture->getTextureWidth());
                tc.y() *= static_cast<float>(activeTexture->getTextureHeight());
                tc.z() *= static_cast<float>(activeTexture->getTextureDepth());
            }
        }

        return const_cast<osg::Texture*>(activeTexture);

    }
    return 0;
}
