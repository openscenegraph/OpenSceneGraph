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
#include <stack>

#include <osg/Node>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LightSource>
#include <osg/Camera>
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
#include <osgDB/ExternalFileWriter>
#include <osgSim/MultiSwitch>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/UpdateBone>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/MorphGeometry>

#include <dae.h>
#include <dae/daeDocument.h>
#include <dom/domChannel.h>


class domCOLLADA;
class domGeometry;
class domInstance_geometry;
class domLibrary_cameras;
class domLibrary_effects;
class domLibrary_geometries;
class domLibrary_lights;
class domLibrary_materials;
class domLibrary_visual_scenes;
class domLibrary_animations;
class domMaterial;
class domMesh;
class domNode;
class domSource;
class domVisual_scene;
class domP;

namespace osgDAE {

/// Convert value to string using it's stream operator
template <typename T>
std::string toString(T value)
{
    std::stringstream str;
    str << value;
    return str.str();
}

std::string toString(const osg::Vec3f& value);
std::string toString(const osg::Vec3d& value);
std::string toString(const osg::Matrix& value);

// Collects all nodes that are targeted by an animation
class FindAnimatedNodeVisitor : public osg::NodeVisitor
{
public:
    FindAnimatedNodeVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {}

    virtual void apply(osg::Node& node)
    {
        osg::NodeCallback* ncb = node.getUpdateCallback();
        if (ncb)
        {
            osgAnimation::AnimationUpdateCallback<osg::NodeCallback>* ut = dynamic_cast<osgAnimation::AnimationUpdateCallback<osg::NodeCallback>*>(ncb);
            if (ut)
            {
                if (_updateCallbackNameNodeMap[ut->getName()] == NULL)
                {
                    _updateCallbackNameNodeMap[ut->getName()] = &node;
                }
                else
                {
                    // TODO store in a multimap and let the exporter create multiple <channel>s for each connected node
                    OSG_WARN << "Multiple nodes using the same update callback not supported" << std::endl;
                }
            }
        }
        traverse(node);
    }

    osg::Node* getTargetNode(const std::string& targetName)
    {
        UpdateCallbackNameNodeMap::iterator it = _updateCallbackNameNodeMap.find(targetName);
        if (it != _updateCallbackNameNodeMap.end())
        {
            return it->second;
        }
        return NULL;
    }

private:
    typedef std::map< std::string, osg::Node*>    UpdateCallbackNameNodeMap;
    UpdateCallbackNameNodeMap _updateCallbackNameNodeMap;
};

/**
@class daeWriter
@brief Write a OSG scene into a DAE file
*/
class daeWriter : public osg::NodeVisitor
{
protected:
    class ArrayNIndices;
public:
    struct Options
    {
        Options();

        bool usePolygons;
        /** work in Google compatibility mode. In daeWMaterials, change transparency color. And in daeWGeometry, replace tristrip and trifans by triangles*/
        bool googleMode;
        /** Write OSG specific data as extra data. */
        bool writeExtras;
        /** work in Google compatibility mode for textures*/
        bool earthTex;
        /** link to original images instead of exporting */
        bool linkOrignialTextures;
        /** force the use an image for a texture, even if the file is not found (when m_linkOrignialTextures). */
        bool forceTexture;
        bool namesUseCodepage;
        unsigned int relativiseImagesPathNbUpDirs;
    };
    daeWriter(DAE *dae_, const std::string &fileURI, const std::string & directory, const std::string & srcDirectory, const osgDB::ReaderWriter::Options * options, TraversalMode tm=TRAVERSE_ALL_CHILDREN, const Options * pluginOptions=NULL);
    virtual ~daeWriter();

    void setRootNode( const osg::Node &node );

    bool isSuccess() { return success; }

    bool writeFile();

    virtual void    apply( osg::Node &node );
    virtual void    apply( osg::Geode &node );
    virtual void    apply( osg::Group &node );
    virtual void    apply( osg::LightSource &node );
    virtual void    apply( osg::Camera &node );
    virtual void    apply( osg::MatrixTransform &node );
    virtual void    apply( osg::PositionAttitudeTransform &node );
    virtual void    apply( osg::Switch &node );
    virtual void    apply( osg::Sequence &node );
    virtual void    apply( osg::LOD &node );

    //virtual void    apply( osg::Billboard &node);
    virtual void    apply( osg::ProxyNode &node );
    //virtual void  apply( osg::Projection &node)
    virtual void    apply( osg::CoordinateSystemNode &node );
    //virtual void  apply( osg::ClipNode &node)
    //virtual void  apply( osg::TexGenNode &node)
    virtual void    apply( osg::Transform &node );
    virtual void    apply( osg::CameraView &node);
    //virtual void  apply( osg::PagedLOD &node)
    //virtual void  apply( osg::ClearNode &node)
    //virtual void  apply( osg::OccluderNode &node)

