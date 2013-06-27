
#include <osg/Notify>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Material>
#include <osg/TexEnv>
#include <osg/ref_ptr>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/TexEnvCombine>
#include <osg/CullFace>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include <osgUtil/TriStripVisitor>

//MIKEC debug only for PrintVisitor
#include <osg/NodeVisitor>

#include "WriterNodeVisitor.h"
#include "lib3ds/lib3ds.h"
#include <stdlib.h>
#include <string.h>

#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <assert.h>

using namespace std;
using namespace osg;

/// Implementation borrowed from boost and slightly modified
template<class T> class scoped_array // noncopyable
{
private:
    T * px;
    scoped_array(scoped_array const &);
    scoped_array & operator=(scoped_array const &);
    typedef scoped_array<T> this_type;

    void operator==( scoped_array const& ) const;
    void operator!=( scoped_array const& ) const;

public:
    typedef T element_type;
    explicit scoped_array( T * p = 0 ) : px( p ) {}

    ~scoped_array() { delete[] px; }

    void reset(T * p = 0) {
        assert( p == 0 || p != px ); // catch self-reset errors
        this_type(p).swap(*this);
    }

    T & operator[](std::ptrdiff_t i) const // never throws
    {
        assert( px != 0 );
        assert( i >= 0 );
        return px[i];
    }

    T * get() const // never throws
    {
        return px;
    }

    void swap(scoped_array & b) // never throws
    {
        T * tmp = b.px;
        b.px = px;
        px = tmp;
    }
};



void copyLib3dsMatrixToOsgMatrix(osg::Matrix& osg_matrix, const Lib3dsMatrix lib3ds_matrix)
{
    osg_matrix.set(
        lib3ds_matrix[0][0],lib3ds_matrix[0][1],lib3ds_matrix[0][2],lib3ds_matrix[0][3],
        lib3ds_matrix[1][0],lib3ds_matrix[1][1],lib3ds_matrix[1][2],lib3ds_matrix[1][3],
        lib3ds_matrix[2][0],lib3ds_matrix[2][1],lib3ds_matrix[2][2],lib3ds_matrix[2][3],
        lib3ds_matrix[3][0],lib3ds_matrix[3][1],lib3ds_matrix[3][2],lib3ds_matrix[3][3]);
}

osg::Matrix copyLib3dsMatrixToOsgMatrix(const Lib3dsMatrix mat)
{
    osg::Matrix osgMatrix;
    copyLib3dsMatrixToOsgMatrix(osgMatrix, mat);
    return osgMatrix;
}

void copyLib3dsVec3ToOsgVec3(osg::Vec3f osgVec, const float vertices[3])
{
    return osgVec.set(vertices[0], vertices[1], vertices[2]);
}

osg::Vec3f copyLib3dsVec3ToOsgVec3(const float vertices[3])
{
    return osg::Vec3f(vertices[0], vertices[1], vertices[2]);
}

osg::Quat copyLib3dsQuatToOsgQuat(const float quat[4])
{
    return osg::Quat(quat[0], quat[1], quat[2], quat[3]);
}



class PrintVisitor : public NodeVisitor
{
public:
    PrintVisitor(std::ostream& out):
      NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN),
          _out(out)
      {
          _indent = 0;
          _step = 4;
      }

      inline void moveIn() { _indent += _step; }
      inline void moveOut() { _indent -= _step; }
      inline void writeIndent()
      {
          for(int i=0;i<_indent;++i) _out << " ";
      }

      virtual void apply(Node& node)
      {
          moveIn();
          writeIndent(); _out << node.className() <<std::endl;
          traverse(node);
          moveOut();
      }

      virtual void apply(Geode& node)         { apply((Node&)node); }
      virtual void apply(Billboard& node)     { apply((Geode&)node); }
      virtual void apply(LightSource& node)   { apply((Group&)node); }
      virtual void apply(ClipNode& node)      { apply((Group&)node); }

      virtual void apply(Group& node)         { apply((Node&)node); }
      virtual void apply(Transform& node)     { apply((Group&)node); }
      virtual void apply(Projection& node)    { apply((Group&)node); }
      virtual void apply(Switch& node)        { apply((Group&)node); }
      virtual void apply(LOD& node)           { apply((Group&)node); }

protected:

    PrintVisitor& operator = (const PrintVisitor&) { return *this; }

    std::ostream& _out;
    int _indent;
    int _step;
};

/// Possible options:
///        - noMatrixTransforms: set the plugin to apply matrices into the mesh vertices ("old behaviour") instead of restoring them ("new behaviour"). You may use this option to avoid a few rounding errors.
///        - checkForEspilonIdentityMatrices: if noMatrixTransforms is \b not set, then consider "almost identity" matrices to be identity ones (in case of rounding errors).
///        - restoreMatrixTransformsNoMeshes: makes an exception to the behaviour when 'noMatrixTransforms' is \b not set for mesh instances. When a mesh instance has a transform on it, the reader creates a MatrixTransform above the Geode. If you don't want the hierarchy to be modified, then you can use this option to merge the transform into vertices.
class ReaderWriter3DS : public osgDB::ReaderWriter
{
public:

    ReaderWriter3DS();

    virtual const char* className() const { return "3DS Auto Studio Reader/Writer"; }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const;
    virtual ReadResult readNode(std::istream& fin, const Options* options) const;
    virtual ReadResult doReadNode(std::istream& fin, const Options* options, const std::string & fileNamelib3ds) const;        ///< Subfunction of readNode()s functions.

    virtual WriteResult writeNode(const osg::Node& /*node*/,const std::string& /*fileName*/,const Options* =NULL) const;
    virtual WriteResult writeNode(const osg::Node& /*node*/,std::ostream& /*fout*/,const Options* =NULL) const;
    virtual WriteResult doWriteNode(const osg::Node& /*node*/,std::ostream& /*fout*/,const Options*, const std::string & fileNamelib3ds) const;

protected:
    ReadResult constructFrom3dsFile(Lib3dsFile *f,const std::string& filename, const Options* options) const;

