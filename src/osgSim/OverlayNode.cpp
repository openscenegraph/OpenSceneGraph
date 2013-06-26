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

#include <osg/ComputeBoundsVisitor>
#include <osg/Texture2D>
#include <osg/CoordinateSystemNode>
#include <osg/TexEnv>
#include <osg/Geometry>
#include <osg/io_utils>

#include <osgDB/FileUtils>

#include <osgUtil/CullVisitor>
#include <osgSim/OverlayNode>

using namespace osgSim;
using namespace osg;


namespace osgSim
{

class CustomPolytope
{
public:

    CustomPolytope() {}

    typedef std::vector<Vec3d> Vertices;

    struct Face
    {
        std::string name;
        osg::Plane  plane;
        Vertices    vertices;
    };

    Face& createFace() { _faces.push_back(Face()); return _faces.back(); }


    /** Create a Polytope which is a cube, centered at 0,0,0, with sides of 2 units.*/
    void setToUnitFrustum(bool withNear=true, bool withFar=true)
    {
        const osg::Vec3d v000(-1.0,-1.0,-1.0);
        const osg::Vec3d v010(-1.0,1.0,-1.0);
        const osg::Vec3d v001(-1.0,-1.0,1.0);
        const osg::Vec3d v011(-1.0,1.0,1.0);
        const osg::Vec3d v100(1.0,-1.0,-1.0);
        const osg::Vec3d v110(1.0,1.0,-1.0);
        const osg::Vec3d v101(1.0,-1.0,1.0);
        const osg::Vec3d v111(1.0,1.0,1.0);

        _faces.clear();

        {   // left plane.
            Face& face = createFace();
            face.name = "left";
            face.plane.set(1.0,0.0,0.0,1.0);
            face.vertices.reserve(4);
            face.vertices.push_back(v000);
            face.vertices.push_back(v001);
            face.vertices.push_back(v011);
            face.vertices.push_back(v010);
        }

        {   // right plane.
            Face& face = createFace();
            face.name = "right";
            face.plane.set(-1.0,0.0,0.0,1.0);
            face.vertices.reserve(4);
            face.vertices.push_back(v100);
            face.vertices.push_back(v110);
            face.vertices.push_back(v111);
            face.vertices.push_back(v101);
        }

        {   // bottom plane.
            Face& face = createFace();
            face.name = "bottom";
            face.plane.set(0.0,1.0,0.0,1.0);
            face.vertices.reserve(4);
            face.vertices.push_back(v000);
            face.vertices.push_back(v100);
            face.vertices.push_back(v101);
            face.vertices.push_back(v001);
        }

        {   // top plane.
            Face& face = createFace();
            face.name = "top";
            face.plane.set(0.0,-1.0,0.0,1.0);
            face.vertices.reserve(4);
            face.vertices.push_back(v111);
            face.vertices.push_back(v011);
            face.vertices.push_back(v010);
            face.vertices.push_back(v110);
        }

        if (withNear)
        {   // near plane
            Face& face = createFace();
            face.name = "near";
            face.plane.set(0.0,0.0,1.0,1.0);
            face.vertices.reserve(4);
            face.vertices.push_back(v000);
            face.vertices.push_back(v010);
            face.vertices.push_back(v110);
            face.vertices.push_back(v100);
        }

        if (withFar)
        {   // far plane
            Face& face = createFace();
            face.name = "far";
            face.plane.set(0.0,0.0,-1.0,1.0);
            face.vertices.reserve(4);
            face.vertices.push_back(v001);
            face.vertices.push_back(v101);
            face.vertices.push_back(v111);
            face.vertices.push_back(v011);
        }

    }

    void setToBoundingBox(const osg::BoundingBox& bb)
    {
#if 0
        OSG_NOTICE<<"setToBoundingBox xrange "<<bb.xMin()<<" "<<bb.xMax()<<std::endl;
        OSG_NOTICE<<"                        "<<bb.yMin()<<" "<<bb.yMax()<<std::endl;
        OSG_NOTICE<<"                        "<<bb.zMin()<<" "<<bb.zMax()<<std::endl;
#endif
        const osg::Vec3d v000(bb.xMin(),bb.yMin(), bb.zMin());
        const osg::Vec3d v010(bb.xMin(),bb.yMax(), bb.zMin());
        const osg::Vec3d v001(bb.xMin(),bb.yMin(), bb.zMax());
        const osg::Vec3d v011(bb.xMin(),bb.yMax(), bb.zMax());
        const osg::Vec3d v100(bb.xMax(),bb.yMin(), bb.zMin());
        const osg::Vec3d v110(bb.xMax(),bb.yMax(), bb.zMin());
        const osg::Vec3d v101(bb.xMax(),bb.yMin(), bb.zMax());
        const osg::Vec3d v111(bb.xMax(),bb.yMax(), bb.zMax());

        _faces.clear();

        {   // x min plane
            Face& face = createFace();
            face.name = "xMin";
            face.plane.set(1.0,0.0,0.0,-bb.xMin());
            face.vertices.reserve(4);
            face.vertices.push_back(v000);
            face.vertices.push_back(v001);
            face.vertices.push_back(v011);
            face.vertices.push_back(v010);
        }

        {   // x max plane.
            Face& face = createFace();
            face.name = "xMax";
            face.plane.set(-1.0,0.0,0.0,bb.xMax());
            face.vertices.reserve(4);
            face.vertices.push_back(v100);
            face.vertices.push_back(v110);
            face.vertices.push_back(v111);
            face.vertices.push_back(v101);
        }

        {   // y min plane.
            Face& face = createFace();
            face.name = "yMin";
            face.plane.set(0.0,1.0,0.0,-bb.yMin());
            face.vertices.reserve(4);
            face.vertices.push_back(v000);
            face.vertices.push_back(v100);
            face.vertices.push_back(v101);
            face.vertices.push_back(v001);
        }

        {   // y max plane.
            Face& face = createFace();
            face.name = "yMax";
            face.plane.set(0.0,-1.0,0.0,bb.yMax());
            face.vertices.reserve(4);
            face.vertices.push_back(v111);
            face.vertices.push_back(v011);
            face.vertices.push_back(v010);
            face.vertices.push_back(v110);
        }
        {   // z min plane
            Face& face = createFace();
            face.name = "zMin";
            face.plane.set(0.0,0.0,1.0,-bb.zMin());
            face.vertices.reserve(4);
            face.vertices.push_back(v000);
            face.vertices.push_back(v010);
            face.vertices.push_back(v110);
            face.vertices.push_back(v100);
        }

        {   // z max plane
            Face& face = createFace();
            face.name = "zMax";
            face.plane.set(0.0,0.0,-1.0,bb.zMax());
            face.vertices.reserve(4);
            face.vertices.push_back(v001);
            face.vertices.push_back(v101);
            face.vertices.push_back(v111);
            face.vertices.push_back(v011);
        }

    }

    void transform(const osg::Matrix& matrix, const osg::Matrix& inverse)
    {
        for(Faces::iterator itr = _faces.begin();
            itr != _faces.end();
            ++itr)
        {
            Face& face = *itr;
            face.plane.transformProvidingInverse(inverse);
            for(Vertices::iterator vitr = face.vertices.begin();
                vitr != face.vertices.end();
                ++vitr)
            {
                *vitr = *vitr * matrix;
            }
        }
    }


