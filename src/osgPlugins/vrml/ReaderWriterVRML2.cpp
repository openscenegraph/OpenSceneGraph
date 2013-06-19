// -*-c++-*-

/*
 *
 * VRML2 file converter for OpenSceneGraph.
 *
 * authors :
 *           Johan Nouvel (johan_nouvel@yahoo.com) for the writeNode function.
 *
 *           Jan Ciger (jan.ciger@gmail.com),
 *           Tolga Abaci (tolga.abaci@gmail.com),
 *           Bruno Herbelin (bruno.herbelin@gmail.com)
 *
 *           (c) VRlab EPFL, Switzerland, 2004-2006
 *
 *           Gino van den Bergen, DTECTA (gino@dtecta.com)
 *           Xiangxian Wang (xiangxianwang@yahoo.com.cn)
 *
 */

#include "ReaderWriterVRML2.h"
#include "ConvertToVRML.h"

#include <iostream>
#include <fstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/utility.hpp>

#if defined(_MSC_VER)
#   pragma warning(disable: 4250)
#   pragma warning(disable: 4290)
#   pragma warning(disable: 4800)
#endif

#include <openvrml/browser.h>
#include <openvrml/node.h>

#include <osg/TexEnv>
#include <osg/CullFace>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Depth>

#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <assert.h>
#include <map>

// -------------------------------------------------------------------------------------
// OpenVRML helper class
// -------------------------------------------------------------------------------------
class resource_fetcher: public openvrml::resource_fetcher
{
    private:
        virtual std::auto_ptr<openvrml::resource_istream> do_get_resource(const std::string & uri)
        {
            using std::auto_ptr;
            using std::invalid_argument;
            using std::string;
            using openvrml::resource_istream;

            class file_resource_istream: public resource_istream
            {
                    std::string url_;
                    std::filebuf buf_;

                public:
                    explicit file_resource_istream(const std::string & path) :
                        resource_istream(&this->buf_)
                    {
                        //
                        // Note that the failbit is set in the constructor if no data
                        // can be read from the stream.  This is important.  If the
                        // failbit is not set on such a stream, OpenVRML will attempt
                        // to read data from a stream that cannot provide it.
                        //
                        if (!this->buf_.open(path.c_str(), ios_base::in | ios_base::binary))
                        {
                            this->setstate(ios_base::badbit);
                        }
                    }

                    void url(const std::string & str) throw (std::bad_alloc)
                    {
                        this->url_ = str;
                    }

                private:
                    virtual const std::string do_url() const throw ()
                    {
                        return this->url_;
                    }

                    virtual const std::string do_type() const throw ()
                    {
                        //
                        // A real application should use OS facilities for this;
                        // however, that is beyond the scope of this example (which
                        // is intended to be portable and stupid).
                        //
                        using std::find;
                        using std::string;
                        using boost::algorithm::iequals;
                        string media_type = "application/octet-stream";

                        const string::const_reverse_iterator dot_pos = find(this->url_.rbegin(), this->url_.rend(), '.');
                        if (dot_pos == this->url_.rend() || boost::next(dot_pos.base()) == this->url_.end())
                        {
                            return media_type;
                        }

                        const string::const_iterator hash_pos = find(boost::next(dot_pos.base()), this->url_.end(), '#');
                        const string ext(dot_pos.base(), hash_pos);

                        if (iequals(ext, "wrl") || iequals(ext, "vrml"))
                        {
                            media_type = "model/vrml";
                        }
                        else if (iequals(ext, "png"))
                        {
                            media_type = "image/png";
                        }
                        else if (iequals(ext, "jpg") || iequals(ext, "jpeg"))
                        {
                            media_type = "image/jpeg";
                        }
                        return media_type;
                    }

                    virtual bool do_data_available() const throw ()
                    {
                        return !!(*this);
                    }
            };

            const string scheme = uri.substr(0, uri.find_first_of(':'));
            if (scheme != "file")
            {
                throw invalid_argument('\"' + scheme + "\" URI scheme not "
                    "supported");
            }
            //
            // file://
            //        ^
            // 01234567
            //
            string path = uri.substr(uri.find_first_of('/', 7));

            auto_ptr<resource_istream> in(new file_resource_istream(path));
            static_cast<file_resource_istream *> (in.get())->url(uri);

            return in;
        }
};

// -------------------------------------------------------------------------------------


// Register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(vrml, ReaderWriterVRML2)

