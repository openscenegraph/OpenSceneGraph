#include <osg/Node>
#include <osg/GeoSet>
#include <osg/Notify>
#include <osg/Transform>
#include <osg/Texture>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <GL/glut.h>
#include <osgGLUT/Viewer>

//
// A simple demo demonstrating different texturing modes, 
// including using of texture extensions.
//


typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;

/**
  * Function to read several images files (typically one) as specified 
  * on the command line, and return them in an ImageList
  */
ImageList getImagesFromFiles(int argc,char **argv)
{

    ImageList imageList;

    int i;

    typedef std::vector<osg::Node*> NodeList;
    NodeList nodeList;
    for( i = 1; i < argc; i++ )
    {

        if (argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case('l'):
                    ++i;
                    if (i<argc)
                    {
                        osgDB::Registry::instance()->loadLibrary(argv[i]);
                    }
                    break;
                case('e'):
                    ++i;
                    if (i<argc)
                    {
                        std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(argv[i]);
                        osgDB::Registry::instance()->loadLibrary(libName);
                    }
                    break;
            }
        } else
        {
            osg::Image *image = osgDB::readImageFile( argv[i] );
            if (image)
            {
                imageList.push_back(image);
            }
        }

    }

    if (imageList.size()==0)
    {
        osg::notify(osg::WARN) << "No image data loaded."<<endl;
    }

    return imageList;
}

/** create 2,2 square with center at 0,0,0 and aligned along the XZ plan */
osg::Drawable* createSquare(float textureCoordMax=1.0f)
{
    // set up the geoset.
    osg::GeoSet* gset = new osg::GeoSet;

    osg::Vec3* coords = new osg::Vec3 [4];
    coords[0].set(-1.0f,0.0f,1.0f);
    coords[1].set(-1.0f,0.0f,-1.0f);
    coords[2].set(1.0f,0.0f,-1.0f);
    coords[3].set(1.0f,0.0f,1.0f);
    gset->setCoords(coords);

    osg::Vec3* norms = new osg::Vec3 [1];
    norms[0].set(0.0f,-1.0f,0.0f);
    gset->setNormals(norms);
    gset->setNormalBinding(osg::GeoSet::BIND_OVERALL);

    osg::Vec2* tcoords = new osg::Vec2 [4];
    tcoords[0].set(0.0f,textureCoordMax);
    tcoords[1].set(0.0f,0.0f);
    tcoords[2].set(textureCoordMax,0.0f);
    tcoords[3].set(textureCoordMax,textureCoordMax);
    gset->setTextureCoords(tcoords);
    gset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);
    
    gset->setNumPrims(1);
    gset->setPrimType(osg::GeoSet::QUADS);

    return gset;
}

osg::Node* createTexturedItem(const osg::Vec3& offset,osg::Texture* texture,osg::Node* geometry)
{
    // create a tranform node to position each square in appropriate
    // place and also to add individual texture set to it, so that
    // that state is inherited down to its children.
    osg::Transform* local_transform = new osg::Transform;
    local_transform->postMult(osg::Matrix::translate(offset));

    // create the StateSet to store the texture data
    osg::StateSet* stateset = new osg::StateSet;

    stateset->setAttributeAndModes(texture,osg::StateAttribute::ON);

    // turn the face culling off so you can see the texture from
    // all angles.
    stateset->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);

    // attach the setset to tranform node.
    local_transform->setStateSet(stateset);

    // add the geode to the transform.
    local_transform->addChild(geometry);
    
    return local_transform;
}