    void insertVertex(const osg::Vec3d& vertex, osg::EllipsoidModel* em=0, double minHeight=0.0)
    {
        // OSG_NOTICE<<"Inserting vertex "<<vertex<<std::endl;

        Faces removedFaces;

        Faces::iterator itr;
        for(itr = _faces.begin();
            itr != _faces.end();
            )
        {
            Face& face = *itr;
            if (face.plane.distance(vertex)<0.0)
            {
                removedFaces.push_back(face);
                itr = _faces.erase(itr);
            }
            else
            {
                ++itr;
            }
        }

        if (removedFaces.empty()) return;

        typedef std::pair<osg::Vec3d, osg::Vec3d> Edge;
        typedef std::map<Edge,int> Edges;
        Edges edges;

        double numVerticesAdded=0.0;
        osg::Vec3d center;
        for(itr = removedFaces.begin();
            itr != removedFaces.end();
            ++itr)
        {
            Face& face = *itr;
            Vertices& vertices = face.vertices;
            for(unsigned int i=0; i<vertices.size(); ++i)
            {
                osg::Vec3d& a = vertices[i];
                osg::Vec3d& b = vertices[ (i+1) % vertices.size()];
                if (a<b) ++edges[Edge(a,b)];
                else ++edges[Edge(b,a)];

                center += a;
                numVerticesAdded += 1.0;
            }
        }

        if (numVerticesAdded==0.0) return;
        center /= numVerticesAdded;

        typedef std::set<osg::Vec3> VertexSet;
        VertexSet uniqueVertices;

        for(Edges::iterator eitr = edges.begin();
            eitr != edges.end();
            ++eitr)
        {
            if (eitr->second==1)
            {
                // OSG_NOTICE<<"     edge Ok"<<std::endl;
                const Edge& edge = eitr->first;
                Face face;
                face.name = "baseSide";
                face.plane.set(vertex, edge.first, edge.second);
                face.vertices.push_back(vertex);
                face.vertices.push_back(edge.first);
                face.vertices.push_back(edge.second);
                if (face.plane.distance(center)<0.0) face.plane.flip();
                _faces.push_back(face);

                uniqueVertices.insert(edge.first);
                uniqueVertices.insert(edge.second);
            }
        }

        // now trim the new polytope back the desired height
        if (em)
        {
            // compute the base vertices at the new height
            Vertices baseVertices;
            for(VertexSet::iterator itr = uniqueVertices.begin();
                itr != uniqueVertices.end();
                ++itr)
            {
                const osg::Vec3d& point = *itr;
                double latitude, longitude, height;
                em->convertXYZToLatLongHeight(point.x(), point.y(), point.z(), latitude, longitude, height);
                osg::Vec3d normal(point);
                normal.normalize();
                baseVertices.push_back(point - normal * (height - minHeight));
            }

            //compute centroid of the base vertices
            osg::Vec3d center;
            double totalArea = 0;
            for(unsigned int i=0; i<baseVertices.size()-1; ++i)
            {
                const osg::Vec3d& a = baseVertices[i];
                const osg::Vec3d& b = baseVertices[i+1];
                const osg::Vec3d& c = baseVertices[(i+2)%baseVertices.size()];
                double area = ((a-b)^(b-c)).length()*0.5;
                osg::Vec3d localCenter = (a+b+c)/3.0;
                center += localCenter*area;
                totalArea += area;
            }
            center /= totalArea;
            osg::Vec3d normal(center);
            normal.normalize();

            osg::Plane basePlane(normal, center);

            cut(basePlane,"basePlane");
        }


        // OSG_NOTICE<<"  Removed faces "<<removedFaces.size()<<std::endl;
    }


    void projectDowntoBase(const osg::Vec3d& control, const osg::Vec3d& normal)
    {
        // OSG_NOTICE<<"CustomPolytope::projectDowntoBase not implementated yet."<<std::endl;

        Faces removedFaces;

        Faces::iterator itr;
        for(itr = _faces.begin();
            itr != _faces.end();
            )
        {
            Face& face = *itr;
            if ((face.plane.getNormal()*normal)>=0.0)
            {
                removedFaces.push_back(face);
                itr = _faces.erase(itr);
            }
            else
            {
                ++itr;
            }
        }

        if (removedFaces.empty()) return;

        typedef std::pair<osg::Vec3d, osg::Vec3d> Edge;
        typedef std::map<Edge,int> Edges;
        Edges edges;

        double numVerticesAdded=0.0;
        osg::Vec3d center;
        for(itr = removedFaces.begin();
            itr != removedFaces.end();
            ++itr)
        {
            Face& face = *itr;
            Vertices& vertices = face.vertices;
            for(unsigned int i=0; i<vertices.size(); ++i)
            {
                osg::Vec3d& a = vertices[i];
                osg::Vec3d& b = vertices[ (i+1) % vertices.size()];
                if (a<b) ++edges[Edge(a,b)];
                else ++edges[Edge(b,a)];

                center += a;
                numVerticesAdded += 1.0;
            }
        }

        if (numVerticesAdded==0.0) return;
        center /= numVerticesAdded;

        typedef std::set<osg::Vec3> VertexSet;
        VertexSet uniqueVertices;

        for(Edges::iterator eitr = edges.begin();
            eitr != edges.end();
            ++eitr)
        {
            if (eitr->second==1)
            {
                // OSG_NOTICE<<"     edge Ok"<<std::endl;
                const Edge& edge = eitr->first;

                double h_first = (edge.first-control) * normal;
                osg::Vec3d projected_first = edge.first - normal * h_first;

                double h_second = (edge.second-control) * normal;
                osg::Vec3d projected_second = edge.second - normal * h_second;

                Face face;
                face.name = "baseSide";
                face.plane.set(projected_first, edge.first, edge.second);
                face.vertices.push_back(projected_first);
                face.vertices.push_back(projected_second);
                face.vertices.push_back(edge.second);
                face.vertices.push_back(edge.first);

                if (face.plane.distance(center)<0.0) face.plane.flip();
                _faces.push_back(face);

                uniqueVertices.insert(projected_first);
                uniqueVertices.insert(projected_second);
            }
        }

        Face newFace;
        newFace.name = "basePlane";
        newFace.plane.set(normal,control);

        osg::Vec3d side = ( fabs(normal.x()) < fabs(normal.y()) ) ?
                          osg::Vec3(1.0, 0.0, 0.0) :
                          osg::Vec3(0.0, 1.0, 0.0);

        osg::Vec3 v = normal ^ side;
        v.normalize();

        osg::Vec3 u = v ^ normal;
        u.normalize();

        typedef std::map<double, Vec3d> AnglePositions;
        AnglePositions anglePositions;
        for(VertexSet::iterator vitr = uniqueVertices.begin();
            vitr != uniqueVertices.end();
            ++vitr)
        {
            osg::Vec3d delta = *vitr - center;
            double angle = atan2(delta * u, delta * v);
            if (angle<0.0) angle += 2.0*osg::PI;
            anglePositions[angle] = *vitr;
        }

        for(AnglePositions::iterator aitr = anglePositions.begin();
            aitr != anglePositions.end();
            ++aitr)
        {
            newFace.vertices.push_back(aitr->second);
        }

        _faces.push_back(newFace);


    }

