/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the SCEA Shared Source License, Version 1.0 (the "License"); you may not use this 
 * file except in compliance with the License. You may obtain a copy of the License at:
 * http://research.scea.com/scea_shared_source_license.html
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License 
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 
 * implied. See the License for the specific language governing permissions and limitations under the 
 * License. 
 */

#ifndef _DAE_WRITER_H_
#define _DAE_WRITER_H_

#include <map>

#include <osg/Node>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LightSource>
#include <osg/CameraNode>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Switch>
#include <osg/StateSet>
#include <osg/LOD>
#include <osg/ProxyNode>
#include <osg/CoordinateSystemNode>
#include <osg/BlendColor>
#include <osg/BlendFunc>

#include <osg/Notify>
#include <osg/NodeVisitor>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include "daeUtils.h"

#include <dae.h>
#include <dae/daeDocument.h>


class domCOLLADA;
class domGeometry;
class domInstance_geometry;
class domLibrary_cameras;
class domLibrary_effects;
class domLibrary_geometries;
class domLibrary_lights;
class domLibrary_materials;
class domLibrary_visual_scenes;
class domMaterial;
class domMesh;
class domNode;
class domSource;
class domVisual_scene;
class domP;

namespace osgdae {
  
/**
@class daeWriter
@brief Write a OSG scene into a DAE file 
*/ 
class daeWriter : public osg::NodeVisitor
{
protected:
    class ArrayNIndices;
public:
    enum NodeType { NODE, GEODE, GROUP, LIGHT, CAMERA, MATRIX, POSATT, SWITCH, LOD };

        daeWriter( const std::string &fname, bool usePolygons=false );
    virtual ~daeWriter();

    void setRootNode( const osg::Node &node );

    bool isSuccess() { return success; }

    bool writeFile();

    virtual void    apply( osg::Node &node );
    virtual void    apply( osg::Geode &node );
    virtual void    apply( osg::Group &node );
    virtual void    apply( osg::LightSource &node );
    virtual void    apply( osg::CameraNode &node );
    virtual void    apply( osg::MatrixTransform &node );
    virtual void    apply( osg::PositionAttitudeTransform &node );
    virtual void    apply( osg::Switch &node );
    virtual void    apply( osg::LOD &node );

    //virtual void  apply( osg::Billboard &node);
    virtual void    apply( osg::ProxyNode &node );
    //virtual void  apply( osg::Projection &node)
    virtual void    apply( osg::CoordinateSystemNode &node );
    //virtual void  apply( osg::ClipNode &node)
    //virtual void  apply( osg::TexGenNode &node)
    virtual void    apply( osg::Transform &node );
    //virtual void  apply( osg::CameraView &node)
    //virtual void  apply( osg::Sequence &node)
    //virtual void  apply( osg::PagedLOD &node)
    //virtual void  apply( osg::ClearNode &node)
    //virtual void  apply( osg::OccluderNode &node)

/*protected:
    struct MeshData {
        domMesh *mesh;
        domSource *pos;
        domSource *norm;
        domSource *color;
        std::vector< domSource * > texcoord;
        std::string name;
    };*/


protected: //methods
    void debugPrint( osg::Node &node );
    
    
    bool processGeometry( osg::Geometry *geom, domGeometry *geo, const std::string &name );
    domSource* createSource( daeElement *parent, const std::string &baseName, int size, bool color = false, bool uv = false );
    template < typename Ty >
        Ty *createPrimGroup( daeString type, domMesh *mesh, domSource *norm, domSource *color, const std::vector< domSource* > &texcoord );

    void processMaterial( osg::StateSet *ss, domInstance_geometry *ig, const std::string &geoName );

    void createAssetTag();

protected: //members
    DAE *dae;
    daeDocument *doc;
    domCOLLADA *dom;
    domLibrary_cameras *lib_cameras;
    domLibrary_effects *lib_effects;
    domLibrary_geometries *lib_geoms;
    domLibrary_lights *lib_lights;
    domLibrary_materials *lib_mats;
    domLibrary_visual_scenes *lib_vis_scenes;
    domNode *currentNode;
    domVisual_scene *vs;

    bool success;
    NodeType lastVisited;
    unsigned int lastDepth;

    std::map< std::string, int > uniqueNames;

    std::map< osg::Geometry*, domGeometry * > geometryMap;
    std::map< osg::StateSet*, domMaterial * > materialMap;

    daeURI rootName;

    bool usePolygons;

protected: //inner classes
    class ArrayNIndices 
    {
    public:
        enum Mode { NONE = 0, VEC2 = 2, VEC3 = 3, VEC4 = 4 };
        osg::Vec2Array *vec2;
        osg::Vec3Array *vec3;
        osg::Vec4Array *vec4;
        osg::IndexArray *inds;
        Mode mode;

        ArrayNIndices( osg::Array *array, osg::IndexArray *ind ) : vec2(0), vec3(0), vec4(0), inds( ind ), mode(NONE)
        {
            if ( array != NULL )
            {
                switch( array->getType() )
                {
                case osg::Array::Vec2ArrayType:
                    mode = VEC2;
                    vec2 = (osg::Vec2Array*)array;
                    break;
                case osg::Array::Vec3ArrayType:
                    mode = VEC3;
                    vec3 = (osg::Vec3Array*)array;
                    break;
                case osg::Array::Vec4ArrayType:
                    mode = VEC4;
                    vec4 = (osg::Vec4Array*)array;
                    break;
                default:
                    osg::notify( osg::WARN ) << "Array is unsupported vector type" << std::endl;
                    break;
                }
            }
        }
    };

private: //members
        
        /** append elements (verts, normals, colors and texcoord) for file write */
        void appendGeometryIndices(osg::Geometry *geom,
                                          domP * p,
                                          unsigned int vindex,
                                          domSource * norm,
                                          domSource * color,
                                          const ArrayNIndices & verts,
                                          const ArrayNIndices & normals,
                                          const ArrayNIndices & colors,
                                          const std::vector<ArrayNIndices> & texcoords,
                                          unsigned int  ncount,
                                          unsigned int  ccount);

        /** provide a name to node */
        std::string getNodeName(const osg::Node & node,const std::string & defaultName);
        
        /** provide an unique name */
        std::string uniquify( const std::string &name );
};

};

#endif