    bool createFileObject(const osg::Node& node, Lib3dsFile * file3ds,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;

    /// An OSG state set with the original 3DS material attached (used to get info such as UV scaling & offset)
    struct StateSetInfo
    {
        StateSetInfo(osg::StateSet * stateset=NULL, Lib3dsMaterial * lib3dsmat=NULL) : stateset(stateset), lib3dsmat(lib3dsmat) {}
        StateSetInfo(const StateSetInfo & v) : stateset(v.stateset), lib3dsmat(v.lib3dsmat) {}
        StateSetInfo & operator=(const StateSetInfo & v) { stateset=v.stateset; lib3dsmat=v.lib3dsmat; return *this; }

        osg::StateSet * stateset;
        Lib3dsMaterial * lib3dsmat;
    };

    class ReaderObject
    {
    public:
        ReaderObject(const osgDB::ReaderWriter::Options* options);

        typedef std::vector<StateSetInfo> StateSetMap;
        typedef std::vector<int> FaceList;
        typedef std::map<std::string,osg::StateSet*> GeoStateMap;

        osg::Texture2D* createTexture(Lib3dsTextureMap *texture,const char* label,bool& transparancy);
        StateSetInfo createStateSet(Lib3dsMaterial *materials);
        osg::Drawable* createDrawable(Lib3dsMesh *meshes,FaceList& faceList, const osg::Matrix * matrix, StateSetInfo & ssi, bool smoothVertexNormals);

        std::string _directory;
        bool _useSmoothingGroups;

        // MIKEC
        osg::Node* processMesh(StateSetMap& drawStateMap,osg::Group* parent,Lib3dsMesh* mesh, const osg::Matrix * matrix);
        osg::Node* processNode(StateSetMap& drawStateMap,Lib3dsFile *f,Lib3dsNode *node);
    private:
        const osgDB::ReaderWriter::Options* options;
        bool noMatrixTransforms;            ///< Should the plugin apply matrices into the mesh vertices ("old behaviour"), instead of restoring matrices ("new behaviour")?
        bool checkForEspilonIdentityMatrices;
        bool restoreMatrixTransformsNoMeshes;
        typedef std::map<unsigned int,FaceList> SmoothingFaceMap;
        void addDrawableFromFace(osg::Geode * geode, FaceList & faceList, Lib3dsMesh * mesh, const osg::Matrix * matrix, StateSetInfo & ssi);

        typedef std::map<std::string, osg::ref_ptr<osg::Texture2D> > TexturesMap;        // Should be an unordered map (faster)
        TexturesMap texturesMap;
};
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(3ds, ReaderWriter3DS)

ReaderWriter3DS::ReaderWriter3DS()
{
    supportsExtension("3ds","3D Studio model format");
    //supportsOption("OutputTextureFiles","Write out the texture images to file");
    //supportsOption("flipTexture", "flip texture upside-down");
    supportsOption("extended3dsFilePaths", "(Write option) Keeps long texture filenames (not 8.3) when exporting 3DS, but can lead to compatibility problems.");
    supportsOption("noMatrixTransforms", "(Read option) Set the plugin to apply matrices into the mesh vertices (\"old behaviour\") instead of restoring them (\"new behaviour\"). You may use this option to avoid a few rounding errors.");
    supportsOption("checkForEspilonIdentityMatrices", "(Read option) If not set, then consider \"almost identity\" matrices to be identity ones (in case of rounding errors).");
    supportsOption("restoreMatrixTransformsNoMeshes", "(Read option) Makes an exception to the behaviour when 'noMatrixTransforms' is not set for mesh instances. When a mesh instance has a transform on it, the reader creates a MatrixTransform above the Geode. If you don't want the hierarchy to be modified, then you can use this option to merge the transform into vertices.");

#if 0
    OSG_NOTICE<<"3DS reader sizes:"<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsBool)="<<sizeof(Lib3dsBool)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsByte)="<<sizeof(Lib3dsByte)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsWord)="<<sizeof(Lib3dsWord)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsDword)="<<sizeof(Lib3dsDword)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsIntb)="<<sizeof(Lib3dsIntb)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsIntw)="<<sizeof(Lib3dsIntw)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsIntd)="<<sizeof(Lib3dsIntd)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsFloat)="<<sizeof(Lib3dsFloat)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsDouble)="<<sizeof(Lib3dsDouble)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsVector)="<<sizeof(Lib3dsVector)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsTexel)="<<sizeof(Lib3dsTexel)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsQuat)="<<sizeof(Lib3dsQuat)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsMatrix)="<<sizeof(Lib3dsMatrix)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsRgb)="<<sizeof(Lib3dsRgb)<<std::endl;
    OSG_NOTICE<<"  sizeof(Lib3dsRgba)="<<sizeof(Lib3dsRgba)<<std::endl;
#endif

}

ReaderWriter3DS::ReaderObject::ReaderObject(const osgDB::ReaderWriter::Options* options) :
    _useSmoothingGroups(true),
    options(options),
    noMatrixTransforms(false),
    checkForEspilonIdentityMatrices(false),
    restoreMatrixTransformsNoMeshes(false)
{
    if (options)
    {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt)
        {
            if (opt == "noMatrixTransforms")
                noMatrixTransforms = true;
            else if (opt == "checkForEspilonIdentityMatrices")
                checkForEspilonIdentityMatrices = true;
            else if (opt == "restoreMatrixTransformsNoMeshes")
                restoreMatrixTransformsNoMeshes = true;
        }
    }
}


/**
    These print methods for 3ds hacking
*/
void pad(int level)
{
    for(int i=0;i<level;i++) std::cout<<"  ";
}
void print(Lib3dsMesh *mesh,int level);
void print(Lib3dsUserData *user,int level);
void print(Lib3dsMeshInstanceNode *object,int level);
void print(Lib3dsNode *node, int level);

void print(Lib3dsMatrix matrix,int level)
{
    pad(level); cout << matrix[0][0] <<" "<< matrix[0][1] <<" "<< matrix[0][2] <<" "<< matrix[0][3] << endl;
    pad(level); cout << matrix[1][0] <<" "<< matrix[1][1] <<" "<< matrix[1][2] <<" "<< matrix[1][3] << endl;
    pad(level); cout << matrix[2][0] <<" "<< matrix[2][1] <<" "<< matrix[2][2] <<" "<< matrix[2][3] << endl;
    pad(level); cout << matrix[3][0] <<" "<< matrix[3][1] <<" "<< matrix[3][2] <<" "<< matrix[3][3] << endl;
}
void print(Lib3dsMesh *mesh,int level)
{
    if (mesh)
    {
        pad(level); cout << "mesh name " << mesh->name  << endl;
        print(mesh->matrix,level);
    }
    else
    {
        pad(level); cout << "no mesh " << endl;
    }
}
void print(Lib3dsUserData *user,int level)
{
    if (user)
    {
        pad(level); cout << "user data" << endl;
        //print(user->mesh,level+1);
    }
    else
    {
        pad(level); cout << "no user data" << endl;
    }
}

void print(Lib3dsMeshInstanceNode *object,int level)
{
    if (object)
    {
        pad(level); cout << "objectdata instance [" << object->instance_name << "]" << endl;
        pad(level); cout << "pivot     " << object->pivot[0] <<" "<< object->pivot[1] <<" "<< object->pivot[2] << endl;
        pad(level); cout << "pos       " << object->pos[0] <<" "<< object->pos[1] <<" "<< object->pos[2] << endl;
        pad(level); cout << "scl       " << object->scl[0] <<" "<< object->scl[1] <<" "<< object->scl[2] << endl;
        pad(level); cout << "rot       " << object->rot[0] <<" "<< object->rot[1] <<" "<< object->rot[2] <<" "<< object->rot[3] << endl;
    }
    else
    {
        pad(level); cout << "no object data" << endl;
    }
}

void print(Lib3dsNode *node, int level)
{

    pad(level); cout << "node name [" << node->name << "]" << endl;
    pad(level); cout << "node id    " << node->user_id << endl;
    pad(level); cout << "node parent id " << (node->parent ? static_cast<int>(node->parent->user_id) : -1) << endl;
    pad(level); cout << "node matrix:" << endl;
    print(node->matrix,level+1);

    if (node->type == LIB3DS_NODE_MESH_INSTANCE)
    {
        pad(level); cout << "mesh instance data:" << endl;
        print(reinterpret_cast<Lib3dsMeshInstanceNode *>(node),level+1);
    }
    else
    {
        pad(level); cout << "node is not a mesh instance (not handled)" << endl;
    }

    print(&node->user_ptr,level);

    for(Lib3dsNode *child=node->childs; child; child=child->next)
    {
        print(child,level+1);
    }

}

