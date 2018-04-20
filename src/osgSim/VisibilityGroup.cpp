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

#include <osgSim/VisibilityGroup>

#include <osgUtil/CullVisitor>
#include <osgUtil/LineSegmentIntersector>

using namespace osgSim;
using namespace osg;

VisibilityGroup::VisibilityGroup():
    _volumeIntersectionMask(0xFFFFFFFF),
    _segmentLength(0.f)
{
}

VisibilityGroup::VisibilityGroup(const VisibilityGroup& sw,const osg::CopyOp& copyop):
    osg::Group(sw,copyop),
    _volumeIntersectionMask(0xFFFFFFFF),
    _segmentLength(0.f)
{
}

void VisibilityGroup::traverse(osg::NodeVisitor& nv)
{
    if (nv.getTraversalMode()==osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN && nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        // cast to cullvisitor
        osgUtil::CullVisitor& cv = (osgUtil::CullVisitor&) nv;

        // here we test if we are inside the visibilityvolume

        // first get the eyepoint and in local coordinates
        osg::Vec3 eye = cv.getEyeLocal();
        osg::Vec3 look = cv.getLookVectorLocal();

        // now scale the segment to the segment length - if 0 use the group bounding sphere radius
        float length = _segmentLength;
        if(length == 0.f)
            length = 2.0f*getBound().radius();
        look *= length;
        osg::Vec3 center = eye + look;

        osg::Vec3 seg = center - eye;

        // perform the intersection using the given mask
        osg::ref_ptr<osgUtil::LineSegmentIntersector> lineseg = new osgUtil::LineSegmentIntersector(eye, center);
        osgUtil::IntersectionVisitor iv(lineseg.get());
        iv.setTraversalMask(_volumeIntersectionMask);

        if(_visibilityVolume.valid())
            _visibilityVolume->accept(iv);

        // now examine the hit record
        if(lineseg->containsIntersections())
        {
            osgUtil::LineSegmentIntersector::Intersection intersection = lineseg->getFirstIntersection();
            osg::Vec3 normal = intersection.getWorldIntersectNormal();

            if((normal*seg) > 0.f ) // we are inside
                Group::traverse(nv);
        }
    }
    else
    {
        Group::traverse(nv);
    }
}

