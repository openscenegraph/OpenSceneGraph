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
#include <osg/io_utils>

#include <osgSim/ElevationSlice>

#include <osg/Notify>
#include <osgUtil/PlaneIntersector>

using namespace osgSim;

namespace ElevationSliceUtils
{

struct DistanceHeightXYZ
{

    DistanceHeightXYZ():
        distance(0.0),
        height(0.0) {}

    DistanceHeightXYZ(double d, double h, const osg::Vec3d& pos):
        distance(d),
        height(h),
        position(pos) {}

    double      distance;
    double      height;
    osg::Vec3d  position;

    bool operator < ( const DistanceHeightXYZ& rhs) const
    {
        // small distance values first
        if (distance < rhs.distance) return true;
        if (distance > rhs.distance) return false;
        
        // greatest heights first
        if (height > rhs.height) return true;
        if (height < rhs.height) return false;
        
        return false;
    }
    
    bool equal_distance(const DistanceHeightXYZ& rhs, double epsilon=1e-6) const
    {
        return osg::absolute(rhs.distance - distance) <= epsilon;
    }

};


struct DistanceHeightCalculator
{
    DistanceHeightCalculator(osg::EllipsoidModel* em, const osg::Vec3d& startPoint, osg::Vec3d& endPoint):
        _em(em),
        _startPoint(startPoint),
        _startNormal(startPoint),
        _endPoint(endPoint),
        _endNormal(endPoint)
    {
        double latitude, longitude, height;

        // set up start point variables
        _em->convertXYZToLatLongHeight(_startPoint.x(), _startPoint.y(), _startPoint.z(), latitude, longitude, height);
        _startRadius = _startPoint.length() - height;
        _startNormal.normalize();

        // set up end point variables
        _em->convertXYZToLatLongHeight(_endPoint.x(), _endPoint.y(), _endPoint.z(), latitude, longitude, height);
        _endRadius = _endPoint.length() - height;
        _endNormal.normalize();

        osg::Vec3d normal = _startNormal ^ _endNormal;
        normal.normalize();

        _angleIncrement = 0.005;
        
        _radiusList.push_back(_startRadius);        
        _distanceList.push_back(0.0);
        
        osg::Matrixd rotationMatrix;
        double angleBetweenStartEnd = acos( _startNormal * _endNormal );
        double prevRadius = _startRadius;
        double distance = 0.0;
        for(double angle = _angleIncrement;
            angle < angleBetweenStartEnd;
            angle += _angleIncrement)
        {
            rotationMatrix.makeRotate(angle, normal);
            osg::Vec3d newVector = osg::Matrixd::transform3x3(_startPoint, rotationMatrix);
            
            _em->convertXYZToLatLongHeight(newVector.x(), newVector.y(), newVector.z(), latitude, longitude, height);
            double newRadius = newVector.length() - height;
            
            double distanceIncrement = _angleIncrement * (newRadius + prevRadius) *0.5;
            distance  += distanceIncrement;
            
            _radiusList.push_back(newRadius);
            _distanceList.push_back(distance);
            
            // osg::notify(osg::NOTICE)<<"  newVector = "<<newVector<<" newRadius = "<<newRadius<<" distanceIncrement="<<distanceIncrement<<std::endl;
            
            prevRadius = newRadius;
        }

    }
    
        
    void computeDistanceHeight(const osg::Vec3d& v, double& distance, double& height) const
    {
        osg::Vec3d vNormal = v;
        vNormal.normalize();

        // compute the height at position
        double latitude, longitude;
        _em->convertXYZToLatLongHeight(v.x(), v.y(), v.z(),
                                       latitude, longitude, height);
                                       
        // compute the radius at the point
        double Rv = v.length() - height;

        // compute the angle from the _startPoint
        double alpha = acos( vNormal * _startNormal);
        
        unsigned int int_alpha = static_cast<unsigned int>(floor(alpha / _angleIncrement));
        if (int_alpha >= _distanceList.size())
        {
            int_alpha = _distanceList.size() - 1;
        }
        
        double prevAlpha = ((double)int_alpha) * _angleIncrement;
        double deltaAlpha = alpha - prevAlpha;
        double prevDistance = _distanceList[int_alpha];
        double prevRadius = _radiusList[int_alpha];
        
        double averageRadius = (prevRadius + Rv)*0.5;
        double distanceIncrement = deltaAlpha * averageRadius;
        
        distance = prevDistance + distanceIncrement;

        // double oldDistance = alpha * (_startRadius + Rv) *0.5;

        // osg::notify(osg::NOTICE)<<" new distance = "<<distance<<" old = "<<oldDistance<<" delta = "<<oldDistance-distance<<std::endl;
    }

    typedef std::vector<double> DoubleList;

