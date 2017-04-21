/* -*-c++-*- OpenSceneGraph - Copyright (C) 20017 Robert Osfield
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

#include <osg/Polytope>
#include <osg/Notify>

using namespace osg;

bool Polytope::contains(const osg::Vec3f& v0, const osg::Vec3f& v1, const osg::Vec3f& v2) const
{
    if (!_maskStack.back()) return true;

    // initialize the set of vertices to test.
    typedef std::vector<osg::Vec3d> Vertices;

    Vertices src, dest;
    src.reserve(4+_planeList.size());
    dest.reserve(4+_planeList.size());

    src.push_back(v0);
    src.push_back(v1);
    src.push_back(v2);
    src.push_back(v0);

    ClippingMask resultMask = _maskStack.back();
    ClippingMask selector_mask = 0x1;

    for(PlaneList::const_iterator pitr = _planeList.begin();
        pitr != _planeList.end();
        ++pitr)
    {
        if (resultMask&selector_mask)
        {
            //OSG_NOTICE<<"Polytope::contains() Plane testing"<<std::endl;

            dest.clear();

            const osg::Plane& plane = *pitr;
            Vertices::iterator vitr = src.begin();

            osg::Vec3d* v_previous = &(*(vitr++));
            double d_previous = plane.distance(*v_previous);

            for(; vitr != src.end(); ++vitr)
            {
                osg::Vec3d* v_current = &(*vitr);
                double d_current = plane.distance(*v_current);

                if (d_previous>=0.0)
                {
                    dest.push_back(*v_previous);
                }

                if (d_previous*d_current<0.0)
                {
                    // edge crosses plane so insert the vertex between them.
                    double distance = d_previous-d_current;
                    double r_current = d_previous/distance;
                    osg::Vec3d v_new = (*v_previous)*(1.0-r_current) + (*v_current)*r_current;
                    dest.push_back(v_new);
                }

                d_previous = d_current;
                v_previous = v_current;

            }

            if (d_previous>=0.0)
            {
                dest.push_back(*v_previous);
            }

            if (dest.size()<=1)
            {
                // OSG_NOTICE<<"Polytope::contains() All points on triangle culled, dest.size()="<<dest.size()<<std::endl;
                return false;
            }

            dest.swap(src);
        }
        else
        {
            // OSG_NOTICE<<"Polytope::contains() Plane disabled"<<std::endl;
        }

        selector_mask <<= 1;
    }

    //OSG_NOTICE<<"Polytope::contains() triangle within Polytope, src.size()="<<src.size()<<std::endl;
    return true;
}