void ReaderWriter3DS::ReaderObject::addDrawableFromFace(osg::Geode * geode, FaceList & faceList,
                                                        Lib3dsMesh * mesh,
                                                        const osg::Matrix * matrix,
                                                        StateSetInfo & ssi)
{
    if (_useSmoothingGroups)
    {
        SmoothingFaceMap smoothingFaceMap;
        for (FaceList::iterator flitr=faceList.begin();
            flitr!=faceList.end();
            ++flitr)
        {
            // ChrisD: Worth bearing in mind that this splitting up of
            // faces into smoothing groups is only correct for faces
            // belonging to a single smoothing group. The smoothing group
            // value is actually a bitmask for all the smoothing groups that
            // a face may belong to.
            smoothingFaceMap[mesh->faces[*flitr].smoothing_group].push_back(*flitr);
        }

        for(SmoothingFaceMap::iterator sitr=smoothingFaceMap.begin();
            sitr!=smoothingFaceMap.end();
            ++sitr)
        {
            // We only compute smoothed vertex normals for faces with non-zero smoothing group value.
            const unsigned int smoothing_group = sitr->first;
            bool smoothVertexNormals = (smoothing_group != 0);

            // each smoothing group to have its own geom
            // to ensure the vertices on adjacent groups
            // don't get shared.
            FaceList& smoothFaceList = sitr->second;
            osg::ref_ptr<osg::Drawable> drawable = createDrawable(mesh, smoothFaceList, matrix, ssi, smoothVertexNormals);
            if (drawable.valid())
            {
                if (ssi.stateset)
                    drawable->setStateSet(ssi.stateset);
                geode->addDrawable(drawable.get());
            }
        }
    }
    else // ignore smoothing groups.
    {
        // Create drawable with no smoothing of normals.
        bool smoothVertexNormals = false;
        osg::ref_ptr<osg::Drawable> drawable = createDrawable(mesh, faceList, matrix, ssi, smoothVertexNormals);
        if (drawable.valid())
        {
            if (ssi.stateset)
                drawable->setStateSet(ssi.stateset);
            geode->addDrawable(drawable.get());
        }
    }
}


// Transforms points by matrix if 'matrix' is not NULL
// Creates a Geode and Geometry (as parent,child) and adds the Geode to 'parent' parameter iff 'parent' is non-NULL
// Returns ptr to the Geode
osg::Node* ReaderWriter3DS::ReaderObject::processMesh(StateSetMap& drawStateMap,osg::Group* parent,Lib3dsMesh* mesh, const osg::Matrix * matrix)
{
    typedef std::vector<FaceList> MaterialFaceMap;
    MaterialFaceMap materialFaceMap;
    unsigned int numMaterials = drawStateMap.size();
    materialFaceMap.insert(materialFaceMap.begin(), numMaterials, FaceList());        // Setup the map
    FaceList defaultMaterialFaceList;
    for (unsigned int i=0; i<mesh->nfaces; ++i)
    {
        if (mesh->faces[i].material>=0)
        {
            materialFaceMap[mesh->faces[i].material].push_back(i);
        }
        else
        {
            defaultMaterialFaceList.push_back(i);
        }
    }
    if (materialFaceMap.empty() && defaultMaterialFaceList.empty())
    {
        OSG_NOTICE<<"Warning : no triangles assigned to mesh '"<<mesh->name<<"'"<< std::endl;
        //OSG_INFO << "No material assigned to mesh '" << mesh->name << "'" << std::endl;
        return NULL;
    }
    else
    {
        osg::Geode* geode = new osg::Geode;
        geode->setName(mesh->name);
        if (!defaultMaterialFaceList.empty())
        {
            StateSetInfo emptySSI;
            addDrawableFromFace(geode, defaultMaterialFaceList, mesh, matrix, emptySSI);
        }
        for(unsigned int imat=0; imat<numMaterials; ++imat)
        {
            addDrawableFromFace(geode, materialFaceMap[imat], mesh, matrix, drawStateMap[imat]);
        }
        if (parent) parent->addChild(geode);
        return geode;
    }
}


/// Returns true if a matrix is 'almost' identity, meaning that the difference between each value and the corresponding identity value is less than an epsilon value.
bool isIdentityEquivalent(const osg::Matrix & mat, osg::Matrix::value_type epsilon=1e-6)
{
    return osg::equivalent(mat(0,0), 1, epsilon) && osg::equivalent(mat(0,1), 0, epsilon) && osg::equivalent(mat(0,2), 0, epsilon) &&  osg::equivalent(mat(0,3), 0, epsilon) &&
           osg::equivalent(mat(1,0), 0, epsilon) && osg::equivalent(mat(1,1), 1, epsilon) && osg::equivalent(mat(1,2), 0, epsilon) &&  osg::equivalent(mat(1,3), 0, epsilon) &&
           osg::equivalent(mat(2,0), 0, epsilon) && osg::equivalent(mat(2,1), 0, epsilon) && osg::equivalent(mat(2,2), 1, epsilon) &&  osg::equivalent(mat(2,3), 0, epsilon) &&
           osg::equivalent(mat(3,0), 0, epsilon) && osg::equivalent(mat(3,1), 0, epsilon) && osg::equivalent(mat(3,2), 0, epsilon) &&  osg::equivalent(mat(3,3), 1, epsilon);
}