    void computeSilhoette(const osg::Vec3d& normal, Vertices& vertices)
    {

        typedef std::pair<osg::Vec3d, osg::Vec3d> EdgePair;
        typedef std::vector<Face*> EdgeFaces;
        typedef std::map<EdgePair, EdgeFaces> EdgeMap;

        EdgeMap edgeMap;


        for(Faces::iterator itr = _faces.begin();
            itr != _faces.end();
            ++itr)
        {
            Face& face = *itr;
            for(unsigned int i=0; i<face.vertices.size(); ++i)
            {
                osg::Vec3d& va = face.vertices[i];
                osg::Vec3d& vb = face.vertices[(i+1)%face.vertices.size()];
                if (va < vb) edgeMap[EdgePair(va,vb)].push_back(&face);
                else edgeMap[EdgePair(vb,va)].push_back(&face);
            }
        }

        typedef std::set<osg::Vec3> VertexSet;
        VertexSet uniqueVertices;

        for(EdgeMap::iterator eitr = edgeMap.begin();
            eitr != edgeMap.end();
            ++eitr)
        {
            const EdgePair& edge = eitr->first;
            const EdgeFaces& edgeFaces = eitr->second;
            if (edgeFaces.size()==1)
            {
                // OSG_NOTICE<<"Open edge found "<<edgeFaces.front()->name<<std::endl;
            }
            else if (edgeFaces.size()==2)
            {

                double dotProduct0 = edgeFaces[0]->plane.getNormal() * normal;
                double dotProduct1 = edgeFaces[1]->plane.getNormal() * normal;
                if (dotProduct0 * dotProduct1 <0.0)
                {
                    // OSG_NOTICE<<"  Silhoette edge found "<<edgeFaces[0]->name<<" "<<edgeFaces[1]->name<<std::endl;
                    uniqueVertices.insert(edge.first);
                    uniqueVertices.insert(edge.second);
                }
                else
                {
                    // OSG_NOTICE<<"  Non silhoette edge found "<<edgeFaces[0]->name<<" "<<edgeFaces[1]->name<<std::endl;
                }

            }
            else
            {
                // OSG_NOTICE<<"Confused edge found "<<edgeFaces.size()<<std::endl;
            }
        }


        // compute center
        osg::Vec3d center;

        VertexSet::iterator vitr;
        for(vitr = uniqueVertices.begin();
            vitr != uniqueVertices.end();
            ++vitr)
        {
            center += *vitr;
        }
        center /= (double)(uniqueVertices.size());


        // compute the ordered points around silhoette
        osg::Vec3d side = ( fabs(normal.x()) < fabs(normal.y()) ) ?
                          osg::Vec3(1.0, 0.0, 0.0) :
                          osg::Vec3(0.0, 1.0, 0.0);

        osg::Vec3 v = normal ^ side;
        v.normalize();

        osg::Vec3 u = v ^ normal;
        u.normalize();

        typedef std::map<double, Vec3d> AnglePositions;
        AnglePositions anglePositions;
        for(vitr = uniqueVertices.begin();
            vitr != uniqueVertices.end();
            ++vitr)
        {
            osg::Vec3d delta = *vitr - center;
            double angle = atan2(delta * u, delta * v);
            if (angle<0.0) angle += 2.0*osg::PI;
            anglePositions[angle] = *vitr;
        }

        for(AnglePositions::iterator aitr = anglePositions.begin();
            aitr != anglePositions.end();
            ++aitr)
        {
            vertices.push_back(aitr->second);
        }


    }


    void cut(const osg::Polytope& polytope)
    {
        // OSG_NOTICE<<"Cutting with polytope "<<std::endl;
        for(osg::Polytope::PlaneList::const_iterator itr = polytope.getPlaneList().begin();
            itr != polytope.getPlaneList().end();
            ++itr)
        {
            cut(*itr);
        }
    }

    void cut(const CustomPolytope& polytope)
    {
        // OSG_NOTICE<<"Cutting with polytope "<<std::endl;
        for(Faces::const_iterator itr = polytope._faces.begin();
            itr != polytope._faces.end();
            ++itr)
        {
            cut(itr->plane, itr->name);
        }
    }

    void cut(const osg::Plane& plane, const std::string& name=std::string())
    {
        // OSG_NOTICE<<"  Cutting plane "<<plane<<std::endl;
        Face newFace;
        typedef std::vector<double> Distances;
        Distances distances;
        Vertices newVertices;

        for(Faces::iterator itr = _faces.begin();
            itr != _faces.end();
            )
        {
            Face& face = *itr;
            int intersect = plane.intersect(face.vertices);
            if (intersect==1)
            {
                // OSG_NOTICE<<"    Face inside"<<std::endl;
                ++itr;
            }
            else if (intersect==0)
            {
                // OSG_NOTICE<<"    Face intersecting - before "<<face.vertices.size()<<std::endl;

                Vertices& vertices = face.vertices;
                newVertices.clear();

                distances.clear();
                distances.reserve(face.vertices.size());
                for(Vertices::iterator vitr = vertices.begin();
                    vitr != vertices.end();
                    ++vitr)
                {
                    distances.push_back(plane.distance(*vitr));
                }

                for(unsigned int i=0; i<vertices.size(); ++i)
                {
                    osg::Vec3d& va = vertices[i];
                    osg::Vec3d& vb = vertices[(i+1)%vertices.size()];
                    double distance_a = distances[i];
                    double distance_b = distances[(i+1)%vertices.size()];

                    // is first edge point inside plane?
                    if (distance_a>=0.0) newVertices.push_back(vertices[i]);

                    // add point to new face if point exactly on a plane
                    if (distance_a==0.0) newFace.vertices.push_back(vertices[i]);

                    // does edge intersect plane
                    if (distance_a * distance_b<0.0)
                    {
                        // inserting vertex
                        osg::Vec3d intersection = (vb*distance_a - va*distance_b)/(distance_a-distance_b);
                        newVertices.push_back(intersection);
                        newFace.vertices.push_back(intersection);

                        // OSG_NOTICE<<"  intersection distance "<<plane.distance(intersection)<<std::endl;
                    }
                }

                vertices.swap(newVertices);

                // OSG_NOTICE<<"        intersecting - after "<<face.vertices.size()<<std::endl;

                ++itr;
            }
            else if (intersect==-1)
            {
                // OSG_NOTICE<<"    Face outside"<<_faces.size()<<std::endl;
                itr = _faces.erase(itr);
            }
        }

        if (!newFace.vertices.empty())
        {
            // OSG_NOTICE<<"    inserting newFace intersecting "<<newFace.vertices.size()<<std::endl;
            newFace.name = name;
            newFace.plane = plane;

            Vertices& vertices = newFace.vertices;

            osg::Vec3d side = ( fabs(plane.getNormal().x()) < fabs(plane.getNormal().y()) ) ?
                              osg::Vec3(1.0, 0.0, 0.0) :
                              osg::Vec3(0.0, 1.0, 0.0);

            osg::Vec3 v = plane.getNormal() ^ side;
            v.normalize();

            osg::Vec3 u = v ^ plane.getNormal();
            u.normalize();

            osg::Vec3d center;
            Vertices::iterator vitr;
            for(vitr = vertices.begin();
                vitr != vertices.end();
                ++vitr)
            {
                center += *vitr;
            }
            center /= vertices.size();

            typedef std::map<double, Vec3d> AnglePositions;
            AnglePositions anglePositions;
            for(vitr = vertices.begin();
                vitr != vertices.end();
                ++vitr)
            {
                osg::Vec3d delta = *vitr - center;
                double angle = atan2(delta * u, delta * v);
                if (angle<0.0) angle += 2.0*osg::PI;
                anglePositions[angle] = *vitr;
            }


            newVertices.clear();
            newVertices.reserve(anglePositions.size());
            for(AnglePositions::iterator aitr = anglePositions.begin();
                aitr != anglePositions.end();
                ++aitr)
            {
                newVertices.push_back(aitr->second);
            }

            newVertices.swap(vertices);

            // OSG_NOTICE<<"     after angle sort  "<<newFace.vertices.size()<<std::endl;

            _faces.push_back(newFace);
        }
    }

    void getPolytope(osg::Polytope& polytope)
    {
        for(Faces::const_iterator itr = _faces.begin();
            itr != _faces.end();
            ++itr)
        {
            polytope.add(itr->plane);
        }
    }

