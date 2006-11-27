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

        osg::Vec3d upVector = em->computeLocalUpVector(_startPoint.x(), _startPoint.y(), _startPoint.z());

        double latitude, longitude, height;
        em->convertXYZToLatLongHeight(_startPoint.x(), _startPoint.y(), _startPoint.z(), latitude, longitude, height);

        osg::notify(osg::NOTICE)<<"lat = "<<latitude<<" longitude = "<<longitude<<" height = "<<height<<std::endl;

    }
    else
    {
        osg::Vec3d upVector (0.0, 0.0, 1.0);

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