/**
How to cope with pivot points in 3ds (short version)

  All object coordinates in 3ds are stored in world space, this is why you can just rip out the meshes and use/draw them without meddeling further
  Unfortunately, this gets a bit wonky with objects with pivot points (conjecture: PP support is retro fitted into the .3ds format and so doesn't fit perfectly?)

  Objects with pivot points have a position relative to their PP, so they have to undergo this transform:

    invert the mesh matrix, apply this matrix to the object. This puts the object back at the origin
    Transform the object by the nodes (negative) pivot point coords, this puts the PP at the origin
    Transform the node by the node matrix, which does the orientation about the pivot point, (and currently) transforms the object back by a translation to the PP.

*/
osg::Node* ReaderWriter3DS::ReaderObject::processNode(StateSetMap& drawStateMap,Lib3dsFile *f,Lib3dsNode *node)
{
    // Get mesh
    Lib3dsMeshInstanceNode * object = (node->type == LIB3DS_NODE_MESH_INSTANCE) ? reinterpret_cast<Lib3dsMeshInstanceNode *>(node) : NULL;
    Lib3dsMesh * mesh = lib3ds_file_mesh_for_node(f,node);
    assert(!(mesh && !object));        // Node must be a LIB3DS_NODE_MESH_INSTANCE if a mesh exists

    // Retreive LOCAL transform
    static const osg::Matrix::value_type MATRIX_EPSILON = 1e-10;
    osg::Matrix osgWorldToNodeMatrix( copyLib3dsMatrixToOsgMatrix(node->matrix) );
    osg::Matrix osgWorldToParentNodeMatrix;
    if (node->parent)
    {
        // Matrices evaluated by lib3DS are multiplied by parents' ones
        osgWorldToParentNodeMatrix = copyLib3dsMatrixToOsgMatrix(node->parent->matrix);
    }
    osg::Matrix osgNodeMatrix( osgWorldToNodeMatrix * osg::Matrix::inverse(osgWorldToParentNodeMatrix) );

    // Test if we should create an intermediate Group (or MatrixTransform) and which matrix we should apply to the vertices
    osg::Group* group = NULL;

    // Get pivot point
    osg::Vec3 pivot( object ? copyLib3dsVec3ToOsgVec3(object->pivot) : osg::Vec3() );
    bool pivoted = pivot.x()!=0 || pivot.y()!=0 || pivot.z()!=0;

    osg::Matrix meshMat;
    if (mesh)
    {
        if (!noMatrixTransforms)
        {
            // There can be a transform directly on a mesh instance (= as if a osg::MatrixTransform and a osg::Geode were merged together) in object->pos/rot/scl
            if (pivoted) {
                meshMat = osg::Matrix::inverse(copyLib3dsMatrixToOsgMatrix(mesh->matrix)) * osg::Matrix::translate(-pivot);
            } else {
                meshMat = osg::Matrix::inverse(copyLib3dsMatrixToOsgMatrix(mesh->matrix));
            }
        }
        else {
            if (pivoted) {
                meshMat = osg::Matrix::inverse(copyLib3dsMatrixToOsgMatrix(mesh->matrix)) * osg::Matrix::translate(-pivot) * osgWorldToNodeMatrix;
            } else {
                meshMat = osg::Matrix::inverse(copyLib3dsMatrixToOsgMatrix(mesh->matrix)) * osgWorldToNodeMatrix;        // ==Identity when not pivoted?
            }
            osgNodeMatrix = osg::Matrix::identity();        // Not sure it's useful, but it's harmless ;)
        }
    }

    bool isOsgNodeMatrixIdentity = false;
    if (osgNodeMatrix.isIdentity() || (checkForEspilonIdentityMatrices && isIdentityEquivalent(osgNodeMatrix, MATRIX_EPSILON)))
    {
        isOsgNodeMatrixIdentity = true;
    }


    //if (node->childs != NULL || pivoted || (!isOsgNodeMatrixIdentity && !noMatrixTransforms))
    if (node->childs != NULL || (!isOsgNodeMatrixIdentity && !noMatrixTransforms))
    {
        if (isOsgNodeMatrixIdentity || noMatrixTransforms)
        {
            group = new osg::Group;
        }
        else
        {
            group = new osg::MatrixTransform(osgNodeMatrix);
        }
    }

    if (group)
    {
        if (strcmp(node->name, "$$$DUMMY") == 0)
        {
            if (node->type == LIB3DS_NODE_MESH_INSTANCE)
                group->setName(reinterpret_cast<Lib3dsMeshInstanceNode *>(node)->instance_name);
        }
        else if (node->type == LIB3DS_NODE_MESH_INSTANCE && strlen(reinterpret_cast<Lib3dsMeshInstanceNode *>(node)->instance_name) != 0)
            group->setName(reinterpret_cast<Lib3dsMeshInstanceNode *>(node)->instance_name);
        else
            group->setName(node->name);

        // Handle all children of this node for hierarchical assemblies
        for (Lib3dsNode *p=node->childs; p!=NULL; p=p->next)
        {
            group->addChild(processNode(drawStateMap,f,p));
        }
    }
    else
    {
        assert(node->childs == NULL);        // Else we must have a group to put childs into
    }

    // Handle mesh
    if (mesh)
    {
        osg::Matrix * meshAppliedMatPtr = NULL;
        if (!meshMat.isIdentity() && !(checkForEspilonIdentityMatrices && isIdentityEquivalent(meshMat, MATRIX_EPSILON)))
        {
            meshAppliedMatPtr = &meshMat;
        }

        // MIKEC: "group" handles the node transform, siblings, and our child geometry if we don't want an MT node for it
        // if noMatrixTransforms is not set, we create a transform node for the mesh's matrix
        osg::Group* meshTransform=NULL;

        if ((noMatrixTransforms==false) && meshAppliedMatPtr) { // we are allowed to have, and need another matrixtransform
            meshTransform=new osg::MatrixTransform(meshMat);
            meshAppliedMatPtr=NULL; // since meshTransform applies it

            meshTransform->setName("3DSMeshMatrix");
            if (group) group->addChild(meshTransform);
        } else {
            meshTransform=group; // don't need the meshTransform node - note group can be NULL
        }

        if (group)
        {
            // add our geometry to group (where our children already are)
            // creates geometry under modifier node
            processMesh(drawStateMap,meshTransform,mesh,meshAppliedMatPtr);
            return group;
        }
        else
        {
            // didnt use group for children, return a ptr directly to the Geode for this mesh
            // there is no group node but may have a meshTransform node to hold the meshes matrix
            if (meshTransform) {
                processMesh(drawStateMap,meshTransform,mesh,meshAppliedMatPtr);
                return meshTransform;
            } else { // no group or meshTransform nodes - create a new Geode and return that
                return processMesh(drawStateMap,NULL,mesh,meshAppliedMatPtr);
            }
        }

    }
    else
    {
        // no mesh for this node - probably a camera or something of that persuasion
        //cout << "no mesh for object " << node->name << endl;
        return group; // we have no mesh, but we might have children
    }
}


static long filei_seek_func(void *self, long offset, Lib3dsIoSeek origin)
{
    std::istream *f = reinterpret_cast<std::istream*>(self);
    ios_base::seekdir o = ios_base::beg;
    if (origin == LIB3DS_SEEK_CUR) o = ios_base::cur;
    else if (origin == LIB3DS_SEEK_END) o = ios_base::end;

    f->seekg(offset, o);
    return f->fail() ? -1 : 0;
}

static long fileo_seek_func(void *self, long offset, Lib3dsIoSeek origin)
{
    std::ostream *f = reinterpret_cast<std::ostream*>(self);
    ios_base::seekdir o = ios_base::beg;
    if (origin == LIB3DS_SEEK_CUR) o = ios_base::cur;
    else if (origin == LIB3DS_SEEK_END) o = ios_base::end;

    f->seekp(offset, o);
    return f->fail() ? -1 : 0;
}

static long filei_tell_func(void *self)
{
    std::istream *f = reinterpret_cast<std::istream*>(self);
    return f->tellg();
}

static long fileo_tell_func(void *self)
{
    std::ostream *f = reinterpret_cast<std::ostream*>(self);
    return f->tellp();
}


static size_t filei_read_func(void *self, void *buffer, size_t size)
{
    std::istream *f = reinterpret_cast<std::istream*>(self);
    f->read(reinterpret_cast<char*>(buffer), size);
    return f->gcount();
}

static size_t fileo_write_func(void *self, const void *buffer, size_t size)
{
    std::ostream *f = reinterpret_cast<std::ostream*>(self);
    f->write(static_cast<const char*>(buffer), size);
    return f->fail() ? 0 : size;
}