    void getPoints(Vertices& vertices)
    {
        typedef std::set<osg::Vec3d> VerticesSet;
        VerticesSet verticesSet;
        for(Faces::iterator itr = _faces.begin();
            itr != _faces.end();
            ++itr)
        {
            Face& face = *itr;
            for(Vertices::iterator vitr = face.vertices.begin();
                vitr != face.vertices.end();
                ++vitr)
            {
                verticesSet.insert(*vitr);
            }
        }

        for(VerticesSet::iterator sitr = verticesSet.begin();
            sitr != verticesSet.end();
            ++sitr)
        {
            vertices.push_back(*sitr);
        }
    }


    osg::Drawable* createDrawable(const osg::Vec4d& colour)
    {
        osg::Geometry* geometry = new osg::Geometry;
        osg::Vec3Array* vertices = new osg::Vec3Array;
        geometry->setVertexArray(vertices);

        for(Faces::iterator itr = _faces.begin();
            itr != _faces.end();
            ++itr)
        {
            Face& face = *itr;
            geometry->addPrimitiveSet( new osg::DrawArrays(GL_LINE_LOOP, vertices->size(), face.vertices.size()) );
            for(Vertices::iterator vitr = face.vertices.begin();
                vitr != face.vertices.end();
                ++vitr)
            {
                vertices->push_back(*vitr);
            }
        }

        osg::Vec4Array* colours = new osg::Vec4Array;
        colours->push_back(colour);
        geometry->setColorArray(colours, osg::Array::BIND_OVERALL);

        osg::StateSet* stateset = geometry->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF);
        stateset->setTextureMode(1, GL_TEXTURE_2D, osg::StateAttribute::OFF);

        return geometry;
    }

protected:

    typedef std::list<Face> Faces;
    Faces _faces;


};

}


OverlayNode::OverlayNode(OverlayTechnique technique):
    _overlayTechnique(technique),
    _texEnvMode(GL_DECAL),
    _textureUnit(1),
    _textureSizeHint(1024),
    _overlayClearColor(0.0f,0.0f,0.0f,0.0f),
    _continuousUpdate(false),
    _overlayBaseHeight(-100.0),
    _updateCamera(false),
    _renderTargetImpl(osg::Camera::FRAME_BUFFER_OBJECT)
{
    setNumChildrenRequiringUpdateTraversal(1);
    init();
}

OverlayNode::OverlayNode(const OverlayNode& copy, const osg::CopyOp& copyop):
    osg::Group(copy,copyop),
    _overlayTechnique(copy._overlayTechnique),
    _overlaySubgraph(copy._overlaySubgraph),
    _texEnvMode(copy._texEnvMode),
    _textureUnit(copy._textureUnit),
    _textureSizeHint(copy._textureSizeHint),
    _overlayClearColor(copy._overlayClearColor),
    _continuousUpdate(copy._continuousUpdate),
    _overlayBaseHeight(copy._overlayBaseHeight),
    _updateCamera(false),
    _renderTargetImpl(copy._renderTargetImpl)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
    init();
}

void OverlayNode::OverlayData::setThreadSafeRefUnref(bool threadSafe)
{
    if (_camera.valid()) _camera->setThreadSafeRefUnref(threadSafe);
    if (_texgenNode.valid()) _texgenNode->setThreadSafeRefUnref(threadSafe);
    if (_overlayStateSet.valid()) _overlayStateSet->setThreadSafeRefUnref(threadSafe);
    if (_mainSubgraphStateSet.valid()) _mainSubgraphStateSet->setThreadSafeRefUnref(threadSafe);
    if (_texture.valid()) _texture->setThreadSafeRefUnref(threadSafe);
}

void OverlayNode::OverlayData::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_camera.valid()) _camera->resizeGLObjectBuffers(maxSize);
    if (_texgenNode.valid()) _texgenNode->resizeGLObjectBuffers(maxSize);
    if (_overlayStateSet.valid()) _overlayStateSet->resizeGLObjectBuffers(maxSize);
    if (_mainSubgraphStateSet.valid()) _mainSubgraphStateSet->resizeGLObjectBuffers(maxSize);
    if (_texture.valid()) _texture->resizeGLObjectBuffers(maxSize);
}

void OverlayNode::OverlayData::releaseGLObjects(osg::State* state) const
{
    if (_camera.valid()) _camera->releaseGLObjects(state);
    if (_texgenNode.valid()) _texgenNode->releaseGLObjects(state);
    if (_overlayStateSet.valid()) _overlayStateSet->releaseGLObjects(state);
    if (_mainSubgraphStateSet.valid()) _mainSubgraphStateSet->releaseGLObjects(state);
    if (_texture.valid()) _texture->releaseGLObjects(state);
}

void OverlayNode::setThreadSafeRefUnref(bool threadSafe)
{
    osg::Group::setThreadSafeRefUnref(threadSafe);

    if (_overlaySubgraph.valid()) _overlaySubgraph->setThreadSafeRefUnref(threadSafe);

    for(OverlayDataMap::iterator itr = _overlayDataMap.begin();
        itr != _overlayDataMap.end();
        ++itr)
    {
        itr->second->setThreadSafeRefUnref(threadSafe);
    }
}

void OverlayNode::resizeGLObjectBuffers(unsigned int maxSize)
{
    osg::Group::resizeGLObjectBuffers(maxSize);

    if (_overlaySubgraph.valid()) _overlaySubgraph->resizeGLObjectBuffers(maxSize);

    for(OverlayDataMap::iterator itr = _overlayDataMap.begin();
        itr != _overlayDataMap.end();
        ++itr)
    {
        itr->second->resizeGLObjectBuffers(maxSize);
    }
}

void OverlayNode::releaseGLObjects(osg::State* state) const
{
    osg::Group::releaseGLObjects(state);

    if (_overlaySubgraph.valid()) _overlaySubgraph->releaseGLObjects(state);

    for(OverlayDataMap::const_iterator itr = _overlayDataMap.begin();
        itr != _overlayDataMap.end();
        ++itr)
    {
        itr->second->releaseGLObjects(state);
    }
}

void OverlayNode::setOverlayTechnique(OverlayTechnique technique)
{
    if (_overlayTechnique==technique) return;

    _overlayTechnique = technique;

    init();
}

void OverlayNode::setRenderTargetImplementation(osg::Camera::RenderTargetImplementation impl)
{
    if(_renderTargetImpl==impl) return;

    _renderTargetImpl = impl;

    init();
    for(OverlayDataMap::const_iterator itr = _overlayDataMap.begin();
      itr != _overlayDataMap.end();
      ++itr)
    {
      itr->second->_camera->setRenderTargetImplementation(_renderTargetImpl);
    }
}

