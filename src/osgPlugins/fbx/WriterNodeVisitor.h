// -*-c++-*-

/*
 * FBX writer for Open Scene Graph
 *
 * Copyright (C) 2009
 *
 * Writing support added 2009 by Thibault Caporal and Sukender (Benoit Neil - http://sukender.free.fr)
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

#ifndef _FBX_WRITER_NODE_VISITOR_HEADER__
#define _FBX_WRITER_NODE_VISITOR_HEADER__

#include <map>
#include <set>
#include <stack>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/NodeVisitor>
#include <osg/PrimitiveSet>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>
#include <osgDB/ExternalFileWriter>

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#pragma warning( default : 4996 )
#endif
#include <fbxsdk.h>

struct Triangle
{
    unsigned int t1;
    unsigned int t2;
    unsigned int t3;
    unsigned int normalIndex1;        ///< Normal index for all bindings except BIND_PER_VERTEX and BIND_OFF.
    unsigned int normalIndex2;
    unsigned int normalIndex3;
    int material;
};

struct VertexIndex
{
    VertexIndex(unsigned int in_vertexIndex, unsigned int in_drawableIndex, unsigned int in_normalIndex)
        : vertexIndex(in_vertexIndex), drawableIndex(in_drawableIndex), normalIndex(in_normalIndex)
    {}
    VertexIndex(const VertexIndex & v) : vertexIndex(v.vertexIndex), drawableIndex(v.drawableIndex), normalIndex(v.normalIndex) {}

    unsigned int vertexIndex;        ///< Index of the vertice position in the vec3 array
    unsigned int drawableIndex;
    unsigned int normalIndex;        ///< Normal index for all bindings except BIND_PER_VERTEX and BIND_OFF.

    bool operator<(const VertexIndex & v) const {
        if (drawableIndex!=v.drawableIndex) return drawableIndex<v.drawableIndex;
        return vertexIndex<v.vertexIndex;
    }
};

typedef std::vector<std::pair<Triangle, int> > ListTriangle; //the int is the drawable of the triangle
typedef std::map<VertexIndex, unsigned int> MapIndices;        ///< Map OSG indices to FBX mesh indices
typedef std::vector<const osg::Geometry*> GeometryList; // a list of geometries to process in batch

namespace pluginfbx
{

///\author Capo (Thibault Caporal), Sukender (Benoit Neil)
class WriterNodeVisitor: public osg::NodeVisitor
{
    public:
        WriterNodeVisitor(FbxScene* pScene,
                          FbxManager* pSdkManager,
                          const std::string& fileName,
                          const osgDB::ReaderWriter::Options* options,
                          const std::string& srcDirectory) :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _pSdkManager(pSdkManager),
            _pScene(pScene),
            _curFbxNode(pScene->GetRootNode()),
            _currentStateSet(new osg::StateSet()),
            _lastMaterialIndex(0),
            _lastMeshIndex(0),
            _options(options),
            _externalWriter(srcDirectory, osgDB::getFilePath(fileName), true, 0),
            _texcoords(false),
            _drawableNum(0),
            _firstNodeProcessed(false)
        {}

        virtual void apply(osg::Geometry& node);
        virtual void apply(osg::Group& node);
        virtual void apply(osg::MatrixTransform& node);
        
        void traverse (osg::Node& node)
        {
            pushStateSet(node.getStateSet());
            osg::NodeVisitor::traverse(node);
            popStateSet(node.getStateSet());
        }

        void pushStateSet(const osg::StateSet* ss)
        {
            if (ss)
            {
                // Save our current stateset
                _stateSetStack.push(_currentStateSet.get());

                // merge with node stateset
                _currentStateSet = static_cast<osg::StateSet*>(
                    _currentStateSet->clone(osg::CopyOp::SHALLOW_COPY));
                _currentStateSet->merge(*ss);
            }
        }


        void popStateSet(const osg::StateSet* ss)
        {
            if (ss)
            {
                // restore the previous stateset
                _currentStateSet = _stateSetStack.top();
                _stateSetStack.pop();
            }
        }

        /// Copy the texture file in current path.
        void copyTexture();
        typedef std::map<const osg::Image*, std::string> ImageSet;
        typedef std::set<std::string> ImageFilenameSet;        // Sub-optimal because strings are doubled (in ImageSet). Moreover, an unordered_set (= hashset) would be more efficient (Waiting for unordered_set to be included in C++ standard ;) ).

        ///\todo Add support for 2nd texture, opacity_map, bump_map, specular_map, shininess_map, self_illum_map, reflection_map.
        class Material
        {
        public:
            ///Create a KfbxMaterial and KfbxTexture from osg::Texture and osg::Material.
            Material(WriterNodeVisitor&   writerNodeVisitor,
                     osgDB::ExternalFileWriter & externalWriter,
                     const osg::StateSet* stateset,
                     const osg::Material* mat,
                     const osg::Texture*  tex,
                     FbxManager*      pSdkManager,
                     const osgDB::ReaderWriter::Options * options,
                     int                  index = -1);

            FbxFileTexture* getFbxTexture() const
            {
                return _fbxTexture;
            }

            FbxSurfaceMaterial* getFbxMaterial() const
            {
                return _fbxMaterial;
            }

            const osg::Image* getOsgImage() const
            {
                return _osgImage;
            }

            const int getIndex() const
            {
                return _index;
            }

            void setIndex(int index)
            {
                _index = index;
            }

        private:
            FbxSurfacePhong*  _fbxMaterial;
            FbxFileTexture*   _fbxTexture;
            int                _index;///< Index in the Map
            const osg::Image*  _osgImage;
        };

    protected:
        /// Compares StateSets.
        ///\todo It may be useful to compare stack of pointers (see pushStateset()) in order to keep the same number of FBX materials when doing reading and then writing without further processing.
        struct CompareStateSet
        {
            bool operator () (const osg::ref_ptr<const osg::StateSet>& ss1, const osg::ref_ptr<const osg::StateSet>& ss2) const
            {
                return *ss1 < *ss2;
            }
        };

    private:

        /**
        *  Fill the faces field of the mesh and call buildMesh().
        *  \param name the name to assign to the Fbx Mesh
        *  \param geometryList is the list of geometries which contains the vertices and faces.
        *  \param listTriangles contain all the mesh's faces.
        *  \param texcoords tell us if we have to handle texture coordinates.
        */
        void buildFaces(const std::string& name,
                        const GeometryList& geometryList,
                        ListTriangle&       listTriangles,
                        bool                texcoords);

        /// Set the layer for texture and Material in layer 0.
        void setLayerTextureAndMaterial(FbxMesh* mesh);

        /// Set Vertices, normals, and UVs
        void setControlPointAndNormalsAndUV(const GeometryList& geometryList,
                                            MapIndices&       index_vert,
                                            bool              texcoords,
                                            FbxMesh*         fbxMesh);

        /**
        *  Create the list of faces from the geode.
        *  \param geo is the geode to study.
        *  \param listTriangles is the list to fill.
        *  \param texcoords tell us if we have to treat texture coord.
        *  \param drawable_n tell us which drawable we are building.
        */
        void createListTriangle(const osg::Geometry* geo,
                                ListTriangle&        listTriangles,
                                bool&                texcoords,
                                unsigned int         drawable_n);

        ///Store the material of the stateset in the MaterialMap.
        int processStateSet(const osg::StateSet* stateset);

        typedef std::stack<osg::ref_ptr<osg::StateSet> > StateSetStack;
        typedef std::map<osg::ref_ptr<const osg::StateSet>, Material, CompareStateSet> MaterialMap;

        ///We need this for every new Node we create.
        FbxManager* _pSdkManager;

        ///Tell us if the last apply succeed, useful to stop going through the graph.
        bool _succeedLastApply;

        ///Marks if the first node is processed.
        bool _firstNodeProcessed;
		
        ///The current directory.
        std::string _directory;

        ///The Scene to save.
        FbxScene* _pScene;

        ///The current Fbx Node.
        FbxNode* _curFbxNode;

        ///The Stack of different stateSet.
        StateSetStack _stateSetStack;

        ///The current stateSet.
        osg::ref_ptr<osg::StateSet> _currentStateSet;

        ///We store the fbx Materials and Textures in this map.
        MaterialMap                         _materialMap;
        unsigned int                        _lastMaterialIndex;
        unsigned int                        _lastMeshIndex;
        const osgDB::ReaderWriter::Options* _options;
        osgDB::ExternalFileWriter           _externalWriter;

        ///Maintain geode state between visits to the geometry
        GeometryList _geometryList;
        ListTriangle _listTriangles;
        bool _texcoords;
        unsigned int _drawableNum;
};

// end namespace pluginfbx
}

#endif // _FBX_WRITER_NODE_VISITOR_HEADER__
