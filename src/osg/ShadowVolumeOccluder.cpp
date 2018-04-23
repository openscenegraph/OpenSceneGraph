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
#include <osg/ShadowVolumeOccluder>
#include <osg/CullStack>

#include <osg/Group>
#include <osg/Geode>

using namespace osg;



typedef std::pair<unsigned int,Vec3>    Point; // bool=true signifies a newly created point, false indicates original point.
typedef std::vector<Point>      PointList;
typedef std::vector<Vec3>       VertexList;


// copyVertexListToPointList a vector for Vec3 into a vector of Point's.
void copyVertexListToPointList(const VertexList& in,PointList& out)
{
    out.reserve(in.size());
    for(VertexList::const_iterator itr=in.begin();
        itr!=in.end();
        ++itr)
    {
        out.push_back(Point(0,*itr));
    }
}

void copyPointListToVertexList(const PointList& in,VertexList& out)
{
    out.reserve(in.size());
    for(PointList::const_iterator itr=in.begin();
        itr!=in.end();
        ++itr)
    {
        out.push_back(itr->second);
    }
}

// clip the convex hull 'in' to plane to generate a clipped convex hull 'out'
// return true if points remain after clipping.
unsigned int clip(const Plane& plane,const PointList& in, PointList& out,unsigned int planeMask)
{
    std::vector<float> distance;
    distance.reserve(in.size());
    for(PointList::const_iterator itr=in.begin();
        itr!=in.end();
        ++itr)
    {
        distance.push_back(plane.distance(itr->second));
    }

    out.clear();

    for(unsigned int i=0;i<in.size();++i)
    {
        unsigned int i_1 = (i+1)%in.size(); // do the mod to wrap the index round back to the start.

        if (distance[i]>=0.0f)
        {
            out.push_back(in[i]);


            if (distance[i_1]<0.0f)
            {
                unsigned int mask = (in[i].first & in[i_1].first) | planeMask;
                float r = distance[i_1]/(distance[i_1]-distance[i]);
                out.push_back(Point(mask,in[i].second*r+in[i_1].second*(1.0f-r)));
            }

        }
        else if (distance[i_1]>0.0f)
        {
            unsigned int mask = (in[i].first & in[i_1].first) | planeMask;
            float r = distance[i_1]/(distance[i_1]-distance[i]);
            out.push_back(Point(mask,in[i].second*r+in[i_1].second*(1.0f-r)));
        }
    }

    return out.size();
}

// clip the convex hull 'in' to planeList to generate a clipped convex hull 'out'
// return true if points remain after clipping.
unsigned int clip(const Polytope::PlaneList& planeList,const VertexList& vin,PointList& out)
{
    PointList in;
    copyVertexListToPointList(vin,in);

    unsigned int planeMask = 0x1;
    for(Polytope::PlaneList::const_iterator itr=planeList.begin();
        itr!=planeList.end();
        ++itr)
    {
        if (!clip(*itr,in,out,planeMask)) return false;
        in.swap(out);
        planeMask <<= 1;
    }

    in.swap(out);

    return out.size();
}

void transform(PointList& points,const osg::Matrix& matrix)
{
    for(PointList::iterator itr=points.begin();
        itr!=points.end();
        ++itr)
    {
        itr->second = itr->second*matrix;
    }
}

void transform(const PointList& in,PointList& out,const osg::Matrix& matrix)
{
    for(PointList::const_iterator itr=in.begin();
        itr!=in.end();
        ++itr)
    {
        out.push_back(Point(itr->first,itr->second * matrix));
    }
}

void pushToFarPlane(PointList& points)
{
    for(PointList::iterator itr=points.begin();
        itr!=points.end();
        ++itr)
    {
        itr->second.z() = 1.0f;
    }
}

void computePlanes(const PointList& front, const PointList& back, Polytope::PlaneList& planeList)
{
    for(unsigned int i=0;i<front.size();++i)
    {
        unsigned int i_1 = (i+1)%front.size(); // do the mod to wrap the index round back to the start.
        if (!(front[i].first & front[i_1].first))
        {
            planeList.push_back(Plane(front[i].second,front[i_1].second,back[i].second));
        }
    }
}

Plane computeFrontPlane(const PointList& front)
{
    return Plane(front[2].second,front[1].second,front[0].second);
}

// compute the volume between the front and back polygons of the occluder/hole.
float computePolytopeVolume(const PointList& front, const PointList& back)
{
    float volume = 0.0f;
    Vec3 frontStart = front[0].second;
    Vec3 backStart = back[0].second;
    for(unsigned int i=1;i<front.size()-1;++i)
    {
        volume += computeVolume(frontStart, front[i].second, front[i+1].second,
                                backStart, back[i].second, back[i+1].second);
    }
    return volume;
}

