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

#include "ReaderWriterVRML2.h"

#include <iostream>
#include <fstream>

#if defined(_MSC_VER)
#   pragma warning(disable: 4250) 
#   pragma warning(disable: 4290) 
#   pragma warning(disable: 4800) 
#endif 

#include <openvrml/vrml97node.h>
#include <openvrml/common.h>
#include <openvrml/browser.h>
#include <openvrml/node.h>
#include <openvrml/node_ptr.h>
#include <openvrml/field.h>

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


// Register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(vrml, ReaderWriterVRML2)

osgDB::ReaderWriter::ReadResult ReaderWriterVRML2::readNode(const std::string &fname, const Options* opt) const
{
    std::string fileName = osgDB::findDataFile(fname, opt);
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // convert possible Windows backslashes to Unix slashes
    // OpenVRML doesn't like backslashes, even on Windows
    std::string unixFileName = osgDB::convertFileNameToUnixStyle(fileName);

#ifdef WIN32
    if(unixFileName[1] == ':') // absolute path
        fileName = "file:///" + unixFileName;
#else
    if(unixFileName[0] == '/') // absolute path
        fileName = "file://" + unixFileName;
#endif
    else // relative path
        fileName = unixFileName;

  

    std::fstream null;
    openvrml::browser *browser = new openvrml::browser(null, null);

    std::vector<std::string> parameter;
    std::vector<std::string> vuri;
    vuri.push_back(fileName);
    browser->load_url(vuri, parameter);

    std::vector< openvrml::node_ptr > mfn;
    mfn = browser->root_nodes();

    if (mfn.size() == 0) {
        return ReadResult::FILE_NOT_HANDLED;

    } else {
        osg::ref_ptr<osg::MatrixTransform> osg_root = 
            new osg::MatrixTransform(osg::Matrix( 1,  0,  0,  0,
                                                  0,  0,  1,  0,
                                                  0, -1,  0,  0,
                                                  0,  0,  0,  1));
        osgDB::getDataFilePathList().push_front(osgDB::getFilePath(unixFileName));
        for (unsigned i = 0; i < mfn.size(); i++) {
            openvrml::node *vrml_node = mfn[i].get();
            osg_root->addChild(convertFromVRML(vrml_node).get());
        }
        osgDB::getDataFilePathList().pop_front();
        return osg_root.get();
    }
}

