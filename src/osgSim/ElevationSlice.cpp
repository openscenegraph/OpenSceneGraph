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
#include <osg/Geode>
#include <osg/Geometry>

#include <osgSim/ElevationSlice>

#include <osg/Notify>
#include <osgUtil/PlaneIntersector>

#include <osgDB/WriteFile>

using namespace osgSim;

namespace ElevationSliceUtils
{

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

            // OSG_NOTICE<<"  newVector = "<<newVector<<" newRadius = "<<newRadius<<" distanceIncrement="<<distanceIncrement<<std::endl;

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

#if 0
        double oldDistance = alpha * (_startRadius + Rv) *0.5;

        distance = oldDistance;

        OSG_NOTICE<<" new distance = "<<distance<<" old = "<<oldDistance<<" delta = "<<oldDistance-distance<<std::endl;
#endif

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

struct DistanceHeightXYZ
{

    DistanceHeightXYZ():
        distance(0.0),
        height(0.0) {}

    DistanceHeightXYZ(const DistanceHeightXYZ& dh):
        distance(dh.distance),
        height(dh.height),
        position(dh.position) {}

    DistanceHeightXYZ(double d, double h, const osg::Vec3d& pos):
        distance(d),
        height(h),
        position(pos) {}

    bool operator < (const DistanceHeightXYZ& rhs) const
    {
        // small distance values first
        if (distance < rhs.distance) return true;
        if (distance > rhs.distance) return false;

        // smallest heights first
        return (height < rhs.height);
    }

    bool operator == (const DistanceHeightXYZ& rhs) const
    {
        return distance==rhs.distance && height==rhs.height;
    }

    bool operator != (const DistanceHeightXYZ& rhs) const
    {
        return distance!=rhs.distance || height!=rhs.height;
    }

    bool equal_distance(const DistanceHeightXYZ& rhs, double epsilon=1e-6) const
    {
        return osg::absolute(rhs.distance - distance) <= epsilon;
    }

    double      distance;
    double      height;
    osg::Vec3d  position;
};

struct Point : public osg::Referenced, public DistanceHeightXYZ
{
    Point() {}
    Point(double d, double h, const osg::Vec3d& pos):
         DistanceHeightXYZ(d,h,pos)
    {
        //OSG_NOTICE<<"Point::Point distance="<<distance<<" height="<<height<<" position="<<position<<std::endl;
    }

    Point(const Point& point):
        osg::Referenced(),
        DistanceHeightXYZ(point) {}


};

struct Segment
{
    Segment(Point* p1, Point* p2)
    {
        if (*p1 < *p2)
        {
            _p1 = p1;
            _p2 = p2;
        }
        else
        {
            _p1 = p2;
            _p2 = p1;
        }

        //OSG_NOTICE.precision(12);
        //OSG_NOTICE<<"Segment::Segment p1 = "<<(_p1->distance)<<" "<<(_p1->height)<<"  p2 = "<<(_p2->distance)<<" "<<(_p2->height)<<std::endl;
    }


    bool operator < ( const Segment& rhs) const
    {
        if (*_p1 < *rhs._p1) return true;
        if (*rhs._p1 < *_p1) return false;

        return (*_p2 < *rhs._p2);
    }

    enum Classification
    {
        UNCLASSIFIED,
        IDENTICAL,
        SEPERATE,
        JOINED,
        OVERLAPPING,
        ENCLOSING,
        ENCLOSED
    };

    Classification compare(const Segment& rhs) const
    {
        if (*_p1 == *rhs._p1 && *_p2==*rhs._p2) return IDENTICAL;

        const double epsilon = 1e-3; // 1mm

        double delta_distance = _p2->distance - rhs._p1->distance;
        if (fabs(delta_distance) < epsilon)
        {
            if (fabs(_p2->height - rhs._p1->height) < epsilon) return JOINED;
        }

        if (delta_distance==0.0)
        {
            return SEPERATE;
        }

        if (rhs._p2->distance < _p1->distance || _p2->distance < rhs._p1->distance) return SEPERATE;

        bool rhs_p1_inside = (_p1->distance <= rhs._p1->distance) && (rhs._p1->distance <= _p2->distance);
        bool rhs_p2_inside = (_p1->distance <= rhs._p2->distance) && (rhs._p2->distance <= _p2->distance);

        if (rhs_p1_inside && rhs_p2_inside) return ENCLOSING;

        bool p1_inside = (rhs._p1->distance <= _p1->distance) && (_p1->distance <= rhs._p2->distance);
        bool p2_inside = (rhs._p1->distance <= _p2->distance) && (_p2->distance <= rhs._p2->distance);

        if (p1_inside && p2_inside) return ENCLOSED;

        if (rhs_p1_inside || rhs_p2_inside || p1_inside || p2_inside) return OVERLAPPING;

        return UNCLASSIFIED;
    }