OverlayNode::OverlayData* OverlayNode::getOverlayData(osgUtil::CullVisitor* cv)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_overlayDataMapMutex);
    OverlayDataMap::iterator itr = _overlayDataMap.find(cv);
    if (itr != _overlayDataMap.end()) return itr->second.get();

    _overlayDataMap[cv] = new OverlayData;

    OverlayData* overlayData = _overlayDataMap[cv].get();


    unsigned int tex_width = _textureSizeHint;
    unsigned int tex_height = _textureSizeHint;

    if (!overlayData->_texture)
    {
        // OSG_NOTICE<<"   setting up texture"<<std::endl;

        osg::Texture2D* texture = new osg::Texture2D;
        texture->setTextureSize(tex_width, tex_height);
        texture->setInternalFormat(GL_RGBA);
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
#if 1
        texture->setBorderColor(osg::Vec4(_overlayClearColor));
#else
        texture->setBorderColor(osg::Vec4(1.0,0.0,0.0,0.5));
#endif
        overlayData->_texture = texture;
    }

    // set up the render to texture camera.
    if (!overlayData->_camera || overlayData->_camera->getRenderTargetImplementation() != _renderTargetImpl)
    {
        // OSG_NOTICE<<"   setting up camera"<<std::endl;

        // create the camera
        overlayData->_camera = new osg::Camera;

        overlayData->_camera->setClearColor(_overlayClearColor);

        overlayData->_camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);

        // set viewport
        overlayData->_camera->setViewport(0,0,tex_width,tex_height);

        overlayData->_camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        // set the camera to render before the main camera.
        overlayData->_camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        overlayData->_camera->setRenderTargetImplementation(_renderTargetImpl);

        // attach the texture and use it as the color buffer.
        overlayData->_camera->attach(osg::Camera::COLOR_BUFFER, overlayData->_texture.get());

        if (_overlaySubgraph.valid()) overlayData->_camera->addChild(_overlaySubgraph.get());
    }

    if (!overlayData->_texgenNode)
    {
        overlayData->_texgenNode = new osg::TexGenNode;
        overlayData->_texgenNode->setTextureUnit(_textureUnit);
    }

    if (!overlayData->_y0) overlayData->_y0 = new osg::Uniform("y0",0.0f);
    if (!overlayData->_lightingEnabled) overlayData->_lightingEnabled = new osg::Uniform("lightingEnabled",true);

    if (!overlayData->_overlayStateSet)
    {
        overlayData->_overlayStateSet = new osg::StateSet;
        overlayData->_overlayStateSet->addUniform(overlayData->_y0.get());
        overlayData->_overlayStateSet->addUniform(overlayData->_lightingEnabled.get());

        osg::Program* program = new osg::Program;
        overlayData->_overlayStateSet->setAttribute(program);

        // get shaders from source
        std::string vertexShaderFile = osgDB::findDataFile("shaders/overlay_perspective_rtt.vert");
        if (!vertexShaderFile.empty())
        {
            program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, vertexShaderFile));
        }
        else
        {
            char vertexShaderSource[] =
                "uniform float y0; \n"
                "uniform bool lightingEnabled; \n"
                " \n"
                "vec4 warp(in vec4 source) \n"
                "{ \n"
                "    float divisor = source.y + y0; \n"
                "    return vec4(source.x * (1.0 + y0 ), source.y * y0 + 1.0, (source.z * y0 + 1.0)*0.01, source.w * divisor); \n"
                "} \n"
                " \n"
                "vec3 fnormal(void) \n"
                "{ \n"
                "    //Compute the normal  \n"
                "    vec3 normal = gl_NormalMatrix * gl_Normal; \n"
                "    normal = normalize(normal); \n"
                "    return normal; \n"
                "} \n"
                " \n"
                "void directionalLight(in int i, \n"
                "                      in vec3 normal, \n"
                "                      inout vec4 ambient, \n"
                "                      inout vec4 diffuse, \n"
                "                      inout vec4 specular) \n"
                "{ \n"
                "   float nDotVP;         // normal . light direction \n"
                "   float nDotHV;         // normal . light half vector \n"
                "   float pf;             // power factor \n"
                " \n"
                "   nDotVP = max(0.0, dot(normal, normalize(vec3 (gl_LightSource[i].position)))); \n"
                "   nDotHV = max(0.0, dot(normal, vec3 (gl_LightSource[i].halfVector))); \n"
                " \n"
                "   if (nDotVP == 0.0) \n"
                "   { \n"
                "       pf = 0.0; \n"
                "   } \n"
                "   else \n"
                "   { \n"
                "       pf = pow(nDotHV, gl_FrontMaterial.shininess); \n"
                " \n"
                "   } \n"
                "   ambient  += gl_LightSource[i].ambient; \n"
                "   diffuse  += gl_LightSource[i].diffuse * nDotVP; \n"
                "   specular += gl_LightSource[i].specular * pf; \n"
                "} \n"
                "void main() \n"
                "{ \n"
                "    gl_Position = warp(ftransform()); \n"
                " \n"
                "    if (lightingEnabled) \n"
                "    {     \n"
                "        vec4 ambient = vec4(0.0); \n"
                "        vec4 diffuse = vec4(0.0); \n"
                "        vec4 specular = vec4(0.0); \n"
                " \n"
                "        vec3 normal = fnormal(); \n"
                " \n"
                "        directionalLight(0, normal, ambient, diffuse, specular); \n"
                " \n"
                "        vec4 color = gl_FrontLightModelProduct.sceneColor + \n"
                "                     ambient  * gl_FrontMaterial.ambient + \n"
                "                     diffuse  * gl_FrontMaterial.diffuse + \n"
                "                     specular * gl_FrontMaterial.specular; \n"
                " \n"
                "        gl_FrontColor = color; \n"
                " \n"
                "    } \n"
                "    else \n"
                "    { \n"
                "        gl_FrontColor = gl_Color; \n"
                "    } \n"
                "   \n"
                "} \n";

            osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
            program->addShader(vertex_shader);
        }

    }

    if (!overlayData->_mainSubgraphProgram)
    {
        overlayData->_mainSubgraphProgram = new osg::Program;

        // get shaders from source
        std::string fragmentShaderFile = osgDB::findDataFile("shaders/overlay_perspective_main.frag");
        if (!fragmentShaderFile.empty())
        {
            overlayData->_mainSubgraphProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, fragmentShaderFile));
        }
        else
        {
            char fragmentShaderSource[] =
                "uniform sampler2D texture_0; \n"
                "uniform sampler2D texture_1; \n"
                " \n"
                "uniform float y0; \n"
                " \n"
                "vec2 warp(in vec2 source) \n"
                "{ \n"
                "    float inv_divisor = 1.0 / (source.y + y0); \n"
                "    return vec2(source.x * (1.0 + y0 ) * inv_divisor , (source.y * y0 + 1.0 ) * inv_divisor); \n"
                "} \n"
                " \n"
                "void main() \n"
                "{ \n"
                "    vec2 coord = gl_TexCoord[1].xy; \n"
                "    coord.x = coord.x*2.0 - 1.0; \n"
                "    coord.y = coord.y*2.0 - 1.0; \n"
                " \n"
                "    vec2 warped = warp(coord); \n"
                "    warped.x = (warped.x + 1.0)*0.5; \n"
                "    warped.y = (warped.y + 1.0)*0.5; \n"
                " \n"
                "    vec4 base_color = texture2D(texture_0, gl_TexCoord[0].xy); \n"
                "    vec4 overlay_color = texture2D(texture_1, warped ); \n"
                "    vec3 mixed_color = mix(base_color.rgb, overlay_color.rgb, overlay_color.a); \n"
                "    gl_FragColor = vec4(mixed_color, base_color.a); \n"
                "} \n";

            osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
            overlayData->_mainSubgraphProgram->addShader(fragment_shader);
        }
    }

    if (!overlayData->_mainSubgraphStateSet)
    {
        overlayData->_mainSubgraphStateSet = new osg::StateSet;

        overlayData->_mainSubgraphStateSet->addUniform(overlayData->_y0.get());
        overlayData->_mainSubgraphStateSet->addUniform(new osg::Uniform("texture_0",0));
        overlayData->_mainSubgraphStateSet->addUniform(new osg::Uniform("texture_1",1));

        overlayData->_mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, overlayData->_texture.get(), osg::StateAttribute::ON);
        overlayData->_mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
        overlayData->_mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
        overlayData->_mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
        overlayData->_mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

        if (_texEnvMode!=GL_NONE)
        {
            overlayData->_mainSubgraphStateSet->setTextureAttribute(_textureUnit, new osg::TexEnv((osg::TexEnv::Mode)_texEnvMode));
        }
    }

    return overlayData;
}

