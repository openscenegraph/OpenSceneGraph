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
#include <osg/io_utils>

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
            face.plane.set(1.0,0.0,0.0,1.0);
            face.vertices.push_back(v000);
            face.vertices.push_back(v001);
            face.vertices.push_back(v011);
            face.vertices.push_back(v010);
        }

        {   // right plane.
            Face& face = createFace();
            face.plane.set(-1.0,0.0,0.0,1.0);
            face.vertices.push_back(v100);
            face.vertices.push_back(v110);
            face.vertices.push_back(v111);
            face.vertices.push_back(v101);
        }

        {   // bottom plane.
            Face& face = createFace();
            face.plane.set(0.0,1.0,0.0,1.0);
            face.vertices.push_back(v000);
            face.vertices.push_back(v010);
            face.vertices.push_back(v110);
            face.vertices.push_back(v100);
        }

        {   // top plane.
            Face& face = createFace();
            face.plane.set(0.0,-1.0,0.0,1.0);
            face.vertices.push_back(v001);
            face.vertices.push_back(v101);
            face.vertices.push_back(v111);
            face.vertices.push_back(v011);
        }
        
        if (withNear)
        {   // near plane
            Face& face = createFace();
            face.plane.set(0.0,0.0,-1.0,1.0);
            face.vertices.push_back(v000);
            face.vertices.push_back(v100);
            face.vertices.push_back(v101);
            face.vertices.push_back(v001);
        }        
        
        if (withFar)
        {   // far plane
            Face& face = createFace();
            face.plane.set(0.0,0.0,1.0,1.0);
            face.vertices.push_back(v010);
            face.vertices.push_back(v011);
            face.vertices.push_back(v111);
            face.vertices.push_back(v110);
        }        
    }

    void setToBoundingBox(const osg::BoundingBox& bb)
    {
        const osg::Vec3d v000(bb.xMin(),bb.yMin(), bb.zMin());
        const osg::Vec3d v010(bb.xMin(),bb.yMax(), bb.zMin());
        const osg::Vec3d v001(bb.xMin(),bb.yMin(), bb.zMax());
        const osg::Vec3d v011(bb.xMin(),bb.yMax(), bb.zMax());
        const osg::Vec3d v100(bb.xMax(),bb.yMin(), bb.zMin());
        const osg::Vec3d v110(bb.xMax(),bb.yMax(), bb.zMin());
        const osg::Vec3d v101(bb.xMax(),bb.yMin(), bb.zMax());
        const osg::Vec3d v111(bb.xMax(),bb.yMax(), bb.zMax());
        
        _faces.clear();

        {   // left plane.
            Face& face = createFace();
            face.plane.set(1.0,0.0,0.0,-bb.xMin());
            face.vertices.push_back(v000);
            face.vertices.push_back(v001);
            face.vertices.push_back(v011);
            face.vertices.push_back(v010);
        }

        {   // right plane.
            Face& face = createFace();
            face.plane.set(-1.0,0.0,0.0,bb.xMax());
            face.vertices.push_back(v100);
            face.vertices.push_back(v110);
            face.vertices.push_back(v111);
            face.vertices.push_back(v101);
        }

        {   // bottom plane.
            Face& face = createFace();
            face.plane.set(0.0,1.0,0.0,-bb.yMin());
            face.vertices.push_back(v000);
            face.vertices.push_back(v010);
            face.vertices.push_back(v110);
            face.vertices.push_back(v100);
        }

        {   // top plane.
            Face& face = createFace();
            face.plane.set(0.0,-1.0,0.0,bb.yMax());
            face.vertices.push_back(v001);
            face.vertices.push_back(v101);
            face.vertices.push_back(v111);
            face.vertices.push_back(v011);
        }
        {   // near plane
            Face& face = createFace();
            face.plane.set(0.0,0.0,1.0,-bb.zMin());
            face.vertices.push_back(v000);
            face.vertices.push_back(v100);
            face.vertices.push_back(v101);
            face.vertices.push_back(v001);
        }        
        
        {   // far plane
            Face& face = createFace();
            face.plane.set(0.0,0.0,-1.0,bb.zMax());
            face.vertices.push_back(v010);
            face.vertices.push_back(v011);
            face.vertices.push_back(v111);
            face.vertices.push_back(v110);
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


    void insertVertex(const osg::Vec3d& vertex)
    {
        // osg::notify(osg::NOTICE)<<"Inserting vertex "<<vertex<<std::endl;
    
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
        
        for(Edges::iterator eitr = edges.begin();
            eitr != edges.end();
            ++eitr)
        {
            if (eitr->second==1) 
            {
                // osg::notify(osg::NOTICE)<<"     edge Ok"<<std::endl;
                const Edge& edge = eitr->first;
                Face face;
                face.plane.set(vertex, edge.first, edge.second);
                face.vertices.push_back(vertex);
                face.vertices.push_back(edge.first);
                face.vertices.push_back(edge.second);
                if (face.plane.distance(center)<0.0) face.plane.flip();
                
                _faces.push_back(face);
            }
        }
    

        // osg::notify(osg::NOTICE)<<"  Removed faces "<<removedFaces.size()<<std::endl;
    }



    void cut(const osg::Polytope& polytope)
    {
        // osg::notify(osg::NOTICE)<<"Cutting with polytope "<<std::endl;
        for(osg::Polytope::PlaneList::const_iterator itr = polytope.getPlaneList().begin();
            itr != polytope.getPlaneList().end();
            ++itr)
        {
            cut(*itr);
        }
    }

    void cut(const CustomPolytope& polytope)
    {
        // osg::notify(osg::NOTICE)<<"Cutting with polytope "<<std::endl;
        for(Faces::const_iterator itr = polytope._faces.begin();
            itr != polytope._faces.end();
            ++itr)
        {
            cut(itr->plane);
        }
    }

    void cut(const osg::Plane& plane)
    {
        // osg::notify(osg::NOTICE)<<"  Cutting plane "<<plane<<std::endl;
        Face newFace;
        
        for(Faces::iterator itr = _faces.begin();
            itr != _faces.end();
            )
        {
            Face& face = *itr;
            int intersect = plane.intersect(face.vertices);
            if (intersect==1)
            {
                // osg::notify(osg::NOTICE)<<"    Face inside"<<std::endl; 
                ++itr;
            }
            else if (intersect==0)
            {
                // osg::notify(osg::NOTICE)<<"    Face intersecting - before "<<face.vertices.size()<<std::endl;
                
                Vertices& vertices = face.vertices;
                Vertices newVertices;

                typedef std::vector<double> Distances;
                Distances distances;
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

                    // does edge intersect plane
                    if (distance_a * distance_b<0.0)
                    {
                        // inserting vertex
                        osg::Vec3d intersection = (vb*distance_a - va*distance_b)/(distance_a-distance_b);
                        newVertices.push_back(intersection);
                        newFace.vertices.push_back(intersection);
                    }
                }
                
                vertices.swap(newVertices);
                
                // osg::notify(osg::NOTICE)<<"        intersecting - after "<<face.vertices.size()<<std::endl;

                ++itr;
            }
            else if (intersect==-1)
            {
                // osg::notify(osg::NOTICE)<<"    Face outside"<<_faces.size()<<std::endl; 
                itr = _faces.erase(itr);
            }
        }
        
        if (!newFace.vertices.empty())
        {
            // osg::notify(osg::NOTICE)<<"    inserting newFace intersecting "<<newFace.vertices.size()<<std::endl;
            newFace.plane = plane;

            Vertices& vertices = newFace.vertices;

            osg::Vec3d side = fabs(plane.getNormal().x() < fabs(plane.getNormal().y())) ? 
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
                anglePositions[angle] = *vitr;
            }

            
            Vertices newVertices;
            for(AnglePositions::iterator aitr = anglePositions.begin();
                aitr != anglePositions.end();
                ++aitr)
            {
                newVertices.push_back(aitr->second);
            }
            
            newVertices.swap(vertices);
            
            // osg::notify(osg::NOTICE)<<"     after angle sort  "<<newFace.vertices.size()<<std::endl;

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
    _updateCamera(false)
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
    _continuousUpdate(copy._continuousUpdate)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
    init();
}

void OverlayNode::OverlayData::setThreadSafeRefUnref(bool threadSafe)
{
    if (_camera.valid()) _camera->setThreadSafeRefUnref(threadSafe);
    if (_texgenNode.valid()) _texgenNode->setThreadSafeRefUnref(threadSafe);
    if (_mainSubgraphStateSet.valid()) _mainSubgraphStateSet->setThreadSafeRefUnref(threadSafe);
    if (_texture.valid()) _texture->setThreadSafeRefUnref(threadSafe);
}

void OverlayNode::OverlayData::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_camera.valid()) _camera->resizeGLObjectBuffers(maxSize);
    if (_texgenNode.valid()) _texgenNode->resizeGLObjectBuffers(maxSize);
    if (_mainSubgraphStateSet.valid()) _mainSubgraphStateSet->resizeGLObjectBuffers(maxSize);
    if (_texture.valid()) _texture->resizeGLObjectBuffers(maxSize);
}