static void fileio_log_func(void *self, Lib3dsLogLevel level, int indent, const char *msg)
{
    osg::NotifySeverity l = osg::INFO;
    // Intentionally NOT mapping 3DS levels with OSG levels
    if (level == LIB3DS_LOG_ERROR) l = osg::WARN;
    else if (level == LIB3DS_LOG_WARN) l = osg::NOTICE;
    else if (level == LIB3DS_LOG_INFO) l = osg::INFO;
    else if (level == LIB3DS_LOG_DEBUG) l = osg::DEBUG_INFO;
    OSG_NOTIFY(l) << msg << std::endl;
}


osgDB::ReaderWriter::ReadResult ReaderWriter3DS::readNode(std::istream& fin,  const osgDB::ReaderWriter::Options* options) const
{
    std::string optFileName;
    if (options)
    {
        optFileName = options->getPluginStringData("STREAM_FILENAME");
        if (optFileName.empty()) optFileName = options->getPluginStringData("filename");
    }
    return doReadNode(fin, options, optFileName);
}

osgDB::ReaderWriter::ReadResult ReaderWriter3DS::doReadNode(std::istream& fin,  const osgDB::ReaderWriter::Options* options, const std::string & fileNamelib3ds) const
{
    osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileNamelib3ds));

    osgDB::ReaderWriter::ReadResult result = ReadResult::FILE_NOT_HANDLED;

    // Prepare io structure to tell how to read the stream
    Lib3dsIo io;
    io.self = &fin;
    io.seek_func = filei_seek_func;
    io.tell_func = filei_tell_func;
    io.read_func = filei_read_func;
    io.write_func = NULL;
    io.log_func = fileio_log_func;

    Lib3dsFile * file3ds = lib3ds_file_new();
    if (lib3ds_file_read(file3ds, &io) != 0)
    {
        result = constructFrom3dsFile(file3ds,fileNamelib3ds,options);
        lib3ds_file_free(file3ds);
    }

    return(result);
}

osgDB::ReaderWriter::ReadResult ReaderWriter3DS::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // Do not use the lib3ds_file_open() as:
    //   1. It relies on FILE* instead of iostreams (less safe)
    //   2. It doesn't allow us to set a custom log output
    osgDB::ifstream fin(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!fin.good()) return ReadResult::ERROR_IN_READING_FILE;
    return doReadNode(fin, options, fileName);
/*
    osgDB::ReaderWriter::ReadResult result = ReadResult::FILE_NOT_HANDLED;
    Lib3dsFile *f = lib3ds_file_open(fileName.c_str());        // ,options

    if (f)
    {
        osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

        result = constructFrom3dsFile(f,file,local_opt.get());
        lib3ds_file_free(f);
    }

    return result;
*/
}