void OverlayNode::init()
{
    switch(_overlayTechnique)
    {
        case(OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY):
            init_OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY();
            break;
        case(VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY):
            init_VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY();
            break;
        case(VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY):
            init_VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY();
            break;
    }
}

void OverlayNode::init_OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY()
{
    OSG_INFO<<"OverlayNode::init() - OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<<std::endl;

    // force initialization of _overlayDataMap for the key 0 (or NULL)
    getOverlayData(0);
}

void OverlayNode::init_VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY()
{
    OSG_INFO<<"OverlayNode::init() - VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<<std::endl;
}

void OverlayNode::init_VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY()
{
    OSG_INFO<<"OverlayNode::init() - VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY"<<std::endl;
}

void OverlayNode::traverse(osg::NodeVisitor& nv)
{
    switch(_overlayTechnique)
    {
        case(OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY):
            traverse_OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY(nv);
            break;
        case(VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY):
            traverse_VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY(nv);
            break;
        case(VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY):
            traverse_VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY(nv);
            break;
    }
}

void OverlayNode::traverse_OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY(osg::NodeVisitor& nv)
{
    OverlayData& overlayData = *getOverlayData(0);
    osg::Camera* camera = overlayData._camera.get();

    if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    {

        Group::traverse(nv);

        if (_continuousUpdate || _updateCamera)
        {


            // now compute the camera's view and projection matrix to point at the shadower (the camera's children)
            osg::BoundingSphere bs;
            for(unsigned int i=0; i<camera->getNumChildren(); ++i)
            {
                bs.expandBy(camera->getChild(i)->getBound());
            }

            if (bs.valid())
            {
                // see if we are within a coordinate system node.
                osg::CoordinateSystemNode* csn = 0;
                osg::NodePath& nodePath = nv.getNodePath();
                for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
                    itr != nodePath.rend() && csn==0;
                    ++itr)
                {
                    csn = dynamic_cast<osg::CoordinateSystemNode*>(*itr);
                }

                osg::EllipsoidModel* em = csn ? csn->getEllipsoidModel() : 0;


                if (em)
                {
                    osg::Vec3d eyePoint(0.0,0.0,0.0); // center of the planet
                    double centerDistance = (eyePoint-osg::Vec3d(bs.center())).length();

                    double znear = centerDistance-bs.radius();
                    double zfar  = centerDistance+bs.radius();
                    double zNearRatio = 0.001f;
                    if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

                    double top   = (bs.radius()/centerDistance)*znear;
                    double right = top;

                    camera->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
                    camera->setViewMatrixAsLookAt(eyePoint, bs.center(), osg::Vec3(0.0f,1.0f,0.0f));
                }
                else
                {
                    osg::Vec3d upDirection(0.0,1.0,0.0);
                    osg::Vec3d viewDirection(0.0,0.0,1.0);

                    double viewDistance = 2.0*bs.radius();
                    osg::Vec3d center = bs.center();
                    osg::Vec3d eyePoint = center+viewDirection*viewDistance;

                    double znear = viewDistance-bs.radius();
                    double zfar  = viewDistance+bs.radius();

                    float top   = bs.radius();
                    float right = top;

                    camera->setProjectionMatrixAsOrtho(-right,right,-top,top,znear,zfar);
                    camera->setViewMatrixAsLookAt(eyePoint,center,upDirection);

                }


                // compute the matrix which takes a vertex from local coords into tex coords
                // will use this later to specify osg::TexGen..
                osg::Matrix MVP = camera->getViewMatrix() *
                                  camera->getProjectionMatrix();

                osg::Matrix MVPT = MVP *
                                   osg::Matrix::translate(1.0,1.0,1.0) *
                                   osg::Matrix::scale(0.5,0.5,0.5);

                overlayData._texgenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
                overlayData._texgenNode->getTexGen()->setPlanesFromMatrix(MVPT);

                overlayData._textureFrustum.setToUnitFrustum(false,false);
                overlayData._textureFrustum.transformProvidingInverse(MVP);
            }
            _updateCamera = false;
        }

        return;
    }

    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        Group::traverse(nv);
        return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (!cv)
    {
        Group::traverse(nv);
        return;
    }


    unsigned int contextID = cv->getState()!=0 ? cv->getState()->getContextID() : 0;

    // if we need to redraw then do cull traversal on camera.
    if (!_textureObjectValidList[contextID] || _continuousUpdate)
    {
        camera->setClearColor(_overlayClearColor);
        camera->accept(*cv);
        _textureObjectValidList[contextID] = 1;
    }


    // now set up the drawing of the main scene.
    {

        overlayData._texgenNode->accept(*cv);

        const osg::Matrix modelView = *(cv->getModelViewMatrix());
        osg::Polytope viewTextureFrustum;
        viewTextureFrustum.setAndTransformProvidingInverse(overlayData._textureFrustum, osg::Matrix::inverse(modelView));

        cv->getProjectionCullingStack().back().addStateFrustum(overlayData._mainSubgraphStateSet.get(), viewTextureFrustum);
        cv->getCurrentCullingSet().addStateFrustum(overlayData._mainSubgraphStateSet.get(), overlayData._textureFrustum);

        // push the stateset.
        // cv->pushStateSet(_mainSubgraphStateSet.get());

        Group::traverse(nv);

        // cv->popStateSet();

        cv->getCurrentCullingSet().getStateFrustumList().pop_back();
        cv->getProjectionCullingStack().back().getStateFrustumList().pop_back();
    }
}