osgDB::ReaderWriter::ReadResult ReaderWriterVRML2::readNode(const std::string& fname, const osgDB::Options* opt) const
{
    std::string fileName = osgDB::findDataFile(fname, opt);
    if (fileName.empty())
        return ReadResult::FILE_NOT_FOUND;

    std::fstream null;
    resource_fetcher fetcher;
    openvrml::browser *b = new openvrml::browser(fetcher, null, null);

    osgDB::ifstream vrml_stream(fileName.c_str());
    if (!vrml_stream.is_open())
    {
        OSG_INFO << "ReaderWriterVRML2: Could not open \"" << fileName << "\"" << std::endl;
        return ReadResult::FILE_NOT_FOUND;
    }

    try
    {
        const std::vector< boost::intrusive_ptr< openvrml::node > > & mfn = b->create_vrml_from_stream(vrml_stream);

        if(mfn.empty())
        {
            OSG_INFO << "ReaderWriterVRML2: OpenVRML library did not return any vrml nodes." << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }
        else
        {
            osg::ref_ptr<osg::MatrixTransform> osg_root = new osg::MatrixTransform(osg::Matrix( 1, 0, 0, 0,
                                                                                                0, 0, 1, 0,
                                                                                                0, -1, 0, 0,
                                                                                                0, 0, 0, 1));

            osgDB::getDataFilePathList().push_front(osgDB::getFilePath(fileName));
            for (unsigned i = 0; i < mfn.size(); i++)
            {
                openvrml::node *vrml_node = mfn[i].get();
                osg_root->addChild(convertFromVRML(vrml_node));
            }
            osgDB::getDataFilePathList().pop_front();
            return osg_root.get();
        }
    }

    catch (const openvrml::invalid_vrml& e)
    {
        OSG_INFO << "ReaderWriterVRML2: Invalid VRML in line " << e.line << " at column " << e.column << ": \"" << e.what() << "\"" << std::endl;
        return ReadResult::FILE_NOT_HANDLED;
    }
    catch (const std::invalid_argument& e)
    {
        OSG_INFO << "ReaderWriterVRML2: Invalid argument: \"" << e.what() << "\"" << std::endl;
        return ReadResult::FILE_NOT_HANDLED;
    }
}

osgDB::ReaderWriter::WriteResult ReaderWriterVRML2::writeNode(const osg::Node& root,const std::string& filename, const osgDB::ReaderWriter::Options *options) const
{
  std::string ext = osgDB::getLowerCaseFileExtension(filename);
  if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;


  osg::notify(osg::INFO) << "osgDB::ReaderWriterVRML::writeNode() Writing file "
                         << filename << std::endl;

  return(convertToVRML(root,filename,options));
}