osgDB::ReaderWriter::ReadResult ReaderWriter3DS::constructFrom3dsFile(Lib3dsFile *f,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
{
    if (f==NULL) return ReadResult::FILE_NOT_HANDLED;

    // MIKEC
    // This appears to build the matrix structures for the 3ds structure
    // It wasn't previously necessary because all the meshes are stored in world coordinates
    // but is VERY necessary if you want to use pivot points...
    lib3ds_file_eval(f,0.0f); // second param is time 't' for animated files

    ReaderObject reader(options);

    reader._directory = ( options && !options->getDatabasePathList().empty() ) ? options->getDatabasePathList().front() : osgDB::getFilePath(fileName);

    ReaderObject::StateSetMap drawStateMap;
    unsigned int numMaterials = f->nmaterials;
    drawStateMap.insert(drawStateMap.begin(), numMaterials, StateSetInfo());        // Setup the map
    for (unsigned int imat=0; imat<numMaterials; ++imat)
    {
        Lib3dsMaterial * mat = f->materials[imat];
        drawStateMap[imat] = reader.createStateSet(mat);
    }

    if (osg::getNotifyLevel()>=osg::INFO)
    {
        int level=0;
        std::cout << "NODE TRAVERSAL of 3ds file "<<f->name<<std::endl;
        for(Lib3dsNode *node=f->nodes; node; node=node->next)
        {
            print(node,level+1);
        }
        std::cout << "MESH TRAVERSAL of 3ds file "<<f->name<<std::endl;
        for (int imesh=0; imesh<f->nmeshes; ++imesh){
            print(f->meshes[imesh],level+1);
        }
    }

    // We can traverse by meshes (old method, broken for pivot points, but otherwise works), or by nodes (new method, not so well tested yet)
    // if your model is broken, especially wrt object positions try setting this flag. If that fixes it,
    // send me the model
    bool traverse_nodes=true;

    // MIKEC: have found 3ds files with NO node structure - only meshes, for this case we fall back to the old traverse-by-meshes code
    // Loading and re-exporting these files from 3DS produces a file with correct node structure, so perhaps these are not 100% conformant?
    if (f->nodes == NULL)
    {
        OSG_WARN<<"Warning: in 3ds loader: file has no nodes, traversing by meshes instead"<< std::endl;
        traverse_nodes=false;
    }

    osg::Node* group = NULL;

    if (!traverse_nodes) // old method, traverse by mesh
    {
        group = new osg::Group();
        for (int imesh=0; imesh<f->nmeshes; ++imesh)
        {
            reader.processMesh(drawStateMap,group->asGroup(),f->meshes[imesh],NULL);
        }
    }
    else
    { // new method
        Lib3dsNode *node=f->nodes;
        if (!node->next)
        {
            group = reader.processNode(drawStateMap,f,node);
        }
        else
        {
            group = new osg::Group();
            for(; node; node=node->next)
            {
                group->asGroup()->addChild(reader.processNode(drawStateMap,f,node));
            }
        }
    }
    if (group && group->getName().empty()) group->setName(fileName);

    if (osg::getNotifyLevel()>=osg::INFO)
    {
        OSG_INFO << "Final OSG node structure looks like this:"<< endl;
        PrintVisitor pv(osg::notify(osg::INFO));
        group->accept(pv);
    }

    return group;
}

struct RemappedFace
{
    Lib3dsFace* face;        // Original face definition.
    osg::Vec3f normal;
    unsigned short index[3]; // Indices to OSG vertex/normal/texcoord arrays.
};

struct VertexParams
{
    VertexParams() : matrix(NULL), smoothNormals(false), scaleUV(1.f, 1.f), offsetUV(0.f, 0.f) { }
    const osg::Matrix* matrix;
    bool smoothNormals;
    osg::Vec2f scaleUV;
    osg::Vec2f offsetUV;
};

static bool isFaceValid(const Lib3dsMesh* mesh, const Lib3dsFace* face)
{
    return
        face->index[0] < mesh->nvertices &&
        face->index[1] < mesh->nvertices &&
        face->index[2] < mesh->nvertices;
}

/* ChrisD: addVertex handles the averaging of normals and spltting of vertices
   required to implement normals for smoothing groups. When a shared
   vertex is encountered when smoothing is required, normals are added
   and normalized. When a shared vertex is encountered when smoothing is
   not required, we must split the vertex if a different normal is required.
   For example if we are processing a cube mesh with no smoothing group
   made from 12 triangles and 8 vertices, the resultant mesh should have
   24 vertices to accomodate the 3 different normals at each vertex.
  */
static void addVertex(
    const Lib3dsMesh* mesh,
    RemappedFace& remappedFace,
    unsigned short int i,
    osg::Geometry* geometry,
    std::vector<int>& origToNewMapping,
    std::vector<int>& splitVertexChain,
    const VertexParams& params)
{
    osg::Vec3Array* vertices = (osg::Vec3Array*)geometry->getVertexArray();
    osg::Vec3Array* normals = (osg::Vec3Array*)geometry->getNormalArray();
    osg::Vec2Array* texCoords = (osg::Vec2Array*)geometry->getTexCoordArray(0);

    unsigned short int index = remappedFace.face->index[i];
    if (origToNewMapping[index] == -1)
    {
        int newIndex = vertices->size();
        remappedFace.index[i] = newIndex;
        origToNewMapping[index] = newIndex;

        // Add the vertex position
        osg::Vec3 vertex = copyLib3dsVec3ToOsgVec3(mesh->vertices[index]);
        if (params.matrix) vertex = vertex * (*params.matrix);
        vertices->push_back(vertex);

        // Add the vertex normal
        normals->push_back(remappedFace.normal);

        // Add the texture coordinate.
        if (texCoords)
        {
            osg::Vec2f texCoord(mesh->texcos[index][0], mesh->texcos[index][1]);
            texCoord = componentMultiply(texCoord, params.scaleUV);
            texCoord += params.offsetUV;
            texCoords->push_back(texCoord);
        }

        // New vertex, so not split yet.
        splitVertexChain.push_back(-1);
    }
    else
    {
        int newIndex = origToNewMapping[index];
        if (params.smoothNormals)
        {
            // Average the normals on the shared vertex.
            remappedFace.index[i] = newIndex;
            osg::Vec3f normal = (*normals)[newIndex];
            normal += remappedFace.normal;
            normal.normalize();
            (*normals)[newIndex] = normal;
        }
        else
        {
            // Find a split vertex chained from newIndex which has the 'same' normal.
            int sharedVertexIndex = newIndex;
            do
            {
                osg::Vec3f normal = (*normals)[sharedVertexIndex];
                float normalDifference = (remappedFace.normal - normal).length2();
                if (normalDifference < 1e-6) break;
                sharedVertexIndex = splitVertexChain[sharedVertexIndex];
            } while (sharedVertexIndex != -1);

            if (sharedVertexIndex == -1)
            {
                // When different normals on a shared vertex required, split the vertex.
                int splitVertexIndex = vertices->size();
                remappedFace.index[i] = splitVertexIndex;
                vertices->push_back((*vertices)[newIndex]);
                normals->push_back(remappedFace.normal);
                if (texCoords)
                {
                    texCoords->push_back((*texCoords)[newIndex]);
                }
                splitVertexChain[newIndex] = splitVertexIndex;
                splitVertexChain.push_back(-1);
            }
            else
            {
                // When normals on shared vertex are identical (or very similar), keep it shared.
                remappedFace.index[i] = sharedVertexIndex;
            }
        }
    }
}

static bool addFace(
    const Lib3dsMesh* mesh,
    RemappedFace& remappedFace,
    osg::Geometry* geometry,
    std::vector<int>& origToNewMapping,
    std::vector<int>& splitVertexChain,
    const VertexParams& params)
{
    if (isFaceValid(mesh, remappedFace.face))
    {
        addVertex(mesh, remappedFace, 0, geometry, origToNewMapping, splitVertexChain, params);
        addVertex(mesh, remappedFace, 1, geometry, origToNewMapping, splitVertexChain, params);
        addVertex(mesh, remappedFace, 2, geometry, origToNewMapping, splitVertexChain, params);
        return true;
    }
    else
    {
        // Avoids crash with corrupted files.
        remappedFace.face = NULL;
        return false;
    }
}

/**
use matrix to pretransform geometry, or NULL to do nothing
*/
osg::Drawable* ReaderWriter3DS::ReaderObject::createDrawable(Lib3dsMesh *m,FaceList& faceList, const osg::Matrix * matrix, StateSetInfo & ssi, bool smoothVertexNormals)
{
    // Avoid creating geoms for empty face list because otherwise osg asserts/crashes during render traversal.
    if (faceList.empty()) return NULL;

    osg::Geometry * geom = new osg::Geometry;

    VertexParams params;
    params.matrix = matrix;
    params.smoothNormals = smoothVertexNormals;

    std::vector<RemappedFace> remappedFaces(faceList.size());

    scoped_array<Lib3dsVector> normals( new Lib3dsVector[m->nfaces] );        // Temporary array
    lib3ds_mesh_calculate_face_normals(m, normals.get());

    osg::ref_ptr<osg::Vec3Array> osg_vertices = new osg::Vec3Array();
    osg_vertices->reserve(m->nvertices);
    geom->setVertexArray(osg_vertices.get());

    osg::ref_ptr<osg::Vec3Array> osg_normals = new osg::Vec3Array();
    osg_normals->reserve(m->nvertices);
    geom->setNormalArray(osg_normals.get(), osg::Array::BIND_PER_VERTEX);

    osg::ref_ptr<osg::Vec2Array> osg_texCoords = NULL;

    if (m->texcos)
    {
        osg_texCoords = new osg::Vec2Array();
        osg_texCoords->reserve(m->nvertices);
        geom->setTexCoordArray(0, osg_texCoords.get());

        // Texture 0 parameters (only one texture supported for now)
        if (ssi.lib3dsmat && *(ssi.lib3dsmat->texture1_map.name))     // valid texture = name not empty
        {
            Lib3dsTextureMap & tex3ds = ssi.lib3dsmat->texture1_map;
            params.scaleUV = osg::Vec2f(tex3ds.scale[0], tex3ds.scale[1]);
            params.offsetUV = osg::Vec2f(tex3ds.offset[0], tex3ds.offset[1]);
            if (tex3ds.rotation != 0) OSG_NOTICE << "3DS texture rotation not supported yet" << std::endl;
            //TODO: tint_1, tint_2, tint_r, tint_g, tint_b
        }
    }

    // The map between lib3ds mesh vertex indices and remapped osg vertices.
    std::vector<int> origToNewMapping(m->nvertices, -1);

    // If osg vertices need to be split to hold a different vertex normal,
    // splitVertexChain allows us to look them up.
    std::vector<int> splitVertexChain;
    splitVertexChain.reserve(m->nvertices);

    unsigned int faceIndex = 0;
    unsigned int faceCount = 0;
    for (FaceList::iterator itr = faceList.begin();
        itr != faceList.end();
        ++itr, ++faceIndex)
    {
        osg::Vec3 normal = copyLib3dsVec3ToOsgVec3(normals[*itr]);
        if (matrix) normal = osg::Matrix::transform3x3(normal, *(params.matrix));
        normal.normalize();

        Lib3dsFace& face = m->faces[*itr];
        remappedFaces[faceIndex].face = &face;
        remappedFaces[faceIndex].normal = normal;
        if (addFace(m, remappedFaces[faceIndex], geom, origToNewMapping, splitVertexChain, params))
        {
            ++faceCount;
        }
    }

    // 'Shrink to fit' all vertex arrays because potentially faceList refers to fewer vertices than the whole mesh.
    // This will almost certainly be the case where mesh has been broken down into smoothing groups.
    if (osg_vertices.valid() && osg_vertices->size() < osg_vertices->capacity()) osg_vertices->trim();
    if (osg_normals.valid() && osg_normals->size() < osg_normals->capacity()) osg_normals->trim();
    if (osg_texCoords.valid() && osg_texCoords->size() < osg_texCoords->capacity()) osg_texCoords->trim();

    // Set geometry color to white.
    osg::ref_ptr<osg::Vec4ubArray> osg_colors = new osg::Vec4ubArray(1);
    (*osg_colors)[0].set(255,255,255,255);
    geom->setColorArray(osg_colors.get(), osg::Array::BIND_OVERALL);

    // Create triangle primitives.
    int numIndices = faceCount * 3;
    osg::ref_ptr<DrawElementsUShort> elements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES, numIndices);
    DrawElementsUShort::iterator index_itr = elements->begin();

    for (unsigned int i = 0; i < remappedFaces.size(); ++i)
    {
        RemappedFace& remappedFace = remappedFaces[i];
        if (remappedFace.face != NULL)
        {
            *(index_itr++) = remappedFace.index[0];
            *(index_itr++) = remappedFace.index[1];
            *(index_itr++) = remappedFace.index[2];
        }
    }

    geom->addPrimitiveSet(elements.get());

#if 0
    osgUtil::TriStripVisitor tsv;
    tsv.stripify(*geom);
#endif

    return geom;
}