void OverlayNode::OverlayData::releaseGLObjects(osg::State* state) const
{
    if (_camera.valid()) _camera->releaseGLObjects(state);
    if (_texgenNode.valid()) _texgenNode->releaseGLObjects(state);
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
        itr->second.setThreadSafeRefUnref(threadSafe);
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
        itr->second.resizeGLObjectBuffers(maxSize);
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
        itr->second.releaseGLObjects(state);
    }
}

void OverlayNode::setOverlayTechnique(OverlayTechnique technique)
{
    if (_overlayTechnique==technique) return;
    
    _overlayTechnique = technique;
    
    init();
}

OverlayNode::OverlayData& OverlayNode::getOverlayData(osgUtil::CullVisitor* cv)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_overlayDataMapMutex);
    OverlayDataMap::iterator itr = _overlayDataMap.find(cv);
    if (itr != _overlayDataMap.end()) return itr->second;
    
    OverlayData& overlayData = _overlayDataMap[cv];
    
    
    unsigned int tex_width = _textureSizeHint;
    unsigned int tex_height = _textureSizeHint;
    
    if (!overlayData._texture) 
    { 
        osg::notify(osg::NOTICE)<<"   setting up texture"<<std::endl;

        osg::Texture2D* texture = new osg::Texture2D;
        texture->setTextureSize(tex_width, tex_height);
        texture->setInternalFormat(GL_RGBA);
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
        texture->setBorderColor(osg::Vec4(_overlayClearColor));
        overlayData._texture = texture;
    }   

    // set up the render to texture camera.
    if (!overlayData._camera)
    {
        osg::notify(osg::NOTICE)<<"   setting up camera"<<std::endl;

        // create the camera
        overlayData._camera = new osg::Camera;
         
        overlayData._camera->setClearColor(_overlayClearColor);

        overlayData._camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);

        // set viewport
        overlayData._camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        overlayData._camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        overlayData._camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        overlayData._camera->attach(osg::Camera::COLOR_BUFFER, overlayData._texture.get());
        
        if (_overlaySubgraph.valid()) overlayData._camera->addChild(_overlaySubgraph.get());
    }

    if (!overlayData._texgenNode)
    {
        overlayData._texgenNode = new osg::TexGenNode;
        overlayData._texgenNode->setTextureUnit(_textureUnit);
    }

    if (!overlayData._mainSubgraphStateSet) 
    {
        overlayData._mainSubgraphStateSet = new osg::StateSet;

        overlayData._mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, overlayData._texture.get(), osg::StateAttribute::ON);
        overlayData._mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
        overlayData._mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
        overlayData._mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
        overlayData._mainSubgraphStateSet->setTextureMode(_textureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

        if (_texEnvMode!=GL_NONE) 
        {
            overlayData._mainSubgraphStateSet->setTextureAttribute(_textureUnit, new osg::TexEnv((osg::TexEnv::Mode)_texEnvMode));
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
    osg::notify(osg::NOTICE)<<"OverlayNode::init() - OBJECT_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<<std::endl;
    OverlayData& overlayData = getOverlayData(0);
}

void OverlayNode::init_VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY()
{
    osg::notify(osg::NOTICE)<<"OverlayNode::init() - VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<<std::endl;
}

void OverlayNode::init_VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY()
{
    osg::notify(osg::NOTICE)<<"OverlayNode::init() - VIEW_DEPENDENT_WITH_PERSPECTIVE_OVERLAY"<<std::endl;
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
    OverlayData& overlayData = getOverlayData(0);
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

  
                if (csn)
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
    // osg::notify(osg::NOTICE)<<"OverlayNode::traverse() - VIEW_DEPENDENT_WITH_ORTHOGRAPHIC_OVERLAY"<<std::endl;


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
    
    OverlayData& overlayData = getOverlayData(cv);
    osg::Camera* camera = overlayData._camera.get();

    if (_overlaySubgraph.valid()) 
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

        osg::BoundingSphere bs = _overlaySubgraph->getBound();



        // push the stateset.
        cv->pushStateSet(overlayData._mainSubgraphStateSet.get());

        Group::traverse(nv);    

        cv->popStateSet();

        osg::Matrix pm = *(cv->getProjectionMatrix());
        
        double znear = cv->getCalculatedNearPlane();
        double zfar = cv->getCalculatedFarPlane();
        
        // osg::notify(osg::NOTICE)<<" before znear ="<<znear<<"\t zfar ="<<zfar<<std::endl;
        
        cv->computeNearPlane();
        
        znear = cv->getCalculatedNearPlane();
        zfar = cv->getCalculatedFarPlane();

        // osg::notify(osg::NOTICE)<<" after znear ="<<znear<<"\t zfar ="<<zfar<<std::endl;

        // osg::notify(osg::NOTICE)<<" before clamp pm="<<pm<<std::endl;

        cv->clampProjectionMatrixImplementation(pm, znear,zfar);
        
        // osg::notify(osg::NOTICE)<<" after clamp pm="<<pm<<std::endl;
        
        osg::Matrix MVP = *(cv->getModelViewMatrix()) * pm;
        osg::Matrix inverseMVP;
        inverseMVP.invert(MVP);

        // create polytope for the view frustum in local coords
        CustomPolytope frustum;
        frustum.setToUnitFrustum();
        frustum.transform(inverseMVP, MVP);
        

        // create polytope for the overlay subgraph in local coords
        CustomPolytope overlayPolytope;

        // get the bounds of the model.    
        osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
        _overlaySubgraph->accept(cbbv);
        overlayPolytope.setToBoundingBox(cbbv.getBoundingBox());
        
        if (csn)
        {
            overlayPolytope.insertVertex(osg::Vec3d(0.0,0.0,0.0));
        }

        frustum.cut(overlayPolytope);
        
        

        CustomPolytope::Vertices corners;
        frustum.getPoints(corners);
        
        osg::notify(osg::NOTICE)<<"AFTER CUT corners = "<<corners.size()<<std::endl;

        if (corners.size()<8) return;
 
        osg::Vec3d center_near = (corners[0]+corners[1]+corners[2]+corners[3])*0.25;
        osg::Vec3d center_far = (corners[4]+corners[5]+corners[6]+corners[7])*0.25;
        osg::Vec3d center = (center_near+center_far)*0.5;
        osg::Vec3d frustum_axis = (center_far-center_near);
        frustum_axis.normalize();
        
        double diagonal = (corners[0]-corners[7]).length();
        float diagonal_near = (corners[0]-center_near).length();
        float diagonal_far = (corners[7]-center_far).length();
        
        osg::notify(osg::NOTICE)<<"    center ="<<center<<" diagonal ="<<diagonal<<std::endl;
        osg::notify(osg::NOTICE)<<"    frustum_axis ="<<frustum_axis<<" diagonal_near ="<<diagonal_near<<" diagonal_far="<<diagonal_far<<std::endl;

        osg::Vec3d lookVector(0.0,0.0,-1.0);
        if (csn)
        {
            lookVector = -center;
            lookVector.normalize();
        }
        
        osg::Vec3d sideVector = lookVector ^ frustum_axis;
        sideVector.normalize();
        
        osg::Vec3d upVector = sideVector ^ lookVector;
        upVector.normalize();
        
        osg::notify(osg::NOTICE)<<"    lookVector ="<<lookVector<<std::endl;
        
        double min_side = DBL_MAX;
        double max_side = -DBL_MAX;
        double min_up = DBL_MAX;
        double max_up = -DBL_MAX;

        for(CustomPolytope::Vertices::iterator itr = corners.begin();
            itr != corners.end();
            ++itr)
        {
            osg::Vec3d delta = *itr - center;
            double distance_side = delta * sideVector;
            double distance_up = delta * upVector;
            
            if (distance_side<min_side) min_side = distance_side;
            if (distance_side>max_side) max_side = distance_side;
            if (distance_up<min_up) min_up = distance_up;
            if (distance_up>max_up) max_up = distance_up;
        }
        
        osg::notify(osg::NOTICE)<<"    upVector ="<<upVector<<"  min="<<min_side<<" max="<<max_side<<std::endl;
        osg::notify(osg::NOTICE)<<"    sideVector ="<<sideVector<<"  min="<<min_up<<" max="<<max_up<<std::endl;



        double bs_center = (bs.center()-center) * lookVector;
        double bs_near = bs_center-bs.radius();
        double bs_far = bs_center+bs.radius();
        
        osg::notify(osg::NOTICE)<<"    bs_near="<<bs_near<<"  bs_far="<<bs_far<<std::endl;
        osg::notify(osg::NOTICE)<<"    bs.radius()="<<bs.radius()<<std::endl;


#if 1
        camera->setProjectionMatrixAsOrtho(min_side,max_side,min_up,max_up,bs_near+bs.center().length(),bs_far+bs.center().length());
        camera->setViewMatrixAsLookAt(osg::Vec3d(0.0f,0.0f,0.0f), bs.center(), upVector);

#else        
        
        camera->setProjectionMatrixAsOrtho(-bs.radius(),bs.radius(),-bs.radius(),bs.radius(),-bs.radius(),bs.radius());
        camera->setViewMatrixAsLookAt(bs.center(), osg::Vec3d(0.0f,0.0f,0.0f), osg::Vec3(0.0,0.0,1.0));
#endif        


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

        osg::notify(osg::NOTICE)<<std::endl;

        unsigned int contextID = cv->getState()!=0 ? cv->getState()->getContextID() : 0;

        // if we need to redraw then do cull traversal on camera.
        camera->setClearColor(_overlayClearColor);
        camera->accept(*cv);
        _textureObjectValidList[contextID] = 1;

        overlayData._texgenNode->accept(*cv);

    }
    else
    {
        Group::traverse(nv);    
    }
    
    // osg::notify(osg::NOTICE)<<"   "<<&overlayData<<std::endl;
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
        osg::Camera* camera = itr->second._camera.get();
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
        if (itr->second._texture.valid()) itr->second._texture->setTextureSize(_textureSizeHint, _textureSizeHint);
        if (itr->second._camera.valid()) itr->second._camera->setViewport(0,0,_textureSizeHint,_textureSizeHint);
    }

    //_texture->dirtyTextureObject();
}

void OverlayNode::updateMainSubgraphStateSet()
{
   osg::notify(osg::NOTICE)<<"OverlayNode::updateMainSubgraphStateSet()"<<std::endl;

   for(OverlayDataMap::iterator itr = _overlayDataMap.begin();
        itr != _overlayDataMap.end();
        ++itr)
    {
        osg::TexGenNode* texgenNode = itr->second._texgenNode.get();
        if (texgenNode) texgenNode->setTextureUnit(_textureUnit);

        osg::StateSet* mainSubgraphStateSet = itr->second._mainSubgraphStateSet.get();
        if (mainSubgraphStateSet)  
        {
            mainSubgraphStateSet->clear();
            mainSubgraphStateSet->setTextureAttributeAndModes(_textureUnit, itr->second._texture.get(), osg::StateAttribute::ON);
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