bool ShadowVolumeOccluder::computeOccluder(const NodePath& nodePath,const ConvexPlanarOccluder& occluder,CullStack& cullStack,bool /*createDrawables*/)
{


//    std::cout<<"    Computing Occluder"<<std::endl;

    CullingSet& cullingset = cullStack.getCurrentCullingSet();

    const RefMatrix& MV = *cullStack.getModelViewMatrix();
    const RefMatrix& P = *cullStack.getProjectionMatrix();

    // take a reference to the NodePath to this occluder.
    _nodePath = nodePath;

    // take a reference to the projection matrix.
    _projectionMatrix = &P;

    // initialize the volume
    _volume = 0.0f;


    // compute the inverse of the projection matrix.
    Matrix invP;
    invP.invert(P);

    float volumeview = cullStack.getFrustumVolume();


    // compute the transformation matrix which takes form local coords into clip space.
    Matrix MVP(MV*P);

    // for the occluder polygon and each of the holes do
    //     first transform occluder polygon into clipspace by multiple it by c[i] = v[i]*(MV*P)
    //     then push to coords to far plane by setting its coord to c[i].z = -1.
    //     then transform far plane polygon back into projection space, by p[i]*inv(P)
    //     compute orientation of front plane, if normal.z()<0 then facing away from eye point, so reverse the polygons, or simply invert planes.
    //     compute volume (quality) between front polygon in projection space and back polygon in projection space.


    const VertexList& vertices_in = occluder.getOccluder().getVertexList();

    PointList clipped_points;

    if (clip(cullingset.getFrustum().getPlaneList(),vertices_in, clipped_points)>=3)
    {
        // compute the points on the far plane.
        PointList clipped_farPoints;
        clipped_farPoints.reserve(clipped_points.size());
        transform(clipped_points,clipped_farPoints,MVP);
        pushToFarPlane(clipped_farPoints);
        transform(clipped_farPoints,invP);

        // move the occlude points into projection space.
        transform(clipped_points,MV);

        // use the points on the front plane as reference vertices on the _occluderVolume
        // so that the vertices can later by used to test for occlusion of the occluder itself.
        copyPointListToVertexList(clipped_points,_occluderVolume.getReferenceVertexList());

        // create the front face of the occluder
        Plane clipped_occludePlane = computeFrontPlane(clipped_points);
        _occluderVolume.add(clipped_occludePlane);

        // create the sides of the occluder
        computePlanes(clipped_points,clipped_farPoints,_occluderVolume.getPlaneList());

        _occluderVolume.setupMask();

        // if the front face is pointing away from the eye point flip the whole polytope.
        if (clipped_occludePlane[3]>0.0f)
        {
            _occluderVolume.flip();
        }

        _volume = computePolytopeVolume(clipped_points,clipped_farPoints)/volumeview;


        for(ConvexPlanarOccluder::HoleList::const_iterator hitr=occluder.getHoleList().begin();
            hitr!=occluder.getHoleList().end();
            ++hitr)
        {
            PointList points;
            if (clip(cullingset.getFrustum().getPlaneList(),hitr->getVertexList(),points)>=3)
            {
                _holeList.push_back(Polytope());
                Polytope& polytope = _holeList.back();

                // compute the points on the far plane.
                PointList farPoints;
                farPoints.reserve(points.size());
                transform(points,farPoints,MVP);
                pushToFarPlane(farPoints);
                transform(farPoints,invP);

                // move the occlude points into projection space.
                transform(points,MV);

                // use the points on the front plane as reference vertices on the _occluderVolume
                // so that the vertices can later by used to test for occlusion of the occluder itself.
                copyPointListToVertexList(points,polytope.getReferenceVertexList());

                // create the front face of the occluder
                Plane occludePlane = computeFrontPlane(points);

                // create the sides of the occluder
                computePlanes(points,farPoints,polytope.getPlaneList());

                polytope.setupMask();

                // if the front face is pointing away from the eye point flip the whole polytope.
                if (occludePlane[3]>0.0f)
                {
                    polytope.flip();
                }

                // remove the hole's volume from the occluder volume.
                _volume -= computePolytopeVolume(points,farPoints)/volumeview;
            }

        }

        //std::cout << "final volume = "<<_volume<<std::endl;

        return true;
    }
    return false;
}

bool ShadowVolumeOccluder::contains(const std::vector<Vec3>& vertices)
{
    if (_occluderVolume.containsAllOf(vertices))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            PointList points;
            if (clip(itr->getPlaneList(),vertices,points)>=3) return false;
        }
        return true;
    }
    return false;
}

bool ShadowVolumeOccluder::contains(const BoundingSphere& bound)
{
    //std::cout << "Sphere testing occluder "<<this<<" mask="<<_occluderVolume.getCurrentMask();
    if (_occluderVolume.containsAllOf(bound))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(bound))
            {
                //std::cout << " - not in occluder"<<std::endl;
                return false;
            }
        }
        //std::cout << " - in occluder ******"<<std::endl;
        return true;
    }
    //std::cout << " - not in occluder"<<std::endl;
    return false;
}

bool ShadowVolumeOccluder::contains(const BoundingBox& bound)
{
    //std::cout << "Box testing occluder "<<this<<" mask="<<_occluderVolume.getCurrentMask();
    if (_occluderVolume.containsAllOf(bound))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(bound))
            {
                //std::cout << " + not in occluder"<<std::endl;
                return false;
            }
        }
        //std::cout << "+ in occluder ********"<<std::endl;
        return true;
    }
    //std::cout << "+ not in occluder"<<std::endl;
    return false;
}
