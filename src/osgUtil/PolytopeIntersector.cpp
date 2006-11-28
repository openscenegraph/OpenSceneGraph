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


#include <osgUtil/PolytopeIntersector>

#include <osg/Geometry>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TriangleFunctor>

using namespace osgUtil;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PolytopeIntersector
//
PolytopeIntersector::PolytopeIntersector(const osg::Polytope& polytope):
    _parent(0),
    _polytope(polytope)
{
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, const osg::Polytope& polytope):
    Intersector(cf),
    _parent(0),
    _polytope(polytope)
{
}

PolytopeIntersector::PolytopeIntersector(CoordinateFrame cf, double xMin, double yMin, double xMax, double yMax):
    Intersector(cf),
    _parent(0)
{
    double zNear = 0.0;
    switch(cf)
    {
        case WINDOW : zNear = 0.0; break;
        case PROJECTION : zNear = 1.0; break;
        case VIEW : zNear = 0.0; break;
        case MODEL : zNear = 0.0; break;
    }

    _polytope.add(osg::Plane(1.0, 0.0, 0.0, -xMin));
    _polytope.add(osg::Plane(-1.0,0.0 ,0.0, xMax));
    _polytope.add(osg::Plane(0.0, 1.0, 0.0,-yMin));
    _polytope.add(osg::Plane(0.0,-1.0,0.0, yMax));
    _polytope.add(osg::Plane(0.0,0.0,1.0, zNear));
}

Intersector* PolytopeIntersector::clone(osgUtil::IntersectionVisitor& iv)
{
    if (_coordinateFrame==MODEL && iv.getModelMatrix()==0)
    {
        osg::ref_ptr<PolytopeIntersector> pi = new PolytopeIntersector(_polytope);
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

    osg::Polytope transformedPolytope;
    transformedPolytope.setAndTransformProvidingInverse(_polytope, matrix);

    osg::ref_ptr<PolytopeIntersector> pi = new PolytopeIntersector(transformedPolytope);
    pi->_parent = this;
    return pi.release();
}

bool PolytopeIntersector::enter(const osg::Node& node)
{
    return !node.isCullingActive() || _polytope.contains( node.getBound() );
}


void PolytopeIntersector::leave()
{
    // do nothing.
}


void PolytopeIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)
{
    if ( !_polytope.contains( drawable->getBound() ) ) return;

    osg::Geometry* geometry = drawable->asGeometry();
    osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;
    if (vertices)
    {
        if (!_polytope.contains(*vertices)) return;
    }

    Intersection hit;
    hit.nodePath = iv.getNodePath();
    hit.drawable = drawable;
    
    insertIntersection(hit);
}


void PolytopeIntersector::reset()
{
    Intersector::reset();
    
    _intersections.clear();
}
