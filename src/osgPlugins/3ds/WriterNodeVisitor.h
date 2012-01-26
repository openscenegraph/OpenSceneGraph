// -*-c++-*-

/*
* 3DS reader/writer for Open Scene Graph
*
* Copyright (C) ???
*
* Writing support added 2009 by Sukender (Benoit Neil), http://sukender.free.fr,
* strongly inspired by the OBJ writer object by Stephan Huber
*
* The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
* real-time rendering of large 3D photo-realistic models.
* The OSG homepage is http://www.openscenegraph.org/
*/

#ifndef _3DS_WRITER_NODE_VISITOR_HEADER__
#define _3DS_WRITER_NODE_VISITOR_HEADER__

#include <string>
#include <stack>
#include <sstream>

#include <osg/Notify>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexMat>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <map>

#include "lib3ds/lib3ds.h"
#include "WriterCompareTriangle.h"
#include <set>

void copyOsgMatrixToLib3dsMatrix(Lib3dsMatrix lib3ds_matrix, const osg::Matrix& osg_matrix);

typedef std::map<std::pair<unsigned int, unsigned int>, unsigned int> MapIndices;
typedef std::vector<std::pair<Triangle, int> > ListTriangle; //the int is the drawable of the triangle

namespace plugin3ds
{

class WriterNodeVisitor: public osg::NodeVisitor
{
    public:
        static const unsigned int MAX_VERTICES = 65000;
        static const unsigned int MAX_FACES    = MAX_VERTICES;

        WriterNodeVisitor(Lib3dsFile * file3ds, const std::string & fileName, 
                        const osgDB::ReaderWriter::Options* options, 
                        const std::string & srcDirectory);

        bool succeeded() const { return _succeeded; }
        virtual void apply(osg::Geode &node);
        virtual void apply(osg::Billboard &node);

        virtual void apply(osg::Group &node);
        virtual void apply(osg::MatrixTransform &node);

        void traverse (osg::Node &node)
        {
            pushStateSet(node.getStateSet());
            osg::NodeVisitor::traverse( node );
            popStateSet(node.getStateSet());
        }

        void pushStateSet(osg::StateSet* ss)
        {
        if (NULL!=ss) {
            // Save our current stateset
            _stateSetStack.push(_currentStateSet.get());

            // merge with node stateset
            _currentStateSet = static_cast<osg::StateSet*>(_currentStateSet->clone(osg::CopyOp::SHALLOW_COPY));
            _currentStateSet->merge(*ss);
        }
        }


        void popStateSet(osg::StateSet* ss)
        {
            if (NULL!=ss) {
            // restore the previous stateset
            _currentStateSet = _stateSetStack.top();
            _stateSetStack.pop();
            }
        }


        void writeMaterials();



        ///\todo Add support for 2nd texture, opacity_map, bump_map, specular_map, shininess_map, self_illum_map, reflection_map.
        class Material {
            public:
                Material(WriterNodeVisitor & writerNodeVisitor, osg::StateSet * stateset, osg::Material* mat, osg::Texture* tex, int index=-1);

                int index;            ///< Index in the 3DS file
                osg::Vec4 diffuse, ambient, specular;
                float shininess;
                float transparency;
                bool  double_sided;
                std::string name;
                osg::ref_ptr<osg::Image> image;
                bool texture_transparency;
                bool texture_no_tile;
            protected:
                Material() : index(-1) {}

        };

    protected:
        struct CompareStateSet
        {
            bool operator()(const osg::ref_ptr<osg::StateSet>& ss1, const osg::ref_ptr<osg::StateSet>& ss2) const
            {
                return *ss1 < *ss2;
            }
        };


    private:
        WriterNodeVisitor& operator = (const WriterNodeVisitor&) { return *this; }

        /** 
        *  Fill the faces field of the mesh and call buildMesh().
        *  \param geo is the geode who contain vertice and faces.
        *  \param mat Local to world matrix applied to the geode
        *  \param listTriangles contain all the meshs faces.
        *  \param texcoords tell us if we have to treat texture coord.
        */
        void buildFaces(osg::Geode & geo, const osg::Matrix & mat, ListTriangle & listTriangles, bool texcoords);

        /** 
        *  Calculate the number of vertices in the geode.
        *  \return the number of vertices in the geode.
        */
        unsigned int calcVertices(osg::Geode & geo);

        /** 
        *  Build a mesh
        *  \param geo is the geode who contain vertice and faces
        *  \param mat Local to world matrix applied to the geode
        *  \param index_vert is the index used to build the new mesh
        *  \param texcoords tell us if we have to treat texture coord
        *  \param mesh is the mesh with faces filled
        *  \sa See cutScene() about the definition of the boxes for faces sorting.
        */
        void
        buildMesh(osg::Geode        &    geo,
                  const osg::Matrix &    mat,
                  MapIndices        &    index_vert,
                  bool                   texcoords,       
                  Lib3dsMesh             *mesh);

        /** 
        *  Add a vertice to the index and link him with the Triangle index and the drawable.
        *  \param index_vert is the map where the vertice are stored.
        *  \param index is the indice of the vertice's position in the vec3.
        *  \param drawable_n is the number of the drawable.
        *  \return the position of the vertice in the final mesh.
        */
        unsigned int
        getMeshIndexForGeometryIndex(MapIndices & index_vert, 
                                     unsigned int index,
                                     unsigned int drawable_n);
        /** 
        *  Create the list of faces from the geode.
        *  \param geo is the geode to study.
        *  \param listTriangles is the list to fill.
        *  \param texcoords tell us if we have to treat texture coord.
        *  \param drawable_n tell us which drawable we are building.
        */ 
        void createListTriangle(osg::Geometry       *    geo, 
                                ListTriangle        &    listTriangles,
                                bool                &    texcoords,
                                unsigned int        &    drawable_n);

        int processStateSet(osg::StateSet* stateset);

        std::string getUniqueName(const std::string& defaultvalue, bool isNodeName, const std::string & defaultPrefix = "", int currentPrefixLen = -1);
        std::string export3DSTexture(const osg::Image * image, const std::string & fileName);

        typedef std::stack<osg::ref_ptr<osg::StateSet> > StateSetStack;
        typedef std::map< osg::ref_ptr<osg::StateSet>, Material, CompareStateSet> MaterialMap;

        void apply3DSMatrixNode(osg::Node &node, const osg::Matrix * m, const char * const prefix);

        bool                                _succeeded;
        std::string                         _directory;
        std::string                         _srcDirectory;
        Lib3dsFile *                        _file3ds;
        StateSetStack                       _stateSetStack;
        osg::ref_ptr<osg::StateSet>         _currentStateSet;
        typedef std::map<std::string, unsigned int> PrefixMap;
        PrefixMap                           _nodePrefixMap;            ///< List of next number to use in unique name generation, for each prefix
        PrefixMap                           _imagePrefixMap;
        typedef std::set<std::string> NameMap;
        NameMap                             _nodeNameMap;
        NameMap                             _imageNameMap;
        MaterialMap                         _materialMap;
        unsigned int                        _lastMaterialIndex;
        unsigned int                        _lastMeshIndex;
        Lib3dsMeshInstanceNode *            _cur3dsNode;
        const osgDB::ReaderWriter::Options* _options;
        unsigned int                        _imageCount;
        bool                                _extendedFilePaths;
        typedef std::map<osg::Image*, std::string> ImageSet;
        ImageSet                            _imageSet;                 ///< Map used to avoid renaming and writing twice an image
};

// end namespace plugin3ds
}

#endif