osg::ref_ptr<osg::Node> ReaderWriterVRML2::convertFromVRML(openvrml::node *obj) const
{
    std::string name = obj->id();
    static int osgLightNum = 0;  //light

    // std::cout << obj->type.id << " Node " << " ["<< name <<']' << std::endl;

    if (obj->type.id == "Group") // Group node
    {
        openvrml::vrml97_node::group_node *vrml_group;
        vrml_group = dynamic_cast<openvrml::vrml97_node::group_node *>(obj);

        osg::ref_ptr<osg::Group> osg_group = new osg::Group;

        try
        {
            const openvrml::field_value &fv = obj->field("children");

            if ( fv.type() == openvrml::field_value::mfnode_id ) {
                const openvrml::mfnode &mfn = dynamic_cast<const openvrml::mfnode &>(fv);
                for (unsigned i = 0; i < mfn.value.size(); i++) {
                    openvrml::node *node = mfn.value[i].get();
                    osg_group->addChild(convertFromVRML(node).get());
                }
            }
        } 
        catch (openvrml::unsupported_interface&)
        {
            // no children
        }

        return osg_group.get();

    } 
    else if (obj->type.id == "Transform") // Handle transforms
    {
        openvrml::vrml97_node::transform_node *vrml_transform;
        vrml_transform = dynamic_cast<openvrml::vrml97_node::transform_node *>(obj);

        openvrml::mat4f vrml_m = vrml_transform->transform();
        osg::ref_ptr<osg::MatrixTransform> osg_m = new osg::MatrixTransform(osg::Matrix(vrml_m[0][0], vrml_m[0][1], vrml_m[0][2], vrml_m[0][3], vrml_m[1][0], vrml_m[1][1], vrml_m[1][2], vrml_m[1][3], vrml_m[2][0], vrml_m[2][1], vrml_m[2][2], vrml_m[2][3], vrml_m[3][0], vrml_m[3][1], vrml_m[3][2], vrml_m[3][3]));

        try
        {
            const openvrml::field_value &fv = obj->field("children");

            if ( fv.type() == openvrml::field_value::mfnode_id ) {
                const openvrml::mfnode &mfn = dynamic_cast<const openvrml::mfnode &>(fv);
                for (unsigned i = 0; i < mfn.value.size(); i++) {
                    openvrml::node *node = mfn.value[i].get();
                    osg_m->addChild(convertFromVRML(node).get());
                }
            }
        } 
        catch (openvrml::unsupported_interface&)
        {
            // no children
        }

        return osg_m.get();

    } 
    else if (obj->type.id == "Shape") // Handle Shape node
    {
        osg::ref_ptr<osg::Geometry> osg_geom;

        // parse the geometry
        {
            const openvrml::field_value &fv = obj->field("geometry");

            if (fv.type() == openvrml::field_value::sfnode_id)
            {
                const openvrml::sfnode &sfn = dynamic_cast<const openvrml::sfnode &>(fv);
                // is it indexed_face_set_node ?
                
                openvrml::vrml97_node::abstract_geometry_node* vrml_geom = 
                    static_cast<openvrml::vrml97_node::abstract_geometry_node*>(sfn.value.get()->to_geometry());
                
                if (openvrml::vrml97_node::indexed_face_set_node *vrml_ifs = dynamic_cast<openvrml::vrml97_node::indexed_face_set_node *>(vrml_geom))
                {                  
                    osg_geom = convertVRML97IndexedFaceSet(vrml_ifs);
                }
                else if (openvrml::vrml97_node::box_node* vrml_box = dynamic_cast<openvrml::vrml97_node::box_node*>(vrml_geom)) 
                {
                    osg_geom = convertVRML97Box(vrml_box);
                }
                else if (openvrml::vrml97_node::sphere_node* vrml_sphere = dynamic_cast<openvrml::vrml97_node::sphere_node*>(vrml_geom)) 
                {
                    osg_geom = convertVRML97Sphere(vrml_sphere);
                }
                else if (openvrml::vrml97_node::cone_node* vrml_cone = dynamic_cast<openvrml::vrml97_node::cone_node*>(vrml_geom)) 
                {
                    osg_geom = convertVRML97Cone(vrml_cone);
                }
                else if (openvrml::vrml97_node::cylinder_node* vrml_cylinder = dynamic_cast<openvrml::vrml97_node::cylinder_node*>(vrml_geom)) 
                { 
                    osg_geom = convertVRML97Cylinder(vrml_cylinder);
                }
                else 
                {
                    // other geometry types not handled yet
                }
            }
        }

        osg::ref_ptr<osg::Geode> osg_geode = new osg::Geode();
        osg_geode->addDrawable(osg_geom.get());
        osg::StateSet *osg_stateset = osg_geode->getOrCreateStateSet();

        osg::ref_ptr<osg::Material> osg_mat = new osg::Material();
        osg_stateset->setAttributeAndModes(osg_mat.get());
        osg_mat->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);


        // parse the appearance
        {
            const openvrml::field_value &fv = obj->field("appearance");

            if (fv.type() == openvrml::field_value::sfnode_id)
            {
                const openvrml::sfnode &sfn = dynamic_cast<const openvrml::sfnode &>(fv);
                //              std::cerr << "FV->sfnode OK" << std::endl << std::flush;

                openvrml::vrml97_node::appearance_node* vrml_app = static_cast<openvrml::vrml97_node::appearance_node*>(sfn.value.get()->to_appearance());

                const openvrml::node_ptr &vrml_material_node = vrml_app->material();
                const openvrml::node_ptr &vrml_texture_node = vrml_app->texture();

                const openvrml::vrml97_node::material_node *vrml_material =
                    dynamic_cast<const openvrml::vrml97_node::material_node *>(vrml_material_node.get());
                //              std::cerr << "sfnode->Material OK" << std::endl << std::flush;

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

                const openvrml::vrml97_node::image_texture_node *vrml_texture =
                    dynamic_cast<const openvrml::vrml97_node::image_texture_node *>(vrml_texture_node.get());
                //              std::cerr << "TextureNode -> ImageTexture OK" << std::endl << std::flush;

                // if texture is provided
                if (vrml_texture != 0) {
                    const openvrml::field_value &texture_url_fv = vrml_texture->field("url");
                    const openvrml::mfstring &mfs = dynamic_cast<const openvrml::mfstring &>(texture_url_fv);
                    //              std::cerr << "Texture URL FV -> mfstring OK" << std::endl << std::flush;

                    const std::string &url = mfs.value[0];

                    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(url);

                    if (image != 0) {
                        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
                        texture->setImage(image.get());

                        // defaults
                        texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
                        texture->setWrap(osg::Texture::WRAP_R, osg::Texture::REPEAT);
                        texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);

                        // get the real texture wrapping parameters (if any)

                        try {
                            const openvrml::field_value &wrap_fv = vrml_texture->field("repeatS");
                            const openvrml::sfbool &sfb = dynamic_cast<const openvrml::sfbool &>(wrap_fv);

                            if (!sfb.value) {
                                texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
                            }
                        } catch (...) {
                            // nothing specified
                        }

                        try {
                            const openvrml::field_value &wrap_fv = vrml_texture->field("repeatT");
                            const openvrml::sfbool &sfb = dynamic_cast<const openvrml::sfbool &>(wrap_fv);

                            if (!sfb.value) {
                                texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
                            }
                        } catch (...) {
                            // nothing specified
                        }

                        osg_stateset->setTextureAttributeAndModes(0, texture.get());
                        //osg_stateset->setMode(GL_BLEND,osg::StateAttribute::ON);  //bhbn

                    } 
                    else {
                        std::cerr << "texture file " << url << " not found !" << std::endl << std::flush;
                    }
                }
            }
        }

        return osg_geode.get();
    } 
    else 
    {
        return 0;
    }

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
}



