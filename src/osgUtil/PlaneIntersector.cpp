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
    osg::notify(osg::NOTICE)<<"PlaneIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)"<<std::endl;

    if ( !_plane.intersect( drawable->getBound() ) ) return;
    if ( !_polytope.contains( drawable->getBound() ) ) return;

    osg::notify(osg::NOTICE)<<"Succed PlaneIntersector::intersect(osgUtil::IntersectionVisitor& iv, osg::Drawable* drawable)"<<std::endl;

    Intersection hit;
    hit.nodePath = iv.getNodePath();
    hit.drawable = drawable;
    
    insertIntersection(hit);
}


void PlaneIntersector::reset()
{
    Intersector::reset();
    
    _intersections.clear();
}
