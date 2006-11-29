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


#include <osgUtil/PlaneIntersector>

#include <osg/Geometry>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TriangleFunctor>

using namespace osgUtil;

namespace PlaneIntersectorUtils
{

    struct TriangleIntersection
    {
        osg::Vec3d v[2];
    };

    typedef std::list<TriangleIntersection> TriangleIntersections;

    struct TriangleIntersector
    {

        osg::Plane _plane;
        osg::Polytope _polytope;
        bool _hit;

        TriangleIntersections _intersections;

        TriangleIntersector()
        {
            _hit = false;
        }

        void set(const osg::Plane& plane, const osg::Polytope& polytope)
        {
            _plane = plane;
            _polytope = polytope;
            _hit = false;
        }

        inline void operator () (const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3, bool)
        {
        
            double d1 = _plane.distance(v1);
            double d2 = _plane.distance(v2);
            double d3 = _plane.distance(v3);
            
            unsigned int numBelow = 0;
            unsigned int numAbove = 0;
            unsigned int numOnPlane = 0;
            if (d1<0) ++numBelow;
            else if (d1>0) ++numAbove;
            else ++numOnPlane;
        
            if (d2<0) ++numBelow;
            else if (d2>0) ++numAbove;
            else ++numOnPlane;
        
            if (d3<0) ++numBelow;
            else if (d3>0) ++numAbove;
            else ++numOnPlane;
            
            // trivially discard triangles that are completely one side of the plane
            if (numAbove==3 || numBelow==3) return;
            
            _hit = true;

            if (numOnPlane==3)
            {
                // triangle lives wholy in the plane
                osg::notify(osg::NOTICE)<<"3"<<std::endl;
                return;
            }

            if (numOnPlane==2)
            {
                // one edge lives wholy in the plane
                osg::notify(osg::NOTICE)<<"2"<<std::endl;
                return;
            }

            if (numOnPlane==1)
            {
                // one point lives wholy in the plane
                osg::notify(osg::NOTICE)<<"1"<<std::endl;
                return;
            }

            TriangleIntersection ti;
            unsigned int numIntersects = 0;

            if (d1*d2 < 0.0)
            {
                // edge 12 itersects
                double div = 1.0 / (d2-d1);
                ti.v[numIntersects++] = v1* (d2*div) - v2 * (d1*div);
            }

            if (d2*d3 < 0.0)
            {
                // edge 23 itersects
                double div = 1.0 / (d3-d2);
                ti.v[numIntersects++] = v2* (d3*div) - v3 * (d2*div);
            }

            if (d1*d3 < 0.0)
            {
                if (numIntersects<2)
                {
                    // edge 13 itersects
                    double div = 1.0 / (d3-d1);
                    ti.v[numIntersects++] = v1* (d3*div) - v3 * (d1*div);
                }
                else
                {
                    osg::notify(osg::NOTICE)<<"!!! too many intersecting edges found !!!"<<std::endl;
                }
                 
            }

            osg::notify(osg::NOTICE)<<"ti.v1="<<ti.v[0]<<" ti.v2="<<ti.v[1]<<std::endl;
            
            _intersections.push_back(ti);

        }

    };

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PlaneIntersector
//
PlaneIntersector::PlaneIntersector(const osg::Plane& plane, const osg::Polytope& boundingPolytope):
    _parent(0),
    _plane(plane),
    _polytope(boundingPolytope)
{
}

PlaneIntersector::PlaneIntersector(CoordinateFrame cf, const osg::Plane& plane, const osg::Polytope& boundingPolytope):
    Intersector(cf),
    _parent(0),
    _plane(plane),
    _polytope(boundingPolytope)
{
}

Intersector* PlaneIntersector::clone(osgUtil::IntersectionVisitor& iv)
{
    if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
    {
        osg::ref_ptr<PlaneIntersector> pi = new PlaneIntersector(_plane, _polytope);
        pi->_parent = this;
        return pi.release();
    }

    // compute the matrix that takes this Intersector from its CoordinateFrame into the local MODEL coordinate frame
    // that geometry in the scene graph will always be in.
    osg::Matrix matrix;
    switch (_coordinateFrame)
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

    osg::Plane plane = _plane;
    plane.transformProvidingInverse(matrix);

    osg::Polytope transformedPolytope;
    transformedPolytope.setAndTransformProvidingInverse(_polytope, matrix);

    osg::ref_ptr<PlaneIntersector> pi = new PlaneIntersector(plane, transformedPolytope);
    pi->_parent = this;
    return pi.release();
}

bool PlaneIntersector::enter(const osg::Node& node)
{
    return !node.isCullingActive() ||
           ( _plane.intersect(node.getBound())==0 && _polytope.contains(node.getBound()) );
}


void PlaneIntersector::leave()
{
    // do nothing.
}


void PlaneIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
    // osg::notify(osg::NOTICE)<<"PlaneIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)"<<std::endl;

    if ( _plane.intersect( drawable->getBound() )!=0 ) return;
    if ( !_polytope.contains( drawable->getBound() ) ) return;

    // osg::notify(osg::NOTICE)<<"Succed PlaneIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)"<<std::endl;

    osg::TriangleFunctor<PlaneIntersectorUtils::TriangleIntersector> ti;
    ti.set(_plane,_polytope);
    drawable->accept(ti);

    if (ti._hit)
    {
        Intersection hit;
        hit.nodePath = iv.getNodePath();
        hit.drawable = drawable;

        insertIntersection(hit);

        osg::notify(osg::NOTICE)<<std::endl<<"++++++++++++ Found intersections +++++++++++++++++++++++++"<<std::endl<<std::endl;
    }
    else
    {
        osg::notify(osg::NOTICE)<<std::endl<<"------------ No intersections -------------------------"<<std::endl<<std::endl;
    }

}


void PlaneIntersector::reset()
{
    Intersector::reset();
    
    _intersections.clear();
}