    osg::ref_ptr<osg::EllipsoidModel>   _em;

    osg::Vec3d                          _startPoint;
    osg::Vec3d                          _startNormal;
    double                              _startRadius;

    osg::Vec3d                          _endPoint;
    osg::Vec3d                          _endNormal;
    double                              _endRadius;

    double                              _angleIncrement;
    
    DoubleList                          _radiusList;    
    DoubleList                          _distanceList;    

};

}

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


    intersector->setRecordHeightsAsAttributes(true);
    intersector->setEllipsoidModel(em);

    _intersectionVisitor.reset();
    _intersectionVisitor.setIntersector( intersector.get() );
    
    scene->accept(_intersectionVisitor);
    
    osgUtil::PlaneIntersector::Intersections& intersections = intersector->getIntersections();

    typedef osgUtil::PlaneIntersector::Intersection::Polyline Polyline;
    typedef osgUtil::PlaneIntersector::Intersection::Attributes Attributes;

    if (!intersections.empty())
    {
        // osg::notify(osg::NOTICE)<<"Got intersections."<<std::endl;
        for(osgUtil::PlaneIntersector::Intersections::iterator itr = intersections.begin();
            itr != intersections.end();
            ++itr)
        {
            osgUtil::PlaneIntersector::Intersection& intersection = *itr;
            // osg::notify(osg::NOTICE)<<"  intersection - "<<intersection.polyline.size()<<std::endl;
            if (intersection.matrix.valid())
            {
                // osg::notify(osg::NOTICE)<<"  transforming "<<std::endl;
                // transform points on polyline 
                for(Polyline::iterator pitr = intersection.polyline.begin();
                    pitr != intersection.polyline.end();
                    ++pitr)
                {
                    *pitr = (*pitr) * (*intersection.matrix);
                }
                
                // matrix no longer needed.
                intersection.matrix = 0;
            }
#if 0            
            for(Polyline::iterator pitr = intersection.polyline.begin();
                pitr != intersection.polyline.end();
                ++pitr)
            {
                osg::notify(osg::NOTICE)<<"  v = "<<*pitr<<std::endl;
            }
#endif            
        }
        
        typedef std::set<ElevationSliceUtils::DistanceHeightXYZ> DistanceHeightSet;
        DistanceHeightSet distanceHeightSet;

        if (em)
        {
            ElevationSliceUtils::DistanceHeightCalculator dhc(em, _startPoint, _endPoint);

            // convert into distance/height
            for(osgUtil::PlaneIntersector::Intersections::iterator itr = intersections.begin();
                itr != intersections.end();
                ++itr)
            {
                osgUtil::PlaneIntersector::Intersection& intersection = *itr;
                
                if (intersection.attributes.size()!=intersection.polyline.size()) continue;
                
                Attributes::iterator aitr = intersection.attributes.begin();
                for(Polyline::iterator pitr = intersection.polyline.begin();
                    pitr != intersection.polyline.end();
                    ++pitr, ++aitr)
                {
                    const osg::Vec3d& v = *pitr;
                    double distance, height;
                    dhc.computeDistanceHeight(v, distance, height);
                    
                    double pi_height = *aitr;
                    
                    osg::notify(osg::NOTICE)<<"computed height = "<<height<<" PI height = "<<pi_height<<std::endl;
                    
                    distanceHeightSet.insert(ElevationSliceUtils::DistanceHeightXYZ( distance, pi_height, v));
                }
            }
        }
        else
        {
            // convert into distance/height
            for(osgUtil::PlaneIntersector::Intersections::iterator itr = intersections.begin();
                itr != intersections.end();
                ++itr)
            {
                osgUtil::PlaneIntersector::Intersection& intersection = *itr;
                for(Polyline::iterator pitr = intersection.polyline.begin();
                    pitr != intersection.polyline.end();
                    ++pitr)
                {
                    const osg::Vec3d& v = *pitr;
                    osg::Vec2d delta_xy( v.x() - _startPoint.x(), v.y() - _startPoint.y());
                    double distance = delta_xy.length();
                    distanceHeightSet.insert(ElevationSliceUtils::DistanceHeightXYZ( distance, v.z(), v));
                }
            }
        } 

        // copy final results
        _intersections.clear();
        _distanceHeightIntersections.clear();
        
        _intersections.reserve(distanceHeightSet.size());
        _distanceHeightIntersections.reserve(distanceHeightSet.size());
        
        for(DistanceHeightSet::iterator dhitr = distanceHeightSet.begin();
            dhitr != distanceHeightSet.end();
            ++dhitr)
        {
            const ElevationSliceUtils::DistanceHeightXYZ& dh = *dhitr;
            _intersections.push_back( dh.position );
            _distanceHeightIntersections.push_back( DistanceHeight(dh.distance, dh.height) );
        }
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
