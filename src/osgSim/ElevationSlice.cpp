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

#include <osg/CoordinateSystemNode>
#include <osgSim/ElevationSlice>

#include <osg/Notify>
#include <osgUtil/PlaneIntersector>

using namespace osgSim;

ElevationSlice::ElevationSlice()
{
    setDatabaseCacheReadCallback(new DatabaseCacheReadCallback);
}

void ElevationSlice::computeIntersections(osg::Node* scene)
{
    osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>(scene);
    osg::EllipsoidModel* em = csn ? csn->getEllipsoidModel() : 0;


    osg::Plane plane;
    osg::Polytope boundingPolytope;

    if (em)
    {

        osg::Vec3d start_upVector = em->computeLocalUpVector(_startPoint.x(), _startPoint.y(), _startPoint.z());
        osg::Vec3d end_upVector = em->computeLocalUpVector(_endPoint.x(), _endPoint.y(), _endPoint.z());

        double start_latitude, start_longitude, start_height;
        em->convertXYZToLatLongHeight(_startPoint.x(), _startPoint.y(), _startPoint.z(),
                                      start_latitude, start_longitude, start_height);

        osg::notify(osg::NOTICE)<<"start_lat = "<<start_latitude<<" start_longitude = "<<start_longitude<<" start_height = "<<start_height<<std::endl;

        double end_latitude, end_longitude, end_height;
        em->convertXYZToLatLongHeight(_endPoint.x(), _endPoint.y(), _endPoint.z(),
                                      end_latitude, end_longitude, end_height);

        osg::notify(osg::NOTICE)<<"end_lat = "<<end_latitude<<" end_longitude = "<<end_longitude<<" end_height = "<<end_height<<std::endl;
        
        // set up the main intersection plane
        osg::Vec3d planeNormal = (_endPoint - _startPoint) ^ start_upVector;
        planeNormal.normalize();        
        plane.set( planeNormal, _startPoint );
        
        // set up the start cut off plane
        osg::Vec3d startPlaneNormal = start_upVector ^ planeNormal;
        startPlaneNormal.normalize();
        boundingPolytope.add( osg::Plane(startPlaneNormal, _startPoint) );
        
        // set up the end cut off plane
        osg::Vec3d endPlaneNormal = planeNormal ^ end_upVector;
        endPlaneNormal.normalize();
        boundingPolytope.add( osg::Plane(endPlaneNormal, _endPoint) );
    }
    else
    {
        osg::Vec3d upVector (0.0, 0.0, 1.0);

        // set up the main intersection plane
        osg::Vec3d planeNormal = (_endPoint - _startPoint) ^ upVector;
        planeNormal.normalize();        
        plane.set( planeNormal, _startPoint );
        
        // set up the start cut off plane
        osg::Vec3d startPlaneNormal = upVector ^ planeNormal;
        startPlaneNormal.normalize();
        boundingPolytope.add( osg::Plane(startPlaneNormal, _startPoint) );
        
        // set up the end cut off plane
        osg::Vec3d endPlaneNormal = planeNormal ^ upVector;
        endPlaneNormal.normalize();
        boundingPolytope.add( osg::Plane(endPlaneNormal, _endPoint) );
    }
    
    osg::ref_ptr<osgUtil::PlaneIntersector> intersector = new osgUtil::PlaneIntersector(plane, boundingPolytope);

    _intersectionVisitor.reset();
    _intersectionVisitor.setIntersector( intersector.get() );
    
    scene->accept(_intersectionVisitor);
    
    osgUtil::PlaneIntersector::Intersections& intersections = intersector->getIntersections();
    if (!intersections.empty())
    {
        osg::notify(osg::NOTICE)<<"Got intersections."<<std::endl;
    }
    else
    {
        osg::notify(osg::NOTICE)<<"No intersections found."<<std::endl;
    }
    
}

ElevationSlice::Vec3dList ElevationSlice::computeElevationSlice(osg::Node* scene, const osg::Vec3d& startPoint, const osg::Vec3d& endPoint)
{
    ElevationSlice es;
    es.setStartPoint(startPoint);
    es.setEndPoint(endPoint);
    es.computeIntersections(scene);
    return es.getIntersections();
}

void ElevationSlice::setDatabaseCacheReadCallback(DatabaseCacheReadCallback* dcrc)
{
    _dcrc = dcrc;
    _intersectionVisitor.setReadCallback(dcrc);
}