    void traverse (osg::Node &node);




protected: //methods

    void writeAnimations(osg::Node& node);
    void writeNodeExtra(osg::Node &node);
    void writeUpdateTransformElements(const osg::Vec3 &pos, const osg::Quat &q,    const osg::Vec3 &s);
    void writeRigGeometry(osgAnimation::RigGeometry *pOsgRigGeometry);
    void writeMorphGeometry(osgAnimation::MorphGeometry *pOsgMorphGeometry);

    void debugPrint( osg::Node &node );

    domGeometry* getOrCreateDomGeometry(osg::Geometry* pOsgGeometry);
    bool processGeometry( osg::Geometry *geom, domGeometry *geo, const std::string &name );
    domSource* createSource( daeElement *parent, const std::string &baseName, int size, bool color = false, bool uv = false );
    template < typename Ty >
        Ty *createPrimGroup( daeString type, domMesh *mesh, domSource *norm, domSource *color, const std::vector< domSource* > &texcoord );

    void processMaterial( osg::StateSet *ss, domBind_material *pDomBindMaterial, const std::string &geoName );

    void createAssetTag(bool isZUpAxis);

    // Overloaded version of createAssetTag which provides ability to
    // set user defined values for child elements
    void createAssetTag(const osg::Node &node);

    void pushStateSet(osg::StateSet* ss);

    void popStateSet(osg::StateSet* ss);

protected: //members
    DAE *dae;
    daeDocument *doc;
    domCOLLADA *dom;
    domLibrary_cameras *lib_cameras;
    domLibrary_effects *lib_effects;
    domLibrary_controllers *lib_controllers;
    domLibrary_geometries *lib_geoms;
    domLibrary_lights *lib_lights;
    domLibrary_materials *lib_mats;
    domLibrary_visual_scenes *lib_vis_scenes;
    domLibrary_animations* _domLibraryAnimations;
    domNode *currentNode;
    domVisual_scene *vs;

    bool success;
    unsigned int lastDepth;

  struct CompareStateSet
  {
      bool operator()(const osg::ref_ptr<osg::StateSet>& ss1, const osg::ref_ptr<osg::StateSet>& ss2) const
    {
      //std::cout << "CompareStateSet: " << ss1->compare(*ss2, false) << " " << ss1 << " " << ss2 << std::endl;
      return ss1->compare(*ss2, true) < 0;
    }
  };


    typedef std::map< osg::ref_ptr<osg::StateSet>, domMaterial *, CompareStateSet> MaterialMap;
    typedef std::stack<osg::ref_ptr<osg::StateSet> > StateSetStack;
    typedef std::map< osg::Geometry*, domGeometry *> OsgGeometryDomGeometryMap;
    typedef std::map< osgAnimation::RigGeometry*, domController *> OsgRigGeometryDomControllerMap;
    typedef std::map< osgAnimation::MorphGeometry*, domController *> OsgMorphGeometryDomControllerMap;

    std::map< std::string, int > uniqueNames;
    OsgGeometryDomGeometryMap geometryMap;
    OsgRigGeometryDomControllerMap _osgRigGeometryDomControllerMap;
    OsgMorphGeometryDomControllerMap _osgMorphGeometryDomControllerMap;

    MaterialMap materialMap;
    StateSetStack stateSetStack;

    osg::ref_ptr<osg::StateSet> currentStateSet;

    daeURI rootName;

    osg::StateSet* CleanStateSet(osg::StateSet* pStateSet) const;

    void updateCurrentDaeNode();

protected: //inner classes
    class ArrayNIndices
    {
    public:
        enum Mode { NONE, VEC2F, VEC2D, VEC3F, VEC3D, VEC4F, VEC4D, VEC4_UB };

        osg::Vec2Array*         vec2;
        osg::Vec3Array*         vec3;
        osg::Vec4Array*         vec4;
        osg::Vec2dArray*        vec2d;
        osg::Vec3dArray*        vec3d;
        osg::Vec4dArray*        vec4d;
        osg::Vec4ubArray*       vec4ub;

        osg::Array*             valArray;
        osg::IndexArray*        inds;

        ArrayNIndices( osg::Array* valArray, osg::IndexArray* ind );

        Mode getMode() const { return mode; }

        unsigned int getDAESize();

        /// Appends the contained OSG vector array to a domListOfFloats
        bool append(domListOfFloats & list);
    protected:
        Mode mode;
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

        /** Current RenderingHint */
        /** This are needed because the stateSet merge code currently does not handle it */
        int m_CurrentRenderingHint;

        FindAnimatedNodeVisitor _animatedNodeCollector;

        const osgDB::ReaderWriter::Options * _options;
        Options _pluginOptions;
        osgDB::ExternalFileWriter _externalWriter;
};

}

#endif