    double height(double d) const
    {
        double delta = (_p2->distance - _p1->distance);
        return _p1->height + ((_p2->height - _p1->height) * (d - _p1->distance) / delta);
    }

    double deltaHeight(Point& point) const
    {
        return point.height - height(point.distance);
    }

    Point* createPoint(double d) const
    {
        if (d == _p1->distance) return _p1.get();
        if (d == _p2->distance) return _p2.get();

        double delta = (_p2->distance - _p1->distance);
        double r = (d - _p1->distance)/delta;
        double one_minus_r = 1.0 - r;
        return new Point(d,
                         _p1->height * one_minus_r + _p2->height * r,
                         _p1->position * one_minus_r + _p2->position * r);
    }

    Point* createIntersectionPoint(const Segment& rhs) const
    {
        double A = _p1->distance;
        double B = _p2->distance - _p1->distance;
        double C = _p1->height;
        double D = _p2->height - _p1->height;

        double E = rhs._p1->distance;
        double F = rhs._p2->distance - rhs._p1->distance;
        double G = rhs._p1->height;
        double H = rhs._p2->height - rhs._p1->height;

        double div = D*F - B*H;
        if (div==0.0)
        {
            OSG_NOTICE<<"ElevationSlideUtils::Segment::createIntersectionPoint(): error Segments are parallel."<<std::endl;
            return _p2.get();
        }

        double r = (G*F - E*H + A*H - C*F) / div;

        if (r<0.0)
        {
            OSG_NOTICE<<"ElevationSlideUtils::Segment::createIntersectionPoint(): error intersection point outwith segment, r ="<<r<<std::endl;
            return _p1.get();
        }

        if (r>1.0)
        {
            OSG_NOTICE<<"ElevationSlideUtils::Segment::createIntersectionPoint(): error intersection point outwith segment, r ="<<r<<std::endl;
            return _p2.get();
        }

//         OSG_NOTICE<<"ElevationSlideUtils::Segment::createIntersectionPoint(): r="<<r<<std::endl;
//         OSG_NOTICE<<"\tp1 = "<<_p1->distance<<" "<<_p1->height<<"  p2 = "<<_p2->distance<<" "<<_p2->height<<std::endl;
//         OSG_NOTICE<<"\trrhs.p1 = "<<rhs._p1->distance<<" "<<rhs._p1->height<<"  p2 = "<<rhs._p2->distance<<" "<<rhs._p2->height<<std::endl;

        return new Point(A + B*r, C + D*r, _p1->position + (_p2->position - _p1->position)*r);
    }


    osg::ref_ptr<Point> _p1;
    osg::ref_ptr<Point> _p2;

};


struct LineConstructor
{

    typedef std::set<Segment> SegmentSet;

    LineConstructor() {}



    void add(double d, double h, const osg::Vec3d& pos)
    {
        osg::ref_ptr<Point> newPoint = new Point(d,h,pos);


        if (_previousPoint.valid() && newPoint->distance != _previousPoint->distance)
        {
            const double maxGradient = 100.0;
            double gradient = fabs( (newPoint->height - _previousPoint->height) / (newPoint->distance - _previousPoint->distance) );

            if (gradient < maxGradient)
            {
                _segments.insert( Segment(_previousPoint.get(), newPoint.get()) );
            }
        }

        _previousPoint = newPoint;
    }

    void endline()
    {
        _previousPoint = 0;
    }

    void report()
    {

        OSG_NOTICE<<"Number of segments = "<<_segments.size()<<std::endl;

        for(SegmentSet::iterator itr = _segments.begin();
             itr != _segments.end();
             ++itr)
        {
            const Segment& seg = *itr;
            OSG_NOTICE<<"p1 = "<<(seg._p1->distance)<<" "<<(seg._p1->height)<<"  p2 = "<<(seg._p2->distance)<<" "<<(seg._p2->height)<<"\t";

            SegmentSet::iterator nextItr = itr;
            ++nextItr;
            if (nextItr != _segments.end())
            {
                Segment::Classification classification = itr->compare(*nextItr);
                switch(classification)
                {
                    case(Segment::IDENTICAL): OSG_NOTICE<<"i"; break;
                    case(Segment::SEPERATE): OSG_NOTICE<<"s"<<std::endl; break;
                    case(Segment::JOINED): OSG_NOTICE<<"j"; break;
                    case(Segment::OVERLAPPING): OSG_NOTICE<<"o"; break;
                    case(Segment::ENCLOSING): OSG_NOTICE<<"E"; break;
                    case(Segment::ENCLOSED): OSG_NOTICE<<"e"; break;
                    case(Segment::UNCLASSIFIED): OSG_NOTICE<<"U"; break;
                }
            }

            OSG_NOTICE<<std::endl;

        }

        OSG_NOTICE<<std::endl;

        if (_em.valid())
        {
            for(SegmentSet::iterator itr = _segments.begin();
                itr != _segments.end();
                ++itr)
            {
                const Segment& s = *itr;
                osg::Vec3d p;


                double latitude, longitude, height;

                p = s._p1->position;
                _em->convertXYZToLatLongHeight(p.x(), p.y(), p.z(), latitude, longitude, height);
                double delta1 = height - s._p1->height;

                p = s._p1->position;
                _em->convertXYZToLatLongHeight(p.x(), p.y(), p.z(), latitude, longitude, height);
                double delta2 = height - s._p2->height;

                if (delta1>0.0 || delta2>0.0)
                {
                    OSG_NOTICE<<"   "<<&s<<" computed height delta  ="<<delta1<<"  delta2= "<<delta2<<std::endl;
                }
            }
        }

    }