osg::Node* ReaderWriterVRML2::convertFromVRML(openvrml::node *obj) const
{
    //static int osgLightNum = 0; //light

    if (obj->type().id() == "Group") // Group node

    {
        osg::ref_ptr<osg::Group> osg_group = new osg::Group;

        if (!obj->id().empty())
            osg_group->setName(obj->id());

        try
        {
            std::auto_ptr<openvrml::field_value> fv = obj->field("children");

            if ( fv->type() == openvrml::field_value::mfnode_id )
            {
                const openvrml::mfnode* mfn = dynamic_cast<const openvrml::mfnode *>(fv.get());
                openvrml::mfnode::value_type node_ptr_vector = mfn->value();
                openvrml::mfnode::value_type::iterator it_npv;

                for (it_npv = node_ptr_vector.begin(); it_npv != node_ptr_vector.end(); it_npv++)
                {
                    openvrml::node *node = (*(it_npv)).get();
                    osg_group->addChild(convertFromVRML(node));
                }
            }
        }
        catch (openvrml::unsupported_interface&)
        {
            // no children
        }

        return osg_group.release();

    }

    else if (obj->type().id() == "Transform") // Handle transforms

    {
        openvrml::transform_node *vrml_transform;
        vrml_transform = dynamic_cast<openvrml::transform_node *>(obj);

        openvrml::mat4f vrml_m = vrml_transform->transform();
        osg::ref_ptr<osg::MatrixTransform> osg_m = new osg::MatrixTransform(osg::Matrix(vrml_m[0][0], vrml_m[0][1], vrml_m[0][2], vrml_m[0][3], vrml_m[1][0], vrml_m[1][1], vrml_m[1][2], vrml_m[1][3], vrml_m[2][0], vrml_m[2][1], vrml_m[2][2], vrml_m[2][3], vrml_m[3][0], vrml_m[3][1], vrml_m[3][2], vrml_m[3][3]));

        if (!obj->id().empty())
            osg_m->setName(obj->id());

        try
        {
            std::auto_ptr<openvrml::field_value> fv = obj->field("children");

            if ( fv->type() == openvrml::field_value::mfnode_id )
            {
                const openvrml::mfnode* mfn = dynamic_cast<const openvrml::mfnode *>(fv.get());
                openvrml::mfnode::value_type node_ptr_vector = mfn->value();
                openvrml::mfnode::value_type::iterator it_npv;

                for (it_npv = node_ptr_vector.begin(); it_npv != node_ptr_vector.end(); it_npv++)
                {
                    openvrml::node *node = (*(it_npv)).get();
                    osg_m->addChild(convertFromVRML(node));
                }
            }
        }
        catch (openvrml::unsupported_interface&)
        {
            // no children
        }

        return osg_m.release();

    }

    else if ((obj->type()).id() == "Shape") // Handle Shape node

    {
        osg::ref_ptr<osg::Geometry> osg_geom;

        // parse the geometry
        {
            std::auto_ptr<openvrml::field_value> fv = obj->field("geometry");

            if (fv->type() == openvrml::field_value::sfnode_id)
            {
                const openvrml::sfnode * sfn = dynamic_cast<const openvrml::sfnode *>(fv.get());
                openvrml::sfnode::value_type node_ptr = sfn->value();

                if (node_ptr.get()) {
                    // is it indexed_face_set_node ?
                    if (node_ptr->type().id()=="IndexedFaceSet")
                        osg_geom = convertVRML97IndexedFaceSet(node_ptr.get());

                    else if (node_ptr->type().id()=="IndexedLineSet")
                        osg_geom = convertVRML97IndexedLineSet(node_ptr.get());

                    else if (node_ptr->type().id() == "Box")
                        osg_geom = convertVRML97Box(node_ptr.get());

                    else if (node_ptr->type().id() == "Sphere")
                        osg_geom = convertVRML97Sphere(node_ptr.get());

                    else if (node_ptr->type().id() == "Cone")
                        osg_geom = convertVRML97Cone(node_ptr.get());

                    else if (node_ptr->type().id() == "Cylinder")
                        osg_geom = convertVRML97Cylinder(node_ptr.get());

                    else
                    {
                        // other geometry types not handled yet
                    }

                    if (osg_geom.valid() && !node_ptr->id().empty())
                        osg_geom->setName(node_ptr->id());
                }
            }
        }

        osg::ref_ptr<osg::Geode> osg_geode = new osg::Geode();
        if (!obj->id().empty())
            osg_geode->setName(obj->id());
        osg_geode->addDrawable(osg_geom.get());
        osg::StateSet *osg_stateset = osg_geode->getOrCreateStateSet();

        osg::ref_ptr<osg::Material> osg_mat = new osg::Material();
        osg_stateset->setAttributeAndModes(osg_mat.get());
        osg_mat->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);

        // parse the appearance
        {
            std::auto_ptr<openvrml::field_value> fv = obj->field("appearance");

            if (fv->type() == openvrml::field_value::sfnode_id)
            {
                const openvrml::sfnode *sfn = dynamic_cast<const openvrml::sfnode *>(fv.get());
                openvrml::appearance_node *vrml_app = dynamic_cast<openvrml::appearance_node *>(sfn->value().get());

                const boost::intrusive_ptr<openvrml::node> vrml_material_node = vrml_app->material();
                const boost::intrusive_ptr<openvrml::texture_node> vrml_texture_node = openvrml::node_cast<openvrml::texture_node*>(vrml_app->texture().get());
                const openvrml::material_node *vrml_material = dynamic_cast<const openvrml::material_node *>(vrml_material_node.get());

                if (vrml_material != NULL)
                {
                    osg_mat->setAmbient(osg::Material::FRONT_AND_BACK,
                                    osg::Vec4(vrml_material->ambient_intensity(),
                                                    vrml_material->ambient_intensity(),
                                                    vrml_material->ambient_intensity(),
                                                    1.0));
                    osg_mat->setDiffuse(osg::Material::FRONT_AND_BACK,
                                    osg::Vec4(vrml_material->diffuse_color().r(),
                                                    vrml_material->diffuse_color().g(),
                                                    vrml_material->diffuse_color().b(),
                                                    1.0));
                    osg_mat->setEmission(osg::Material::FRONT_AND_BACK,
                                    osg::Vec4(vrml_material->emissive_color().r(),
                                                    vrml_material->emissive_color().g(),
                                                    vrml_material->emissive_color().b(),
                                                    1.0));
                    osg_mat->setSpecular(osg::Material::FRONT_AND_BACK,
                                    osg::Vec4(vrml_material->specular_color().r(),
                                                    vrml_material->specular_color().g(),
                                                    vrml_material->specular_color().b(),
                                                    1.0));

                    osg_mat->setShininess(osg::Material::FRONT_AND_BACK, vrml_material->shininess() );

                    if (vrml_material->transparency() > 0.0f)
                    {
                        osg_mat->setTransparency(osg::Material::FRONT_AND_BACK, vrml_material->transparency());
                        osg_stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
                        osg_stateset->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false)); // GvdB: transparent objects do not write depth
                        osg_stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                    }
                    else
                    {
                        osg_stateset->setMode(GL_BLEND, osg::StateAttribute::OFF);
                        osg_stateset->setRenderingHint(osg::StateSet::OPAQUE_BIN);
                    }

                    osg_stateset->setAttributeAndModes(osg_mat.get());

                }

                // if texture is provided
                if (vrml_texture_node != 0)
                {
                    osg::ref_ptr<osg::Image> image;

                    if (vrml_texture_node->type().id() == "ImageTexture")
                    {
                        try
                        {
                            std::auto_ptr<openvrml::field_value> texture_url_fv = vrml_texture_node->field("url");
                            const openvrml::mfstring *mfs = dynamic_cast<const openvrml::mfstring *>(texture_url_fv.get());
                            const std::string &url = mfs->value()[0];

                            image = osgDB::readRefImageFile(url);

                            if (!image.valid())
                            {
                                std::cerr << "texture file " << url << " not found !" << std::endl << std::flush;
                            }
                        }
                        catch (openvrml::unsupported_interface&)
                        {
                            // no url field in the texture
                        }
                    }

                    if (!image.valid())
                    {
                        // If we cannot read the image try the openvrml builtin mechanisms to read the image.
                        // This includes PixelTexture fields.
                        const openvrml::image& vrml_image = vrml_texture_node->image();

                        // Convert to an osg image
                        image = new osg::Image;
                        image->allocateImage(vrml_image.x(), vrml_image.y(), 1, GL_RGBA, GL_UNSIGNED_BYTE);
                        for (std::size_t y = 0; y < vrml_image.y(); ++y)
                        {
                            for (std::size_t x = 0; x < vrml_image.x(); ++x)
                            {
                                openvrml::int32 p = vrml_image.pixel(x, y);
                                unsigned char* data = image->data(x, y);
                                data[0] = 0xff & (p >> 24);
                                data[1] = 0xff & (p >> 16);
                                data[2] = 0xff & (p >> 8);
                                data[3] = 0xff & (p >> 0);
                            }
                        }
                    }

                    if (image.valid())
                    {
                        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
                        texture->setImage(image.get());

                        // get the real texture wrapping parameters
                        if (vrml_texture_node->repeat_s())
                        {
                            texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
                        }
                        else
                        {
                            texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
                        }

                        if (vrml_texture_node->repeat_t())
                        {
                            texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
                        }
                        else
                        {
                            texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
                        }
                        texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);

                        osg_stateset->setTextureAttributeAndModes(0, texture.get());

                        if (image->isImageTranslucent()) {
                            osg_stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
                            osg_stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
                        }
                    }
                }
            }
        }

        return osg_geode.release();
    }
    else
    {
        return 0;
    }