osg::Texture2D*  ReaderWriter3DS::ReaderObject::createTexture(Lib3dsTextureMap *texture,const char* label,bool& transparency)
{
    if (texture && *(texture->name))
    {
        OSG_INFO<<"texture->name="<<texture->name<<", _directory="<<_directory<<std::endl;

        // First try already loaded textures.
        TexturesMap::iterator itTex = texturesMap.find(texture->name);
        if (itTex != texturesMap.end()) {
            OSG_DEBUG << "Texture '" << texture->name << "' found in cache." << std::endl;
            return itTex->second.get();
        }

        // Texture not in cache: locate and load.
        std::string fileName = osgDB::findFileInDirectory(texture->name,_directory,osgDB::CASE_INSENSITIVE);
        if (fileName.empty())
        {
            // file not found in .3ds file's directory, so we'll look in the datafile path list.
            fileName = osgDB::findDataFile(texture->name,options, osgDB::CASE_INSENSITIVE);
            OSG_INFO<<"texture->name="<<texture->name<<", _directory="<<_directory<<std::endl;
        }

        if (fileName.empty())
        {
            if (osgDB::containsServerAddress(_directory))
            {
                // if 3DS file is loaded from http, just attempt to load texture from same location.
                fileName = _directory + "/" + texture->name;
            }
            else
            {
                // MIKEC: We can still continue to call osgDB::readRefImageFile in case user has a ReadFileCallback registered
                //        in that case we just use the image's filename as it exists in the 3DS file
                fileName=texture->name;
            }
        }

        if (label) { OSG_DEBUG << label; }
        else { OSG_DEBUG << "texture name"; }

        OSG_DEBUG << " '"<<texture->name<<"'"<< std::endl;
        OSG_DEBUG << "    texture flag        "<<texture->flags<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_DECALE       "<<((texture->flags)&LIB3DS_TEXTURE_DECALE)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_MIRROR       "<<((texture->flags)&LIB3DS_TEXTURE_MIRROR)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_NEGATE       "<<((texture->flags)&LIB3DS_TEXTURE_NEGATE)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_NO_TILE      "<<((texture->flags)&LIB3DS_TEXTURE_NO_TILE)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_SUMMED_AREA  "<<((texture->flags)&LIB3DS_TEXTURE_SUMMED_AREA)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_ALPHA_SOURCE "<<((texture->flags)&LIB3DS_TEXTURE_ALPHA_SOURCE)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_TINT         "<<((texture->flags)&LIB3DS_TEXTURE_TINT)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_IGNORE_ALPHA "<<((texture->flags)&LIB3DS_TEXTURE_IGNORE_ALPHA)<< std::endl;
        OSG_DEBUG << "    LIB3DS_TEXTURE_RGB_TINT     "<<((texture->flags)&LIB3DS_TEXTURE_RGB_TINT)<< std::endl;

        osg::ref_ptr<osg::Image> osg_image = osgDB::readRefImageFile(fileName.c_str(), options); //Absolute Path
        if (!osg_image.valid())
        {
            OSG_NOTICE << "Warning: Cannot create texture "<<texture->name<< std::endl;
            return NULL;
        }
        if (osg_image->getFileName().empty()) // it should be done in OSG with osgDB::readRefImageFile(fileName.c_str());
            osg_image->setFileName(fileName);
        osg::Texture2D* osg_texture = new osg::Texture2D;
        osg_texture->setImage(osg_image.get());
        osg_texture->setName(texture->name);
        // does the texture support transparancy?
        //transparency = ((texture->flags)&LIB3DS_TEXTURE_ALPHA_SOURCE)!=0;

        // what is the wrap mode of the texture.
        osg::Texture2D::WrapMode wm = ((texture->flags)&LIB3DS_TEXTURE_NO_TILE) ?
                osg::Texture2D::CLAMP :
                osg::Texture2D::REPEAT;
        osg_texture->setWrap(osg::Texture2D::WRAP_S,wm);
        osg_texture->setWrap(osg::Texture2D::WRAP_T,wm);
        osg_texture->setWrap(osg::Texture2D::WRAP_R,wm);
                                 // bilinear.
        osg_texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_NEAREST);

        // Insert in cache map
        texturesMap.insert(TexturesMap::value_type(texture->name, osg_texture));
        return osg_texture;
    }
    else
        return NULL;
}