void OverlayNode::traverse_VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY(osg::NodeVisitor& nv)
{
    // OSG_NOTICE<<"OverlayNode::traverse() - VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<<std::endl;


    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        Group::traverse(nv);
        return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (!cv)
    {
        Group::traverse(nv);
        return;
    }

    OverlayData& overlayData = *getOverlayData(cv);
    osg::Camera* camera = overlayData._camera.get();

    if (_overlaySubgraph.valid())
    {

#if 0
        if (!overlayData._geode)
        {
            overlayData._geode = new osg::Geode;
        }
#endif
        // see if we are within a coordinate system node.
        osg::CoordinateSystemNode* csn = 0;
        osg::NodePath& nodePath = nv.getNodePath();
        for(osg::NodePath::reverse_iterator itr = nodePath.rbegin();
            itr != nodePath.rend() && csn==0;
            ++itr)
        {
            csn = dynamic_cast<osg::CoordinateSystemNode*>(*itr);
        }

        osg::EllipsoidModel* em = csn ? csn->getEllipsoidModel() : 0;

        osg::BoundingSphere bs = _overlaySubgraph->getBound();

        // push the stateset.
        cv->pushStateSet(overlayData._mainSubgraphStateSet.get());

        Group::traverse(nv);

        cv->popStateSet();

        osg::Matrix pm = *(cv->getProjectionMatrix());

        double znear = cv->getCalculatedNearPlane();
        double zfar = cv->getCalculatedFarPlane();

        // OSG_NOTICE<<" before znear ="<<znear<<"\t zfar ="<<zfar<<std::endl;

        cv->computeNearPlane();

        znear = cv->getCalculatedNearPlane();
        zfar = cv->getCalculatedFarPlane();

        // OSG_NOTICE<<" after znear ="<<znear<<"\t zfar ="<<zfar<<std::endl;

        // OSG_NOTICE<<" before clamp pm="<<pm<<std::endl;

        cv->clampProjectionMatrixImplementation(pm, znear,zfar);

        // OSG_NOTICE<<" after clamp pm="<<pm<<std::endl;

        osg::Matrix MVP = *(cv->getModelViewMatrix()) * pm;
        osg::Matrix inverseMVP;
        inverseMVP.invert(MVP);

        // create polytope for the view frustum in local coords
        CustomPolytope frustum;
#if 0
        frustum.setToUnitFrustum(false, false);
#else
        frustum.setToUnitFrustum(true, true);
#endif
        frustum.transform(inverseMVP, MVP);


        // create polytope for the overlay subgraph in local coords
        CustomPolytope overlayPolytope;

        // get the bounds of the model.
        osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
        _overlaySubgraph->accept(cbbv);
        overlayPolytope.setToBoundingBox(cbbv.getBoundingBox());

        if (em)
        {
            overlayPolytope.insertVertex(osg::Vec3d(0.0,0.0,0.0), em, _overlayBaseHeight);
        }
        else
        {
            overlayPolytope.projectDowntoBase(osg::Vec3d(0.0,0.0,_overlayBaseHeight), osg::Vec3d(0.0,0.0,1.0));
        }


        if (overlayData._geode.valid() && overlayData._geode->getNumDrawables()>100)
        {
            overlayData._geode->removeDrawables(0, 3);
        }

        if (overlayData._geode.valid())
        {
            overlayData._geode->addDrawable(overlayPolytope.createDrawable(osg::Vec4d(1.0f,1.0f,0.0f,1.0f)));
            overlayData._geode->addDrawable(frustum.createDrawable(osg::Vec4d(0.0f,0.0f,1.0f,1.0f)));
        }





        // OSG_NOTICE<<"AFTER CUT corners = "<<corners.size()<<std::endl;


        osg::Vec3d center = _overlaySubgraph->getBound().center();

        osg::Vec3d lookVector(0.0,0.0,-1.0);
        if (em)
        {
            lookVector = -center;
            lookVector.normalize();
        }

        osg::Vec3d sideVectorLV = lookVector ^ cv->getLookVectorLocal();
        osg::Vec3d sideVectorUP = lookVector ^ cv->getUpLocal();

        osg::Vec3d sideVector = sideVectorLV.length() > sideVectorUP.length() ?
            sideVectorLV :
            sideVectorUP;

        sideVector.normalize();

        osg::Vec3d upVector = sideVector ^ lookVector;
        upVector.normalize();

        osg::Vec3d overlayLookVector = upVector ^ sideVector;
        overlayLookVector.normalize();


        overlayPolytope.cut(frustum);

        CustomPolytope::Vertices corners;
#if 0
        overlayPolytope.getPoints(corners);
#else
        overlayPolytope.computeSilhoette(lookVector, corners);
#endif
        if (corners.empty())
        {
            camera->setClearColor(_overlayClearColor);
            //camera->setStateSet(overlayData._overlayStateSet.get());
            camera->accept(*cv);
            return;
        }

        if (overlayData._geode.valid())
        {
            overlayData._geode->addDrawable(overlayPolytope.createDrawable(osg::Vec4d(1.0f,1.0f,1.0f,1.0f)));
        }


        // OSG_NOTICE<<"    lookVector ="<<lookVector<<std::endl;

        double min_side = DBL_MAX;
        double max_side = -DBL_MAX;
        double min_up = DBL_MAX;
        double max_up = -DBL_MAX;
        double min_distanceEye = DBL_MAX;
        double max_distanceEye = -DBL_MAX;

        double zNear = -bs.radius();
        double zFar = bs.radius();

        typedef std::vector<osg::Vec2d> ProjectedVertices;
        ProjectedVertices projectedVertices;

        osg::Vec3 eyeLocal = cv->getEyeLocal();


        // computed the expected near/far ratio
        unsigned int i;
        for(i=0; i< corners.size(); ++i)
        {
            double distanceEye = (corners[i] - eyeLocal).length();
            if (distanceEye < min_distanceEye) min_distanceEye = distanceEye;
            if (distanceEye > max_distanceEye) max_distanceEye = distanceEye;

            osg::Vec3d delta = corners[i] - center;
            double distance_side = delta * sideVector;
            double distance_up = delta * upVector;
            projectedVertices.push_back(osg::Vec2d(distance_side, distance_up));

            if (distance_side<min_side)
            {
                min_side = distance_side;
            }
            if (distance_side>max_side)
            {
                max_side = distance_side;
            }
            if (distance_up<min_up) min_up = distance_up;
            if (distance_up>max_up) max_up = distance_up;
        }

        double mid_side = (min_side + max_side) * 0.5;
        double ratio = min_distanceEye / max_distanceEye;
        bool usePerspectiveShaders = (_overlayTechnique==VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY);

        if (usePerspectiveShaders)
        {
//            OSG_NOTICE<<"ratio = "<<ratio<<std::endl;
//            double original_width = max_side-min_side;

            double minRatio = 0.02;
            if (ratio<minRatio) ratio = minRatio;

            double base_up = min_up - (max_up - min_up) * ratio / (1.0 - ratio);
            double max_side_over_up = 0.0;
            for(i=0; i< projectedVertices.size(); ++i)
            {
                double delta_side = fabs(projectedVertices[i].x() - mid_side);
                double delta_up = projectedVertices[i].y() - base_up;
                double side_over_up = delta_side / delta_up;
                if (side_over_up > max_side_over_up) max_side_over_up = side_over_up;
            }
            osg::Vec3d v000 = osg::Vec3d(-1.0, -1.0, -1.0) * inverseMVP;
            osg::Vec3d v010 = osg::Vec3d(-1.0, 1.0, -1.0) * inverseMVP;
            osg::Vec3d v100 = osg::Vec3d(1.0, -1.0, -1.0) * inverseMVP;
            osg::Vec3d v110 = osg::Vec3d(1.0, 1.0, -1.0) * inverseMVP;

            osg::Vec3d v001 = osg::Vec3d(-1.0, -1.0, 1.0) * inverseMVP;
            osg::Vec3d v011 = osg::Vec3d(-1.0, 1.0, 1.0) * inverseMVP;
            osg::Vec3d v101 = osg::Vec3d(1.0, -1.0, 1.0) * inverseMVP;
            osg::Vec3d v111 = osg::Vec3d(1.0, 1.0, 1.0) * inverseMVP;


            osg::Vec3d edgeBottomLeft = v001-v000;
            osg::Vec3d edgeBottomRight = v101-v100;
            osg::Vec3d edgeTopRight = v111-v110;
            osg::Vec3d edgeTopLeft = v011-v010;
            edgeBottomLeft.normalize();
            edgeBottomRight.normalize();
            edgeTopLeft.normalize();
            edgeTopRight.normalize();


            double frustumDiagonal = osg::RadiansToDegrees(acos(edgeBottomLeft * edgeBottomRight));


            //OSG_NOTICE<<"  width ratio  = "<<new_width/original_width<<std::endl;
            //OSG_NOTICE<<"  near ratio  = "<<ratio * new_width/original_width<<std::endl;
            double angle = 2.0*osg::RadiansToDegrees(atan(max_side_over_up));


            if (angle > frustumDiagonal)
            {
                double maxHalfAngle = osg::DegreesToRadians(30.0);

                // move ratio back
                max_side_over_up = tan(maxHalfAngle);
                double lowest_up = min_up;

                for(i=0; i< projectedVertices.size(); ++i)
                {
                    double delta_side = fabs(projectedVertices[i].x() - mid_side);
                    double delta_up = delta_side / max_side_over_up;
                    double local_base_up = projectedVertices[i].y() - delta_up;
                    if (local_base_up < lowest_up)
                    {
                        lowest_up = local_base_up;
//                        double side_over_up = delta_side / delta_up;
                    }
                }

                double new_ratio = (min_up-lowest_up)/(max_up-lowest_up);

                //OSG_NOTICE<<"  originalRatio  = "<<ratio<<" new_ratio="<<new_ratio<<std::endl;

                if (new_ratio > ratio) ratio = new_ratio;

                base_up = lowest_up;

            }

            double max_half_width = max_side_over_up*(max_up - base_up);
            min_side = mid_side - max_half_width;
            max_side = mid_side + max_half_width;

//            double new_width = max_side-min_side;


            double y0 = (1.0 + ratio) / (1.0 - ratio);
            overlayData._y0->set(static_cast<float>(y0));


            // OSG_NOTICE<<"y0 = "<<y0<<std::endl;

            overlayData._mainSubgraphStateSet->setAttribute(overlayData._mainSubgraphProgram.get());

            osg::Matrixd projection;
            projection.makeOrtho(min_side,max_side,min_up,max_up,zNear ,zFar);

            camera->setProjectionMatrix(projection);

        }
        else
        {
            overlayData._mainSubgraphStateSet->removeAttribute(osg::StateAttribute::PROGRAM);
            camera->setProjectionMatrixAsOrtho(min_side, max_side, min_up, max_up, zNear ,zFar);
        }


#if 0
        double width = max_side-min_side;
        double height = max_up-min_up;
        double area = width*height;

        OSG_NOTICE<<"width = "<<width<<"\t height = "<<height<<"\t area = "<<area<<std::endl;

        OSG_NOTICE<<"a  min_side    = "<<min_side<<std::endl;
        OSG_NOTICE<<"a  max_side  = "<<max_side<<std::endl;
        OSG_NOTICE<<"a  min_up    = "<<min_up<<std::endl;
        OSG_NOTICE<<"a  max_up  = "<<max_up<<std::endl;
#endif

        if (em)
        {
            camera->setViewMatrixAsLookAt(bs.center(), osg::Vec3d(0.0f,0.0f,0.0f), upVector);
        }
        else
        {
            camera->setViewMatrixAsLookAt(bs.center(), bs.center()+overlayLookVector, upVector);
        }


        // compute the matrix which takes a vertex from local coords into tex coords
        // will use this later to specify osg::TexGen..
        MVP = camera->getViewMatrix() * camera->getProjectionMatrix();

        osg::Matrix MVPT = MVP *
                           osg::Matrix::translate(1.0,1.0,1.0) *
                           osg::Matrix::scale(0.5,0.5,0.5);

        //overlayData._texgenNode->setReferenceFrame(osg::TexGenNode::ABSOLUTE_RF);
        overlayData._texgenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
        overlayData._texgenNode->getTexGen()->setPlanesFromMatrix(MVPT);

        overlayData._textureFrustum.setToUnitFrustum(false,false);
        overlayData._textureFrustum.transformProvidingInverse(MVP);

        // OSG_NOTICE<<std::endl;

        unsigned int contextID = cv->getState()!=0 ? cv->getState()->getContextID() : 0;

        if (usePerspectiveShaders)
        {
            cv->pushStateSet(overlayData._overlayStateSet.get());

            typedef std::list<const osg::StateSet*> StateSetStack;
            StateSetStack statesetStack;

            osgUtil::StateGraph* sg = cv->getCurrentStateGraph();
            while(sg)
            {
                const osg::StateSet* stateset = sg->getStateSet();
                if (stateset)
                {
                    statesetStack.push_front(stateset);
                }
                sg = sg->_parent;
            }

            osg::StateAttribute::GLModeValue base_mode = osg::StateAttribute::ON;
            for(StateSetStack::iterator itr = statesetStack.begin();
                itr != statesetStack.end();
                ++itr)
            {
                osg::StateAttribute::GLModeValue mode = (*itr)->getMode(GL_LIGHTING);
                if ((mode & ~osg::StateAttribute::INHERIT)!=0)
                {
                    if ((mode & osg::StateAttribute::PROTECTED)!=0 ||
                        (base_mode & osg::StateAttribute::OVERRIDE)==0)
                    {
                        base_mode = mode;
                    }
                }
            }

            overlayData._lightingEnabled->set((base_mode & osg::StateAttribute::ON)!=0);
        }

        // if we need to redraw then do cull traversal on camera.
        camera->setClearColor(_overlayClearColor);
        //camera->setStateSet(overlayData._overlayStateSet.get());
        camera->accept(*cv);

        if (usePerspectiveShaders) cv->popStateSet();

        _textureObjectValidList[contextID] = 1;

        overlayData._texgenNode->accept(*cv);

        if (overlayData._geode.valid())
        {
               overlayData._geode->accept(*cv);
        }

    }
    else
    {
        Group::traverse(nv);
    }

    // OSG_NOTICE<<"   "<<&overlayData<<std::endl;
}

