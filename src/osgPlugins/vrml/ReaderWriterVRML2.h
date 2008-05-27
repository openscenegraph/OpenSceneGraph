// -*-c++-*-

/*
 *
 * VRML2 file converter for OpenSceneGraph.
 *
 * authors : Jan Ciger (jan.ciger@gmail.com), 
 *           Tolga Abaci (tolga.abaci@gmail.com), 
 *           Bruno Herbelin (bruno.herbelin@gmail.com)
 *           
 *           (c) VRlab EPFL, Switzerland, 2004-2006
 *
 *           Gino van den Bergen, DTECTA (gino@dtecta.com) 
 * 
 */

#include <string>

#include <osg/Node>
#include <osg/Geometry>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

namespace openvrml
{
    class node;

    namespace vrml97_node
    {
        class indexed_face_set_node;
        class box_node;
        class sphere_node;
        class cone_node;
        class cylinder_node;
    }
}

/**
 * OpenSceneGraph plugin wrapper/converter.
 */
class ReaderWriterVRML2 
    : public osgDB::ReaderWriter
{
public:
    ReaderWriterVRML2() 
    {}

    virtual const char* className()
    {
        return "VRML2 Reader/Writer";
    }

    virtual bool acceptsExtension(const std::string& extension)
    {
        return osgDB::equalCaseInsensitive(extension, "wrl");
    }

    virtual ReadResult readNode(const std::string&, const osgDB::ReaderWriter::Options *options = NULL) const;

private:
    static osg::ref_ptr<osg::Node> convertFromVRML(openvrml::node *obj);

    static osg::ref_ptr<osg::Geometry> convertVRML97IndexedFaceSet(openvrml::vrml97_node::indexed_face_set_node *vrml_ifs);
    static osg::ref_ptr<osg::Geometry> convertVRML97Box(openvrml::vrml97_node::box_node* vrml_box);
    static osg::ref_ptr<osg::Geometry> convertVRML97Sphere(openvrml::vrml97_node::sphere_node* vrml_sphere);
    static osg::ref_ptr<osg::Geometry> convertVRML97Cone(openvrml::vrml97_node::cone_node* vrml_cone);
    static osg::ref_ptr<osg::Geometry> convertVRML97Cylinder(openvrml::vrml97_node::cylinder_node* vrml_cylinder);
};
