#include <osg/ShadowVolumeOccluder>
#include <osg/CullStack>

#include <osg/Group>
#include <osg/Geode>
#include <osg/GeoSet>

using namespace osg;



typedef std::pair<unsigned int,Vec3>    Point; // bool=true signifies a newly created point, false indicates original point.
typedef std::vector<Point>      PointList;
typedef std::vector<Vec3>       VertexList;


// convert a vector for Vec3 into a vector of Point's.
void convert(const VertexList& in,PointList& out)
{
    out.reserve(in.size());
    for(VertexList::const_iterator itr=in.begin();
        itr!=in.end();
        ++itr)
    {
        out.push_back(Point(0,*itr));
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
    convert(vin,in);
    
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

Drawable* createOccluderDrawable(const PointList& front, const PointList& back)
{
    // create a drawable for occluder.
    osg::GeoSet* geoset = osgNew osg::GeoSet;
    
    int totalNumber = front.size()+back.size();
    osg::Vec3* coords = osgNew osg::Vec3[front.size()+back.size()];
    osg::Vec3* cptr = coords;
    for(PointList::const_iterator fitr=front.begin();
        fitr!=front.end();
        ++fitr)
    {
        *cptr = fitr->second;
        ++cptr;
    }

    for(PointList::const_iterator bitr=back.begin();
        bitr!=back.end();
        ++bitr)
    {
        *cptr = bitr->second;
        ++cptr;
    }

    geoset->setCoords(coords);
    
    osg::Vec4* color = osgNew osg::Vec4[1];
    color[0].set(1.0f,1.0f,1.0f,0.5f);
    geoset->setColors(color);
    geoset->setColorBinding(osg::GeoSet::BIND_OVERALL);
    
    geoset->setPrimType(osg::GeoSet::POINTS);
    geoset->setNumPrims(totalNumber);
    
    //cout << "totalNumber = "<<totalNumber<<endl;


    osg::Geode* geode = osgNew osg::Geode;
    geode->addDrawable(geoset);
    
    osg::StateSet* stateset = osgNew osg::StateSet;
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    geoset->setStateSet(stateset);

    return geoset;
}



bool ShadowVolumeOccluder::computeOccluder(const NodePath& nodePath,const ConvexPlanerOccluder& occluder,CullStack& cullStack,bool createDrawables)
{


//    std::cout<<"    Computing Occluder"<<std::endl;

    CullingSet& cullingset = cullStack.getCurrentCullingSet();

    const Matrix& MV = cullStack.getModelViewMatrix();
    const Matrix& P = cullStack.getProjectionMatrix();

    // take a reference to the NodePath to this occluder.
    _nodePath = nodePath;
    

    // take a reference to the projection matrix.
    _projectionMatrix = &P;
    
    

    // compute the inverse of the projection matrix.
    Matrix invP;
    invP.invert(P);
    
    
    // compute the transformation matrix which takes form local coords into clip space.
    Matrix MVP(MV*P);
    
    // for the occluder polygon and each of the holes do
    //     first transform occluder polygon into clipspace by multiple it by c[i] = v[i]*(MV*P)
    //     then push to coords to far plane by setting its coord to c[i].z = -1.
    //     then transform far plane polygon back into projection space, by p[i]*inv(P)
    //     compute orientation of front plane, if normal.z()<0 then facing away from eye pont, so reverse the polygons, or simply invert planes.
    //     compute volume (quality) betwen front polygon in projection space and back polygon in projection space.
    
    
    const VertexList& vertices_in = occluder.getOccluder().getVertexList();
    
    PointList points;
    
    if (clip(cullingset.getFrustum().getPlaneList(),vertices_in,points)>=3)
    {
        // compute the points on the far plane.
        PointList farPoints;
        farPoints.reserve(points.size());
        transform(points,farPoints,MVP);
        pushToFarPlane(farPoints);
        transform(farPoints,invP);
        
        // move the occlude points into projection space.
        transform(points,MV);

        // create the front face of the occluder
        Plane occludePlane = computeFrontPlane(points);
        _occluderVolume.add(occludePlane);

        // create the sides of the occluder
        computePlanes(points,farPoints,_occluderVolume.getPlaneList());

        _occluderVolume.setupMask();
        
        // if the front face is pointing away from the eye point flip the whole polytope.
        if (occludePlane[3]>0.0f)
        {
            _occluderVolume.flip();
        }

        if (createDrawables && !nodePath.empty())
        {
            osg::Group* group = dynamic_cast<osg::Group*>(nodePath.back());
            if (group)
            {
            
                osg::Matrix invMV;
                invMV.invert(MV);
                
                transform(points,invMV);
                transform(farPoints,invMV);
            
                osg::Geode* geode = osgNew osg::Geode;
                group->addChild(geode);
                geode->addDrawable(createOccluderDrawable(points,farPoints));
            }
        }


        return true;
    }
    else
    {
        return false;
    }
}

bool ShadowVolumeOccluder::contains(const std::vector<Vec3>& vertices)
{
    if (_occluderVolume.containsAllOf(vertices))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(vertices)) return false;
        }
        return true;
    }
    return false;
}

bool ShadowVolumeOccluder::contains(const BoundingSphere& bound)
{
    if (_occluderVolume.containsAllOf(bound))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(bound)) return false;
        }
        return true;
    }
    return false;
}

bool ShadowVolumeOccluder::contains(const BoundingBox& bound)
{
    if (_occluderVolume.containsAllOf(bound))
    {
        for(HoleList::iterator itr=_holeList.begin();
            itr!=_holeList.end();
            ++itr)
        {
            if (itr->contains(bound)) return false;
        }
        return true;
    }
    return false;
}