void OverlayNode::traverse_VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY(osg::NodeVisitor& nv)
{
    traverse_VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY(nv);
}


void OverlayNode::setOverlaySubgraph(osg::Node* node)
{
    if (_overlaySubgraph == node) return;

    _overlaySubgraph = node;

    for(OverlayDataMap::iterator itr = _overlayDataMap.begin();
        itr != _overlayDataMap.end();
        ++itr)
    {
        osg::Camera* camera = itr->second->_camera.get();
        if (camera)
        {
            camera->removeChildren(0, camera->getNumChildren());
            camera->addChild(node);
        }
    }


    dirtyOverlayTexture();
}

void OverlayNode::dirtyOverlayTexture()
{
    _textureObjectValidList.setAllElementsTo(0);
    _updateCamera = true;
}


void OverlayNode::setTexEnvMode(GLenum mode)
{
    _texEnvMode = mode;
    updateMainSubgraphStateSet();
}


void OverlayNode::setOverlayTextureUnit(unsigned int unit)
{
    _textureUnit = unit;

    updateMainSubgraphStateSet();
}

void OverlayNode::setOverlayTextureSizeHint(unsigned int size)
{
    if (_textureSizeHint == size) return;

    _textureSizeHint = size;


    for(OverlayDataMap::iterator itr = _overlayDataMap.begin();
        itr != _overlayDataMap.end();
        ++itr)
    {
        if (itr->second->_texture.valid()) itr->second->_texture->setTextureSize(_textureSizeHint, _textureSizeHint);
        if (itr->second->_camera.valid()) itr->second->_camera->setViewport(0,0,_textureSizeHint,_textureSizeHint);
    }

    //_texture->dirtyTextureObject();
}

void OverlayNode::updateMainSubgraphStateSet()
{
   OSG_INFO<<"OverlayNode::updateMainSubgraphStateSet()"<<std::endl;

   for(OverlayDataMap::iterator itr = _overlayDataMap.begin();
        itr != _overlayDataMap.end();
        ++itr)
    {
        osg::TexGenNode* texgenNode = itr->second->_texgenNode.get();
        if (texgenNode) texgenNode->setTextureUnit(_textureUnit);

        osg::StateSet* mainSubgraphStateSet = itr->second->_mainSubgraphStateSet.get();
        if (mainSubgraphStateSet)
        {
            mainSubgraphStateSet->clear();
            mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, itr->second->_texture.get(), osg::StateAttribute::ON);
            mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
            mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
            mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
            mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

            if (_texEnvMode!=GL_NONE)
            {
                mainSubgraphStateSet->setTextureAttribute(_textureUnit, new osg::TexEnv((osg::TexEnv::Mode)_texEnvMode));
            }
        }
    }
}