#if 0
    /*
     } else if(obj->type.id == "DirectionalLight")    // Handle lights
     {
     osg::Group* lightGroup = new osg::Group;

     openvrml::vrml97_node::directional_light_node *vrml_light;
     vrml_light = dynamic_cast<openvrml::vrml97_node::directional_light_node *>(obj);

     // create light with global params
     osg::Light* myLight = new osg::Light;
     myLight->setLightNum(osgLightNum);
     myLight->setAmbient(osg::Vec4(vrml_light->ambient_intensity(),vrml_light->ambient_intensity(),vrml_light->ambient_intensity(),vrml_light->ambient_intensity()));
     float osgR = vrml_light->color().r()*vrml_light->intensity();
     float osgG = vrml_light->color().g()*vrml_light->intensity();
     float osgB = vrml_light->color().b()*vrml_light->intensity();
     myLight->setDiffuse(osg::Vec4(osgR, osgG, osgB, 1.0f));
     myLight->setSpecular(osg::Vec4(osgR, osgG, osgB, 1.0f));

     // configure light as DIRECTIONAL
     openvrml::sfvec3f &dir = vrml_light->direction_;
     myLight->setDirection(osg::Vec3(dir.value[0],dir.value[1],dir.value[2]));
     myLight->setPosition(osg::Vec4(dir.value[0],dir.value[1],dir.value[2], 0.0f));

     // add the light in the scenegraph
     osg::LightSource* lightS = new osg::LightSource;
     lightS->setLight(myLight);
     if (vrml_light->on()) {
     lightS->setLocalStateSetModes(osg::StateAttribute::ON);
     //lightS->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
     }

     lightGroup->addChild(lightS);
     osgLightNum++;

     return lightGroup;

     } else if(obj->type.id == "PointLight")    // Handle lights
     {
     osg::Group* lightGroup = new osg::Group;

     openvrml::vrml97_node::point_light_node *vrml_light;
     vrml_light = dynamic_cast<openvrml::vrml97_node::point_light_node *>(obj);

     // create light with global params
     osg::Light* myLight = new osg::Light;
     myLight->setLightNum(osgLightNum);
     //std::cout<<"lightnum = "<<osgLightNum;

     openvrml::sfvec3f &pos = vrml_light->location_;
     myLight->setPosition(osg::Vec4(pos.value[0], pos.value[1], pos.value[2], 1.0f));

     myLight->setAmbient(osg::Vec4(vrml_light->ambient_intensity(),vrml_light->ambient_intensity(),vrml_light->ambient_intensity(),vrml_light->ambient_intensity()));
     float osgR = vrml_light->color().r()*vrml_light->intensity();
     float osgG = vrml_light->color().g()*vrml_light->intensity();
     float osgB = vrml_light->color().b()*vrml_light->intensity();
     myLight->setDiffuse(osg::Vec4(osgR, osgG, osgB, 1.0f));
     myLight->setSpecular(osg::Vec4(osgR, osgG, osgB, 1.0f));

     // configure light as POINT
     myLight->setDirection(osg::Vec3(0.f,0.f,0.f));

     // add the light in the scenegraph
     osg::LightSource* lightS = new osg::LightSource;
     lightS->setLight(myLight);
     if (vrml_light->on()) {
     lightS->setLocalStateSetModes(osg::StateAttribute::ON);
     //lightS->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
     }

     lightGroup->addChild(lightS);
     osgLightNum++;

     return lightGroup;

     } else if(obj->type.id == "SpotLight")    // Handle lights
     {
     osg::Group* lightGroup = new osg::Group;

     openvrml::vrml97_node::spot_light_node *vrml_light;
     vrml_light = dynamic_cast<openvrml::vrml97_node::spot_light_node *>(obj);

     // create light with global params
     osg::Light* myLight = new osg::Light;
     myLight->setLightNum(osgLightNum);
     myLight->setPosition(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
     myLight->setAmbient(osg::Vec4(vrml_light->ambient_intensity(),vrml_light->ambient_intensity(),vrml_light->ambient_intensity(),vrml_light->ambient_intensity()));
     float osgR = vrml_light->color().r()*vrml_light->intensity();
     float osgG = vrml_light->color().g()*vrml_light->intensity();
     float osgB = vrml_light->color().b()*vrml_light->intensity();
     myLight->setDiffuse(osg::Vec4(osgR, osgG, osgB, 1.0f));
     myLight->setSpecular(osg::Vec4(osgR, osgG, osgB, 1.0f));

     // configure light as SPOT
     openvrml::sfvec3f &dir = vrml_light->direction_;
     myLight->setDirection(osg::Vec3(dir.value[0],dir.value[1],dir.value[2]));

     // The cutOff value in osg ranges from 0 to 90, we need
     // to divide by 2 to avoid openGL error.
     //      myLight->setSpotCutoff(ls.fallsize/2.0f);
     // The bigger the differens is between fallsize and hotsize
     // the bigger the exponent should be.
     //      float diff = ls.fallsize - ls.hotsize;
     //      myLight->setSpotExponent(diff);

     // add the light in the scenegraph
     osg::LightSource* lightS = new osg::LightSource;
     lightS->setLight(myLight);
     if (vrml_light->on()) {
     lightS->setLocalStateSetModes(osg::StateAttribute::ON);
     //lightS->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
     }

     lightGroup->addChild(lightS);
     osgLightNum++;

     return lightGroup;

     }  else {

     return NULL;
     }
     */

#endif

    return 0;
}

