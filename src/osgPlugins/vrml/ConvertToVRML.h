/*
 *
 * OSG to VRML2  converter for OpenSceneGraph.
 *
 * authors :
 *           Johan Nouvel (johan_nouvel@yahoo.com)
 *
 *
 */


#ifndef __convert_to_vrml_to_osg_
#define __convert_to_vrml_to_osg_ 1

#include <fstream>
#include <sstream>

// OSG headers
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/Notify>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/BlendFunc>
#include <osg/Material>
#include <osg/CullFace>
#include <osg/LightModel>
#include <osg/LOD>
#include <osgUtil/TransformCallback>
#include <osg/Object>
#include <osgDB/ReaderWriter>
#include <osg/NodeVisitor>

#include <string>
#include <stack>
#include <osg/Node>
#include <vector>
#include <map>
#include <stack>

using namespace std;

/**
   * This function is called to write an osg graph into a VRML file
   *
   */
osgDB::ReaderWriter::WriteResult convertToVRML(const osg::Node& root, const std::string& filename, const osgDB::ReaderWriter::Options* options=NULL) ;

class ToVRML: public osg::NodeVisitor {
  public:

    typedef enum {
      NO_TEXTURE = 0, KEEP_ORIGINAL_TEXTURE = 1, COPY_TEXTURE = 2, CONVERT_TEXTURE = 3
    } TextureMode;
    ToVRML(const std::string& fileName, const osgDB::ReaderWriter::Options* options);
    virtual ~ToVRML();
    osgDB::ReaderWriter::WriteResult result() {
      return (_res);
    }
    ;

    virtual void apply(osg::Geode& node);
    virtual void apply(osg::Group& node);
    virtual void apply(osg::Billboard& node);
    virtual void apply(osg::MatrixTransform& node);
    virtual void apply(osg::PositionAttitudeTransform& node);
    virtual void apply(osg::Node& node);
    virtual void apply(osg::LOD& node);

    char *indent();
    char *indentM();
    char *indentL();
    void pushStateSetStack(osg::StateSet* ss);
    void popStateSetStack();
    osg::StateSet* getCurrentStateSet();

    //void findObjectName(osg::Object* obj,std::string& name, bool& alreadyLoaded);
    void findMaterialName(osg::Material* mat, std::string& name, bool& alreadyLoaded);
    void findTextureName(osg::Texture2D* tex, std::string& name, bool& alreadyLoaded);

    void apply(osg::Geometry* geom);
    void apply(osg::Drawable* drawable);
    void writeAppearance(osg::StateSet* stateset);
    void writeCoord(osg::Vec3Array* array);
    template<typename T> void writeCoordIndex(GLenum mode, T* indices, unsigned int number, std::vector<int>& primitiveSetFaces, int& primitiveFaces);
    void writeTexCoord(osg::Vec2Array* array, osg::Vec3Array* array2);
    osg::Vec2Array* buildUVArray(osg::TexGen* tGen, osg::Vec3Array* array);
    osg::Vec2Array* buildUVArray(osg::TexEnv* tEnv, osg::Vec3Array* array);
    void writeUVArray(osg::Vec2Array* uvArray, osg::Texture::WrapMode wrap_s, osg::Texture::WrapMode wrap_t);
    void writeNormal(osg::Geometry* geom, std::vector<int>& primitiveSetFaces, int nbFaces);
    void writeColor(osg::Geometry* geom, std::vector<int>& primitiveSetFaces, int nbFaces);

    osgDB::ReaderWriter::WriteResult _res;
    ofstream _fout;
    int _indent;
    char* _strIndent;

    map<osg::ref_ptr<osg::Texture2D>, std::string> _texMap;
    map<osg::ref_ptr<osg::Material>, std::string> _matMap;
    vector<osg::ref_ptr<osg::StateSet> > _stack;
    osg::ref_ptr<osg::Image> _defaultImage;
    std::string _pathToOutput;
    std::string _pathRelativeToOutput;
    TextureMode _textureMode;
    int _txtUnit;

};

#endif