    void pruneOverlappingSegments()
    {
        SegmentSet::iterator prevItr = _segments.begin();
        SegmentSet::iterator nextItr = prevItr;
        ++nextItr;

        double epsilon = 0.001;

        for(SegmentSet::iterator itr = _segments.begin();
            itr != _segments.end();
            ++itr)
        {
            SegmentSet::iterator nextItr = itr;
            ++nextItr;
            Segment::Classification classification = nextItr != _segments.end() ?  itr->compare(*nextItr) : Segment::UNCLASSIFIED;

            // if (classification>=Segment::OVERLAPPING) OSG_NOTICE<<std::endl;
            // else OSG_NOTICE<<".";
            // OSG_NOTICE.precision(12);

            while (classification>=Segment::OVERLAPPING)
            {

                switch(classification)
                {
                    case(Segment::OVERLAPPING):
                    {
                        // cases....
                        // compute new end points for both segments
                        // need to work out which points are overlapping - lhs_p2 && rhs_p1  or  lhs_p1 and rhs_p2
                        // also need to check for cross cases.

                        const Segment& lhs = *itr;
                        const Segment& rhs = *nextItr;

                        bool rhs_p1_inside = (lhs._p1->distance <= rhs._p1->distance) && (rhs._p1->distance <= lhs._p2->distance);
                        bool lhs_p2_inside = (rhs._p1->distance <= lhs._p2->distance) && (lhs._p2->distance <= rhs._p2->distance);

                        if (rhs_p1_inside && lhs_p2_inside)
                        {
                            double distance_between = osg::Vec2d(lhs._p2->distance - rhs._p1->distance,
                                                                 lhs._p2->height - rhs._p1->height).length2();

                            if (distance_between < epsilon)
                            {
                                // OSG_NOTICE<<"OVERLAPPING : distance_between acceptable "<<distance_between<<std::endl;

                                Segment newSeg(lhs._p2.get(), rhs._p2.get());
                                _segments.insert(newSeg);

                                _segments.erase(nextItr);

                                nextItr = _segments.find(newSeg);
                            }
                            else
                            {
//                                OSG_NOTICE<<"OVERLAPPING : distance_between unacceptable "<<distance_between<<std::endl;

                                double dh1 = lhs.deltaHeight(*rhs._p1);
                                double dh2 = -rhs.deltaHeight(*lhs._p2);

                                if (dh1 * dh2 < 0.0)
                                {
//                                     OSG_NOTICE<<"OVERLAPPING : crossing "<<dh1<<" "<<dh2<<std::endl;
//                                     OSG_NOTICE<<"    lhs_p1 "<<lhs._p1->distance<<" "<<lhs._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    lhs_p2 "<<lhs._p2->distance<<" "<<lhs._p2->height<<std::endl;
//                                     OSG_NOTICE<<"    rhs_p1 "<<rhs._p1->distance<<" "<<rhs._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    rhs_p2 "<<rhs._p2->distance<<" "<<rhs._p2->height<<std::endl;

                                    Point* cp = lhs.createIntersectionPoint(rhs);

                                    Segment seg1( lhs._p1.get(), lhs.createPoint(rhs._p2->distance) );
                                    Segment seg2( rhs._p1.get(), cp );
                                    Segment seg3( cp, lhs._p2.get() );
                                    Segment seg4( rhs.createPoint(lhs._p2->distance), lhs._p2.get() );

                                    _segments.erase(nextItr);
                                    _segments.erase(itr);

                                    _segments.insert(seg1);
                                    _segments.insert(seg2);
                                    _segments.insert(seg3);
                                    _segments.insert(seg4);

                                    itr = _segments.find(seg1);
                                    nextItr = itr;
                                    ++nextItr;

                                }
                                else if (dh1 <= 0.0 && dh2 <= 0.0)
                                {
//                                     OSG_NOTICE<<"++ OVERLAPPING : rhs below lhs "<<dh1<<" "<<dh2<<std::endl;
//                                     OSG_NOTICE<<"    lhs_p1 "<<lhs._p1->distance<<" "<<lhs._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    lhs_p2 "<<lhs._p2->distance<<" "<<lhs._p2->height<<std::endl;
//                                     OSG_NOTICE<<"    rhs_p1 "<<rhs._p1->distance<<" "<<rhs._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    rhs_p2 "<<rhs._p2->distance<<" "<<rhs._p2->height<<std::endl;

                                    Segment newSeg(rhs.createPoint(lhs._p2->distance), rhs._p2.get());

                                    _segments.erase(nextItr);
                                    _segments.insert(newSeg);
                                    nextItr = itr;
                                    ++nextItr;

//                                     OSG_NOTICE<<"    newSeg_p1 "<<newSeg._p1->distance<<" "<<newSeg._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    newSeg_p2 "<<newSeg._p2->distance<<" "<<newSeg._p2->height<<std::endl;
                                 }
                                else if (dh1 >= 0.0 && dh2 >= 0.0)
                                {
//                                     OSG_NOTICE<<"++ OVERLAPPING : rhs above lhs "<<dh1<<" "<<dh2<<std::endl;
//                                     OSG_NOTICE<<"    lhs_p1 "<<lhs._p1->distance<<" "<<lhs._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    lhs_p2 "<<lhs._p2->distance<<" "<<lhs._p2->height<<std::endl;
//                                     OSG_NOTICE<<"    rhs_p1 "<<rhs._p1->distance<<" "<<rhs._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    rhs_p2 "<<rhs._p2->distance<<" "<<rhs._p2->height<<std::endl;


                                    Segment newSeg(lhs._p1.get(), lhs.createPoint(rhs._p1->distance));

                                    _segments.erase(itr);
                                    _segments.insert(newSeg);
                                    itr = _segments.find(newSeg);
                                    nextItr = itr;
                                    ++nextItr;

//                                     OSG_NOTICE<<"    newSeg_p1 "<<newSeg._p1->distance<<" "<<newSeg._p1->height<<std::endl;
//                                     OSG_NOTICE<<"    newSeg_p2 "<<newSeg._p2->distance<<" "<<newSeg._p2->height<<std::endl;

                                }
                                else
                                {
                                    OSG_NOTICE<<"OVERLAPPING : unidentified "<<dh1 <<" "<<dh2<<std::endl;
                                    OSG_NOTICE<<"    lhs_p1 "<<lhs._p1->distance<<" "<<lhs._p1->height<<std::endl;
                                    OSG_NOTICE<<"    lhs_p2 "<<lhs._p2->distance<<" "<<lhs._p2->height<<std::endl;
                                    OSG_NOTICE<<"    rhs_p1 "<<rhs._p1->distance<<" "<<rhs._p1->height<<std::endl;
                                    OSG_NOTICE<<"    rhs_p2 "<<rhs._p2->distance<<" "<<rhs._p2->height<<std::endl;
                                    ++nextItr;
                                }
                            }
                        }
                        else
                        {
                            OSG_NOTICE<<" OVERLAPPING problem, !rhs_p1_inside || !lhs_p2_inside - unhandled case" <<std::endl;
                            ++nextItr;
                        }


                        break;
                    }
                    case(Segment::ENCLOSING):
                    {
                        // need to work out if rhs is below lhs or rhs is above lhs or crossing

                        const Segment& enclosing = *itr;
                        const Segment& enclosed = *nextItr;
                        double dh1 = enclosing.deltaHeight(*enclosed._p1);
                        double dh2 = enclosing.deltaHeight(*enclosed._p2);

                        if (dh1<=epsilon && dh2<=epsilon)
                        {
                            // OSG_NOTICE<<"+++ ENCLOSING: ENCLOSING is above enclosed "<<dh1<<" "<<dh2<<std::endl;

                            _segments.erase(nextItr);

                            nextItr = itr;
                            ++nextItr;

                        }
                        else if (dh1>=0.0 && dh2>=0.0)
                        {

                            double d_left = enclosed._p1->distance - enclosing._p1->distance;
                            double d_right = enclosing._p2->distance - enclosed._p2->distance;

                            if (d_left < epsilon && d_right < epsilon)
                            {
                                // treat ENCLOSED as ENCLOSING.
                                // OSG_NOTICE<<"   Treat enclosed above as enclosing "<<std::endl;

                                nextItr = itr;
                                ++nextItr;

                                _segments.erase(itr);

                                itr = nextItr;
                                ++nextItr;

                            }
                            else if (d_left < epsilon)
                            {
//                                 OSG_NOTICE<<"ENCLOSING: ENCLOSING is below enclosed "<<dh1<<" "<<dh2<<std::endl;
//
//                                 OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;
//
//                                 OSG_NOTICE<<"   Replace enclosing with right section"<<std::endl;

                                Segment newSeg(enclosing.createPoint(enclosed._p2->distance), enclosing._p2.get());
                                nextItr = itr;
                                ++nextItr;

                                _segments.erase(itr);
                                _segments.insert(newSeg);

                                itr = nextItr;
                                ++nextItr;

//                                 OSG_NOTICE<<"    newSeg_p1 "<<newSeg._p1->distance<<" "<<newSeg._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    newSeg_p2 "<<newSeg._p2->distance<<" "<<newSeg._p2->height<<std::endl;
                            }
                            else if (d_right < epsilon)
                            {
//                                 OSG_NOTICE<<"ENCLOSING: ENCLOSING is below enclosed "<<dh1<<" "<<dh2<<std::endl;
//
//                                 OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;
//
//                                 OSG_NOTICE<<"   Replace enclosing with left section"<<std::endl;

                                Segment newSeg(enclosing._p1.get(), enclosing.createPoint(enclosed._p1->distance) );

                                _segments.insert(newSeg);
                                _segments.erase(itr);

                                itr = _segments.find(newSeg);
                                nextItr = itr;
                                ++nextItr;

//                                 OSG_NOTICE<<"    newSeg_p1 "<<newSeg._p1->distance<<" "<<newSeg._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    newSeg_p2 "<<newSeg._p2->distance<<" "<<newSeg._p2->height<<std::endl;
                            }
                            else
                            {
//                                 OSG_NOTICE<<"ENCLOSING: ENCLOSING is below enclosed "<<dh1<<" "<<dh2<<std::endl;
//
//                                 OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;
//
//                                 OSG_NOTICE<<"   Replace enclosing with left and right sections"<<std::endl;


                                Segment newSegLeft(enclosing._p1.get(), enclosing.createPoint(enclosed._p1->distance) );
                                Segment newSegRight(enclosing.createPoint(enclosed._p2->distance), enclosing._p2.get());

                                _segments.erase(itr);
                                _segments.insert(newSegLeft);
                                _segments.insert(newSegRight);

                                itr = _segments.find(newSegLeft);
                                nextItr = itr;
                                ++nextItr;

//                                 OSG_NOTICE<<"    newSegLeft_p1 "<<newSegLeft._p1->distance<<" "<<newSegLeft._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    newSegLeft_p2 "<<newSegLeft._p2->distance<<" "<<newSegLeft._p2->height<<std::endl;
//                                 OSG_NOTICE<<"    newSegRight_p1 "<<newSegRight._p1->distance<<" "<<newSegRight._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    newSegRight_p2 "<<newSegRight._p2->distance<<" "<<newSegRight._p2->height<<std::endl;

                            }

                        }
                        else if (dh1 * dh2 < 0.0)
                        {
//                             OSG_NOTICE<<"ENCLOSING: ENCLOSING is crossing enclosed "<<dh1<<" "<<dh2<<std::endl;
//
//                             OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
//                             OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
//                             OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
//                             OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;

                            double d_left = enclosed._p1->distance - enclosing._p1->distance;
                            double d_right = enclosing._p2->distance - enclosed._p2->distance;

                            if (d_left < epsilon && d_right < epsilon)
                            {
                                // treat ENCLOSED as ENCLOSING.
                                if (dh1 < 0.0)
                                {
                                    // OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosing._p1.get(), cp);
                                    Segment newSegRight(cp, enclosed._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }
                                else
                                {
                                    // OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosed._p1.get(), cp);
                                    Segment newSegRight(cp, enclosing._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }

                            }
                            else if (d_left < epsilon)
                            {
                                // OSG_NOTICE<<"   >> Replace enclosing with right section"<<std::endl;

                                if (dh1 < 0.0)
                                {
                                    // OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosing._p1.get(), cp);
                                    Segment newSegMid(cp, enclosed._p2.get());
                                    Segment newSegRight(enclosing.createPoint(enclosed._p2->distance), enclosing._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegMid);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }
                                else
                                {
                                    // OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosed._p1.get(), cp);
                                    Segment newSegRight(cp, enclosing._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }
                            }
                            else if (d_right < epsilon)
                            {
//                                OSG_NOTICE<<"   >> Replace enclosing with left section"<<std::endl;

                                if (dh1 < 0.0)
                                {
//                                    OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosing._p1.get(), cp);
                                    Segment newSegRight(cp, enclosed._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }
                                else
                                {
//                                    OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosing._p1.get(), enclosing.createPoint(enclosed._p1->distance));
                                    Segment newSegMid(enclosed._p1.get(), cp);
                                    Segment newSegRight(cp, enclosing._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegMid);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }
                            }
                            else
                            {
//                                OSG_NOTICE<<"   >> Replace enclosing with left and right sections"<<std::endl;


                                if (dh1 < 0.0)
                                {
//                                    OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosing._p1.get(), cp);
                                    Segment newSegMid(cp, enclosed._p2.get());
                                    Segment newSegRight(enclosing.createPoint(enclosed._p2->distance), enclosing._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegMid);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }
                                else
                                {
//                                    OSG_NOTICE<<"   >> enclosing left side is above enclosed left side"<<std::endl;

                                    Point* cp = enclosing.createIntersectionPoint(enclosed);
                                    Segment newSegLeft(enclosing._p1.get(), enclosing.createPoint(enclosed._p1->distance));
                                    Segment newSegMid(enclosed._p1.get(), cp);
                                    Segment newSegRight(cp, enclosing._p2.get());

                                    _segments.erase(itr);
                                    _segments.erase(nextItr);

                                    _segments.insert(newSegLeft);
                                    _segments.insert(newSegMid);
                                    _segments.insert(newSegRight);

                                    itr = _segments.find(newSegLeft);
                                    nextItr = itr;
                                    ++nextItr;
                                }

                            }
                        }
                        else
                        {
                            OSG_NOTICE<<"ENCLOSING: ENCLOSING - error case "<<dh1<<" "<<dh2<<std::endl;

                            OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
                            OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
                            OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
                            OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;
                            ++nextItr;
                        }

                        break;
                    }
                    case(Segment::ENCLOSED):
                    {
                        // need to work out if lhs is below rhs or lhs is above rhs or crossing
                        const Segment& enclosing = *nextItr;
                        const Segment& enclosed = *itr;
                        double dh1 = enclosing.deltaHeight(*enclosed._p1);
                        double dh2 = enclosing.deltaHeight(*enclosed._p2);

                        double d_left = enclosed._p1->distance - enclosing._p1->distance;
                        double d_right = enclosing._p2->distance - enclosed._p2->distance;

                        if (d_left<=epsilon)
                        {

                            if (dh1<=epsilon && dh2<=epsilon)
                            {
                                // OSG_NOTICE<<"+++ ENCLOSED: ENCLOSING is above enclosed "<<dh1<<" "<<dh2<<std::endl;
                                _segments.erase(itr);

                                itr = nextItr;
                                ++nextItr;
                            }
                            else if (dh1>=0.0 && dh2>=0.0)
                            {
//                                 OSG_NOTICE<<"ENCLOSED: ENCLOSING is below enclosed "<<dh1<<" "<<dh2<<std::endl;
//                                 OSG_NOTICE<<"    d_left "<<d_left<<" d_right="<<d_right<<std::endl;
//                                 OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;

                                _segments.insert(Segment(enclosing.createPoint(enclosed._p2->distance), enclosed._p2.get()));
                                _segments.erase(nextItr);

                                nextItr = itr;
                                ++nextItr;
                            }
                            else if (dh1 * dh2 < 0.0)
                            {
//                                 OSG_NOTICE<<"ENCLOSED: ENCLOSING is crossing enclosed "<<dh1<<" "<<dh2<<std::endl;
//                                 OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
//                                 OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;

                                if (d_right<=epsilon)
                                {
                                    // enclosed and enclosing effectively overlap
                                    if (dh1 < 0.0)
                                    {
                                        Point* cp = enclosed.createIntersectionPoint(enclosing);
                                        Segment segLeft(enclosed._p1.get(), cp);
                                        Segment segRight(cp, enclosing._p2.get());

                                        _segments.erase(itr);
                                        _segments.erase(nextItr);

                                        _segments.insert(segLeft);
                                        _segments.insert(segRight);

                                        itr = _segments.find(segLeft);
                                        nextItr = itr;
                                        ++nextItr;
                                    }
                                    else
                                    {
                                        Point* cp = enclosed.createIntersectionPoint(enclosing);
                                        Segment segLeft(enclosing._p1.get(), cp);
                                        Segment segRight(cp, enclosed._p2.get());

                                        _segments.erase(itr);
                                        _segments.erase(nextItr);

                                        _segments.insert(segLeft);
                                        _segments.insert(segRight);

                                        itr = _segments.find(segLeft);
                                        nextItr = itr;
                                        ++nextItr;
                                    }
                                }
                                else
                                {
                                    // right hand side needs to be created
                                    if (dh1 < 0.0)
                                    {
                                        Point* cp = enclosed.createIntersectionPoint(enclosing);
                                        Segment segLeft(enclosed._p1.get(), cp);
                                        Segment segRight(cp, enclosing._p2.get());

                                        _segments.erase(itr);
                                        _segments.erase(nextItr);

                                        _segments.insert(segLeft);
                                        _segments.insert(segRight);

                                        itr = _segments.find(segLeft);
                                        nextItr = itr;
                                        ++nextItr;
                                    }
                                    else
                                    {
                                        Point* cp = enclosed.createIntersectionPoint(enclosing);
                                        Segment segLeft(enclosing._p1.get(), cp);
                                        Segment segMid(cp, enclosed._p2.get());
                                        Segment segRight(enclosing.createPoint(enclosed._p2->distance), enclosing._p2.get());

                                        _segments.erase(itr);
                                        _segments.erase(nextItr);

                                        _segments.insert(segLeft);
                                        _segments.insert(segMid);
                                        _segments.insert(segRight);

                                        itr = _segments.find(segLeft);
                                        nextItr = itr;
                                        ++nextItr;
                                    }
                                }
                            }
                            else
                            {
                                OSG_NOTICE<<"ENCLOSED: ENCLOSING - error case "<<dh1<<" "<<dh2<<std::endl;
                                OSG_NOTICE<<"    enclosing_p1 "<<enclosing._p1->distance<<" "<<enclosing._p1->height<<std::endl;
                                OSG_NOTICE<<"    enclosing_p2 "<<enclosing._p2->distance<<" "<<enclosing._p2->height<<std::endl;
                                OSG_NOTICE<<"    enclosed_p1 "<<enclosed._p1->distance<<" "<<enclosed._p1->height<<std::endl;
                                OSG_NOTICE<<"    enclosed_p2 "<<enclosed._p2->distance<<" "<<enclosed._p2->height<<std::endl;
                                ++nextItr;
                            }
                        }
                        else
                        {
                            OSG_NOTICE<<"*** ENCLOSED: is not coincendet with left handside of ENCLOSING, case not handled, advancing."<<std::endl;
                        }

                        break;
                    }
                    default:
                        OSG_NOTICE<<"** Not handled, advancing"<<std::endl;
                        ++nextItr;
                        break;
                }

                classification = ((itr != _segments.end()) && (nextItr != _segments.end())) ?  itr->compare(*nextItr) : Segment::UNCLASSIFIED;
            }
        }
    }

    unsigned int numOverlapping(SegmentSet::const_iterator startItr) const
    {
        if (startItr==_segments.end()) return 0;

        SegmentSet::const_iterator nextItr = startItr;
        ++nextItr;

        unsigned int num = 0;
        while (nextItr!=_segments.end() && startItr->compare(*nextItr)>=Segment::OVERLAPPING)
        {
            ++num;
            ++nextItr;
        }
        return num;
    }

    unsigned int totalNumOverlapping() const
    {
        unsigned int total = 0;
        for(SegmentSet::const_iterator itr = _segments.begin();
            itr != _segments.end();
            ++itr)
        {
            total += numOverlapping(itr);
        }
        return total;
    }

    void copyPoints(ElevationSlice::Vec3dList& intersections, ElevationSlice::DistanceHeightList& distanceHeightIntersections)
    {
        SegmentSet::iterator prevItr = _segments.begin();
        SegmentSet::iterator nextItr = prevItr;
        ++nextItr;

        intersections.push_back( prevItr->_p1->position );
        distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(prevItr->_p1->distance, prevItr->_p1->height) );

        intersections.push_back( prevItr->_p2->position );
        distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(prevItr->_p2->distance, prevItr->_p2->height) );

        for(;
            nextItr != _segments.end();
            ++nextItr,++prevItr)
        {
            Segment::Classification classification = prevItr->compare(*nextItr);
            switch(classification)
            {
                case(Segment::SEPERATE):
                {
                    intersections.push_back( nextItr->_p1->position );
                    distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(nextItr->_p1->distance, nextItr->_p1->height) );

                    intersections.push_back( nextItr->_p2->position );
                    distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(nextItr->_p2->distance, nextItr->_p2->height) );
                    break;
                }
                case(Segment::JOINED):
                {
#if 1
                    intersections.push_back( nextItr->_p2->position );
                    distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(nextItr->_p2->distance, nextItr->_p2->height) );
#else
                    intersections.push_back( nextItr->_p1->position );
                    distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(nextItr->_p1->distance, nextItr->_p1->height) );

                    intersections.push_back( nextItr->_p2->position );
                    distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(nextItr->_p2->distance, nextItr->_p2->height) );
#endif
                    break;
                }
                default:
                {
                    intersections.push_back( nextItr->_p1->position );
                    distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(nextItr->_p1->distance, nextItr->_p1->height) );