osg::Node* createLayer(const osg::Vec3& offset,osg::Image* image,osg::Node* geometry,osg::Node* geometryRep)
{
    if (image==NULL) return NULL;
    
    osg::Transform* top_transform = new osg::Transform;
    top_transform->postMult(osg::Matrix::translate(offset));

    osg::Vec3 local_offset(0.0f,0.0f,0.0f);
    osg::Vec3 local_delta(3.0f,0.0f,0.0f);

    // defaults mipmapped texturing.
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }
        
        
    // bilinear
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);
        
        // set up bilinear filtering.
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_NEAREST);
        texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }

    // trilinear
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);
        
        // set up trilinear filtering.
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }


    // anisotropic
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        // set up anistropic filtering.
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::ANISOTROPIC);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }

    // arb compression
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        texture->setInternalFormatMode(osg::Texture::USE_ARB_COMPRESSION);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }

    // s3tc_dxt1 compression
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        texture->setInternalFormatMode(osg::Texture::USE_S3TC_DXT1_COMPRESSION);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }
    
    // default wrap mode. (osg::Texture::CLAMP)
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometryRep));

        local_offset += local_delta;

    }

    // clamp-to-edge mode.
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometryRep));

        local_offset += local_delta;

    }

    // repeat wrap mode.
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        texture->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
        texture->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometryRep));

        local_offset += local_delta;

    }

    // mirror wrap mode.
    {
        // create the texture attribute
        osg::Texture* texture = new osg::Texture;
        texture->setImage(image);

        texture->setWrap(osg::Texture::WRAP_S,osg::Texture::MIRROR);
        texture->setWrap(osg::Texture::WRAP_T,osg::Texture::MIRROR);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometryRep));

        local_offset += local_delta;

    }

    return top_transform;
}

osg::Node* createModelFromImages(ImageList& imageList)
{

    if (imageList.empty()) return NULL;
    
    // create the root node which will hold the model.
    osg::Group* root = new osg::Group();

    // create a single drawable to be shared by each texture instance.
    osg::Drawable* drawable_noTexCoodRep = createSquare(1.0f);
    
    // add the drawable into a single goede to be shared...
    osg::Geode* geode_noTexCoodRep = new osg::Geode();
    geode_noTexCoodRep->addDrawable(drawable_noTexCoodRep);
    

    // create a single drawable to be shared by each texture instance.
    osg::Drawable* drawable_texCoodRep = createSquare(2.0f);

    // add the drawable into a single goede to be shared...
    osg::Geode* geode_texCoodRep = new osg::Geode();
    geode_texCoodRep->addDrawable(drawable_texCoodRep);
    
    osg::Vec3 offset(0.0f,0.0f,0.0f);
    osg::Vec3 delta(0.0f,0.0f,3.0f);
    
    // step through the image list processing each image in turn.
    for(ImageList::iterator itr=imageList.begin();
        itr!=imageList.end();
        ++itr)
    {
    
        // add the transform node to root group node.
        root->addChild(createLayer(offset,itr->get(),geode_noTexCoodRep,geode_texCoodRep));
        
        offset += delta;
    
    }
    
    return root;
}


void write_usage()
{
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE)<<"usage:"<<endl;
    osg::notify(osg::NOTICE)<<"    osgtexture [options] image_file1 [image_infile2 ...]"<<endl;
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE)<<"options:"<<endl;
    osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<<endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<<endl;
    osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<<endl;
    osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<<endl;
    osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<<endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<<endl;
    osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<<endl;
    osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<<endl;
    osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<<endl;
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE)<<"example:"<<endl;
    osg::notify(osg::NOTICE)<<"    osgtexture lz.rgb"<<endl;
    osg::notify(osg::NOTICE)<<endl;
}

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage();
        return 0;
    }

    // load the images specified on command line
    ImageList imageList = getImagesFromFiles(argc,argv);
    
    
    if (!imageList.empty())
    {

        // create a model from the images.
        osg::Node* rootNode = createModelFromImages(imageList);

        // initialize the viewer.
        osgGLUT::Viewer viewer;
        viewer.addViewport( rootNode );

        // register trackball, flight and drive.
        viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
        viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
        viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

        viewer.open();

        viewer.run();
    }
    else
        write_usage();
    
    
    
    return 0;
}
