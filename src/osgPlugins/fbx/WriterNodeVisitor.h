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

#if defined(_MSC_VER)
#pragma warning( disable : 4505 )
#endif
#include <fbxsdk.h>

struct Triangle
{
    unsigned int t1;
    unsigned int t2;
    unsigned int t3;
    int material;
};

typedef std::map<std::pair<unsigned int, unsigned int>, unsigned int> MapIndices;
typedef std::vector<std::pair<Triangle, int> > ListTriangle; //the int is the drawable of the triangle

namespace pluginfbx
{

///\author Capo (Thibault Caporal)
class WriterNodeVisitor: public osg::NodeVisitor
{
    public:
        WriterNodeVisitor(KFbxScene* pScene,
                          KFbxSdkManager* pSdkManager,
                          const std::string& fileName,
                          const osgDB::ReaderWriter::Options* options,
                          const std::string& srcDirectory) :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _pScene(pScene),
            _pSdkManager(pSdkManager),
            _currentStateSet(new osg::StateSet()),
            _lastMaterialIndex(0),
            _lastMeshIndex(0),
            _lastGeneratedImageFileName(0),
            _curFbxNode(pScene->GetRootNode()),
            _options(options),
            _succeedLastApply(true),
            _directory(osgDB::getFilePath(fileName)),
            _srcDirectory(srcDirectory)
        {}

        ///Tell us if last Node succeed traversing.
        bool succeedLastApply() const { return _succeedLastApply; }

        ///Set the flag _succeedLastApply to false.
        void failedApply() { _succeedLastApply = false; }

        virtual void apply(osg::Geode& node);
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
                     const std::string&   srcDirectory,
                     const osg::StateSet* stateset,
                     const osg::Material* mat,
                     const osg::Texture*  tex,
                     KFbxSdkManager*      pSdkManager,
                     const std::string&   directory,
                     ImageSet&            imageSet,
                     ImageFilenameSet&    imageFilenameSet,
                     unsigned int&        lastGeneratedImageFileName,
                     int                  index = -1);

            KFbxTexture* getFbxTexture() const
            {
                return _fbxTexture;
            }

            KFbxSurfaceMaterial* getFbxMaterial() const
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
            KFbxSurfacePhong*  _fbxMaterial;
            KFbxTexture*       _fbxTexture;
            int                _index;///< Index in the Map
            const osg::Image*  _osgImage;
            const std::string& _directory;
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
        *  \param geo is the geode which contains the vertices and faces.
        *  \param listTriangles contain all the mesh's faces.
        *  \param texcoords tell us if we have to handle texture coordinates.
        */
        void buildFaces(const osg::Geode&   geo,
                        ListTriangle&       listTriangles,
                        bool                texcoords);

        /// Set the layer for texture and Material in layer 0.
        void setLayerTextureAndMaterial(KFbxMesh* mesh);

        /// Set Vertices, normals, and UVs
        void setControlPointAndNormalsAndUV(const osg::Geode& geo,
                                            MapIndices&       index_vert,
                                            bool              texcoords,       
                                            KFbxMesh*         fbxMesh);

        /**
        *  Add a vertex to the index and link him with the Triangle index and the drawable.
        *  \param index_vert is the map where the vertices are stored.
        *  \param index is the indices of the vertices position in the vec3.
        *  \param drawable_n is the number of the drawable.
        *  \return the position of the vertices in the final mesh.
        */
        unsigned int getMeshIndexForGeometryIndex(MapIndices&  index_vert,
                                                  unsigned int index,
                                                  unsigned int drawable_n);

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
                                unsigned int&        drawable_n);

        ///Store the material of the stateset in the MaterialMap.
        int processStateSet(const osg::StateSet* stateset);

        typedef std::stack<osg::ref_ptr<osg::StateSet> > StateSetStack;
        typedef std::map<osg::ref_ptr<const osg::StateSet>, Material, CompareStateSet> MaterialMap;
       
        ///We need this for every new Node we create.
        KFbxSdkManager* _pSdkManager;

        ///Tell us if the last apply succeed, useful to stop going through the graph.
        bool _succeedLastApply;

        ///The current directory.
        std::string _directory;

        ///The Scene to save.
        KFbxScene* _pScene;

        ///The current Fbx Node.
        KFbxNode* _curFbxNode;

        ///The Stack of different stateSet.
        StateSetStack _stateSetStack;

        ///The current stateSet.
        osg::ref_ptr<osg::StateSet> _currentStateSet;

        ///We store the fbx Materials and Textures in this map.
        MaterialMap                         _materialMap;
        ImageSet                            _imageSet;
        ImageFilenameSet                    _imageFilenameSet;
        unsigned int                        _lastGeneratedImageFileName;
        unsigned int                        _lastMaterialIndex;
        unsigned int                        _lastMeshIndex;
        const osgDB::ReaderWriter::Options* _options;
        const std::string                   _srcDirectory;
};

// end namespace pluginfbx
}

#endif // _FBX_WRITER_NODE_VISITOR_HEADER__