                    intersections.push_back( nextItr->_p2->position );
                    distanceHeightIntersections.push_back( ElevationSlice::DistanceHeight(nextItr->_p2->distance, nextItr->_p2->height) );
                    break;
                }
            }

        }

    }

    SegmentSet _segments;
    osg::ref_ptr<Point> _previousPoint;
    osg::Plane  _plane;
    osg::ref_ptr<osg::EllipsoidModel>   _em;

};

}

ElevationSlice::ElevationSlice()
{
    setDatabaseCacheReadCallback(new DatabaseCacheReadCallback);
}

void ElevationSlice::computeIntersections(osg::Node* scene, osg::Node::NodeMask traversalMask)
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

        OSG_NOTICE<<"start_lat = "<<start_latitude<<" start_longitude = "<<start_longitude<<" start_height = "<<start_height<<std::endl;

        double end_latitude, end_longitude, end_height;
        em->convertXYZToLatLongHeight(_endPoint.x(), _endPoint.y(), _endPoint.z(),
                                      end_latitude, end_longitude, end_height);

        OSG_NOTICE<<"end_lat = "<<end_latitude<<" end_longitude = "<<end_longitude<<" end_height = "<<end_height<<std::endl;

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
    _intersectionVisitor.setTraversalMask(traversalMask);
    _intersectionVisitor.setIntersector( intersector.get() );

    scene->accept(_intersectionVisitor);

    osgUtil::PlaneIntersector::Intersections& intersections = intersector->getIntersections();

    typedef osgUtil::PlaneIntersector::Intersection::Polyline Polyline;
    typedef osgUtil::PlaneIntersector::Intersection::Attributes Attributes;

    if (!intersections.empty())
    {

        osgUtil::PlaneIntersector::Intersections::iterator itr;
        for(itr = intersections.begin();
            itr != intersections.end();
            ++itr)
        {
            osgUtil::PlaneIntersector::Intersection& intersection = *itr;

            if (intersection.matrix.valid())
            {
                // OSG_NOTICE<<"  transforming "<<std::endl;
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
        }

#if 0
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;

        for(itr = intersections.begin();
            itr != intersections.end();
            ++itr)
        {

            osgUtil::PlaneIntersector::Intersection& intersection = *itr;
            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            vertices->reserve(intersection.polyline.size());
            for(Polyline::iterator pitr = intersection.polyline.begin();
                pitr != intersection.polyline.end();
                ++pitr)
            {
                vertices->push_back(*pitr);
            }

            geometry->setVertexArray( vertices );
            geometry->addPrimitiveSet( new osg::DrawArrays(GL_LINE_STRIP, 0, vertices->size()) );

            osg::Vec4Array* colours = new osg::Vec4Array;
            colours->push_back( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );

            geometry->setColorArray( colours );

            geode->addDrawable( geometry );
            geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        }

        osgDB::writeNodeFile(*geode, "raw.osg");
#endif

        ElevationSliceUtils::LineConstructor constructor;
        constructor._plane = plane;
        constructor._em = em;

        if (em)
        {
            ElevationSliceUtils::DistanceHeightCalculator dhc(em, _startPoint, _endPoint);

            // convert into distance/height
            for(itr = intersections.begin();
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

                    // OSG_NOTICE<<"computed height = "<<height<<" PI height = "<<pi_height<<std::endl;

                    constructor.add( distance, pi_height, v);

                }
                constructor.endline();
            }


        }
        else
        {
            // convert into distance/height
            for(itr = intersections.begin();
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

                    constructor.add( distance, v.z(), v);
                }
                constructor.endline();
            }
        }

        // copy final results
        _intersections.clear();
        _distanceHeightIntersections.clear();

        // constructor.report();

        unsigned int numOverlapping = constructor.totalNumOverlapping();

        while(numOverlapping>0)
        {
            unsigned int previousNumOverlapping = numOverlapping;

            constructor.pruneOverlappingSegments();
            // constructor.report();

            numOverlapping = constructor.totalNumOverlapping();
            if (previousNumOverlapping == numOverlapping) break;
        }

        constructor.copyPoints(_intersections, _distanceHeightIntersections);