ReaderWriter3DS::StateSetInfo ReaderWriter3DS::ReaderObject::createStateSet(Lib3dsMaterial *mat)
{
    if (mat==NULL) return StateSetInfo();

    bool textureTransparency=false;
    bool transparency = false;
    float alpha = 1.0f - mat->transparency;
    int unit = 0;

    osg::StateSet* stateset = new osg::StateSet;
    osg::Material* material = new osg::Material;

    osg::Vec3 ambient(mat->ambient[0],mat->ambient[1],mat->ambient[2]);
    osg::Vec3 diffuse(mat->diffuse[0],mat->diffuse[1],mat->diffuse[2]);
    osg::Vec3 specular(mat->specular[0],mat->specular[1],mat->specular[2]);
    specular *= mat->shin_strength;
    float shininess = mat->shininess*128.0f;

    // diffuse
    osg::Texture2D* texture1_map = createTexture(&(mat->texture1_map),"texture1_map",textureTransparency);
    if (texture1_map)
    {
        stateset->setTextureAttributeAndModes(unit, texture1_map, osg::StateAttribute::ON);

        double factor = mat->texture1_map.percent;
        if(factor < 1.0)
        {
            osg::TexEnvCombine* texenv = new osg::TexEnvCombine();
            texenv->setCombine_RGB(osg::TexEnvCombine::MODULATE);
            texenv->setSource0_RGB(osg::TexEnvCombine::TEXTURE);
            texenv->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
            texenv->setSource2_RGB(osg::TexEnvCombine::CONSTANT);
            texenv->setConstantColor(osg::Vec4(factor, factor, factor, factor));
            stateset->setTextureAttributeAndModes(unit, texenv, osg::StateAttribute::ON);
        }
        else
        {
            // from an email from Eric Hamil, September 30, 2003.
            // According to the 3DS spec, and other
            // software (like Max, Lightwave, and Deep Exploration) a 3DS material that has
            // a non-white diffuse base color and a 100% opaque bitmap texture, will show the
            // texture with no influence from the base color.

            // so we'll override material back to white.
            // and no longer require the decal hack below...
#if 0
            // Eric original fallback
            osg::Vec4 white(1.0f,1.0f,1.0f,alpha);
            material->setAmbient(osg::Material::FRONT_AND_BACK,white);
            material->setDiffuse(osg::Material::FRONT_AND_BACK,white);
            material->setSpecular(osg::Material::FRONT_AND_BACK,white);
#else
            // try alternative to avoid saturating with white
            // setting white as per OpenGL defaults.
            ambient.set(0.2f,0.2f,0.2f);
            diffuse.set(0.8f,0.8f,0.8f);
            specular.set(0.0f,0.0f,0.0f);
#endif
        }

        unit++;
    }

    // opacity
    osg::Texture2D* opacity_map = createTexture(&(mat->opacity_map),"opacity_map", textureTransparency);
    if (opacity_map)
    {
        if(opacity_map->getImage()->isImageTranslucent())
        {
            transparency = true;

            stateset->setTextureAttributeAndModes(unit, opacity_map, osg::StateAttribute::ON);

            double factor = mat->opacity_map.percent;

                osg::TexEnvCombine* texenv = new osg::TexEnvCombine();
                texenv->setCombine_Alpha(osg::TexEnvCombine::INTERPOLATE);
                texenv->setSource0_Alpha(osg::TexEnvCombine::TEXTURE);
                texenv->setSource1_Alpha(osg::TexEnvCombine::PREVIOUS);
                texenv->setSource2_Alpha(osg::TexEnvCombine::CONSTANT);
                texenv->setConstantColor(osg::Vec4(factor, factor, factor, 1.0 - factor));
                stateset->setTextureAttributeAndModes(unit, texenv, osg::StateAttribute::ON);

            unit++;
        }
        else
        {
            osg::notify(WARN)<<"The plugin does not support images without alpha channel for opacity"<<std::endl;
        }
    }

    // material
    material->setName(mat->name);
    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(ambient, alpha));
    material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(diffuse, alpha));
    material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(specular, alpha));
    material->setShininess(osg::Material::FRONT_AND_BACK, shininess);

    stateset->setAttribute(material);

    if ((alpha < 1.0f) || transparency)
    {
        //stateset->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

    // Set back face culling state if single sided material applied.
    // This seems like a reasonable assumption given that the backface cull option
    // doesn't appear to be encoded directly in the 3DS format, and also because
    // it mirrors the effect of code in 3DS writer which uses the the face culling
    // attribute to determine the state of the 'two_sided' 3DS material being written.
    if (!mat->two_sided)
    {
        stateset->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));
    }

/*
    osg::ref_ptr<osg::Texture> texture1_mask = createTexture(&(mat->texture1_mask),"texture1_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> texture2_map = createTexture(&(mat->texture2_map),"texture2_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> texture2_mask = createTexture(&(mat->texture2_mask),"texture2_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> opacity_map = createTexture(&(mat->opacity_map),"opacity_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> opacity_mask = createTexture(&(mat->opacity_mask),"opacity_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> bump_map = createTexture(&(mat->bump_map),"bump_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> bump_mask = createTexture(&(mat->bump_mask),"bump_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> specular_map = createTexture(&(mat->specular_map),"specular_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> specular_mask = createTexture(&(mat->specular_mask),"specular_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> shininess_map = createTexture(&(mat->shininess_map),"shininess_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> shininess_mask = createTexture(&(mat->shininess_mask),"shininess_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> self_illum_map = createTexture(&(mat->self_illum_map),"self_illum_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> self_illum_mask = createTexture(&(mat->self_illum_mask),"self_illum_mask",textureTransparancy);
    osg::ref_ptr<osg::Texture> reflection_map = createTexture(&(mat->reflection_map),"reflection_map",textureTransparancy);
    osg::ref_ptr<osg::Texture> reflection_mask = createTexture(&(mat->reflection_mask),"reflection_mask",textureTransparancy);
*/
    return StateSetInfo(stateset, mat);
}


osgDB::ReaderWriter::WriteResult ReaderWriter3DS::writeNode(const osg::Node& node,const std::string& fileName,const Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    osgDB::makeDirectoryForFile(fileName.c_str());
    osgDB::ofstream fout(fileName.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!fout.good()) return WriteResult::ERROR_IN_WRITING_FILE;
    return doWriteNode(node, fout, options, fileName);
/*
    bool ok = true;
    Lib3dsFile * file3ds = lib3ds_file_new();
    if (!file3ds) return WriteResult(WriteResult::ERROR_IN_WRITING_FILE);

    osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));
    if (!createFileObject(node, file3ds, fileName, local_opt)) ok = false;
    if (ok && !lib3ds_file_save(file3ds, fileName.c_str())) ok = false;
    lib3ds_file_free(file3ds);

    return ok ? WriteResult(WriteResult::FILE_SAVED) : WriteResult(WriteResult::ERROR_IN_WRITING_FILE);
*/
}


osgDB::ReaderWriter::WriteResult ReaderWriter3DS::writeNode(const osg::Node& node,std::ostream& fout,const Options* options) const
{
    //OSG_WARN << "!!WARNING!! 3DS write support is incomplete" << std::endl;
    std::string optFileName;
    if (options)
    {
        optFileName = options->getPluginStringData("STREAM_FILENAME");
    }

    return doWriteNode(node, fout, options, optFileName);
}

osgDB::ReaderWriter::WriteResult ReaderWriter3DS::doWriteNode(const osg::Node& node,std::ostream& fout, const Options* options, const std::string & fileNamelib3ds) const
{
    osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileNamelib3ds));

    Lib3dsIo io;
    io.self = &fout;
    io.seek_func = fileo_seek_func;
    io.tell_func = fileo_tell_func;
    io.read_func = NULL;
    io.write_func = fileo_write_func;
    io.log_func = fileio_log_func;

    Lib3dsFile * file3ds = lib3ds_file_new();
    if (!file3ds) return WriteResult(WriteResult::ERROR_IN_WRITING_FILE);

    bool ok = true;
    if (!createFileObject(node, file3ds, fileNamelib3ds, local_opt.get())) ok = false;
    if (ok && !lib3ds_file_write(file3ds, &io)) ok = false;
    lib3ds_file_free(file3ds);

    return ok ? WriteResult(WriteResult::FILE_SAVED) : WriteResult(WriteResult::ERROR_IN_WRITING_FILE);
    //return ok ? WriteResult(WriteResult::FILE_SAVED) : WriteResult(WriteResult::FILE_NOT_HANDLED);
}

bool ReaderWriter3DS::createFileObject(const osg::Node& node, Lib3dsFile * file3ds,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
{
    plugin3ds::WriterNodeVisitor w(file3ds, fileName, options, osgDB::getFilePath(node.getName()));
    const_cast<osg::Node &>(node).accept(w);                // Ugly const_cast<> for visitor...
    if (!w.succeeded()) return false;
    w.writeMaterials();
    return w.succeeded();
}