#if 0
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;

            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            vertices->reserve(_intersections.size());
            for(Vec3dList::iterator pitr = _intersections.begin();
                pitr != _intersections.end();
                ++pitr)
            {
                vertices->push_back(*pitr);
            }

            geometry->setVertexArray( vertices );
            geometry->addPrimitiveSet( new osg::DrawArrays(GL_LINE_STRIP, 0, _intersections.size()) );

            osg::Vec4Array* colours = new osg::Vec4Array;
            colours->push_back( osg::Vec4(1.0f,0.5f,0.5f,1.0f) );

            geometry->setColorArray( colours );

            geode->addDrawable( geometry );
            geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

            osgDB::writeNodeFile(*geode, "processed.osg");
        }
#endif

    }
    else
    {
        OSG_NOTICE<<"No intersections found."<<std::endl;
    }

}

ElevationSlice::Vec3dList ElevationSlice::computeElevationSlice(osg::Node* scene, const osg::Vec3d& startPoint, const osg::Vec3d& endPoint, osg::Node::NodeMask traversalMask)
{
    ElevationSlice es;
    es.setStartPoint(startPoint);
    es.setEndPoint(endPoint);
    es.computeIntersections(scene, traversalMask);
    return es.getIntersections();
}

void ElevationSlice::setDatabaseCacheReadCallback(DatabaseCacheReadCallback* dcrc)
{
    _dcrc = dcrc;
    _intersectionVisitor.setReadCallback(dcrc);
}
