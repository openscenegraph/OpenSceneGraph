#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/DrawPixels>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>


//
// A simple demo demonstrating different texturing modes, 
// including using of texture extensions.
//


// include std to get round dumb compilers which can't handle std::hex/dec.
using namespace std;

typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;


class Texture2DCallback : public osg::NodeCallback
{
    public:
        Texture2DCallback(osg::Texture2D* texture):_texture(texture)
        {
            _filterRange.push_back(osg::Texture2D::LINEAR);
            _filterRange.push_back(osg::Texture2D::LINEAR_MIPMAP_LINEAR);
            _filterRange.push_back(osg::Texture2D::LINEAR_MIPMAP_NEAREST);
            _filterRange.push_back(osg::Texture2D::NEAREST);
            _filterRange.push_back(osg::Texture2D::NEAREST_MIPMAP_LINEAR);
            _filterRange.push_back(osg::Texture2D::NEAREST_MIPMAP_NEAREST);
            _currPos = 0;
            _prevTime = 0.0;
        }
        
	virtual ~Texture2DCallback() {}
        
        virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
        {
            if (nv->getFrameStamp())
            {
                double currTime = nv->getFrameStamp()->getReferenceTime();
                if (currTime-_prevTime>1.0) 
                {
                    cout<<"Updating texturing filter to "<<hex<<_filterRange[_currPos]<<dec<<std::endl;
                    _texture->setFilter(osg::Texture2D::MAG_FILTER,_filterRange[_currPos]);
                    _currPos++;
                    if (_currPos>=_filterRange.size()) _currPos=0;                    
                    _prevTime = currTime;
                }
            }
        }
        
        osg::ref_ptr<osg::Texture2D>              _texture;
        std::vector<osg::Texture2D::FilterMode>   _filterRange;
        osg::uint                               _currPos;
        double                                  _prevTime;
};


/**
  * Function to read several images files (typically one) as specified 
  * on the command line, and return them in an ImageList
  */
ImageList getImagesFromFiles(std::vector<std::string>& commandLine)
{

    ImageList imageList;

    for(std::vector<std::string>::iterator itr=commandLine.begin();
        itr!=commandLine.end();
        ++itr)
    {
        if ((*itr)[0]!='-')
        {
            // not an option so assume string is a filename.
            osg::Image *image = osgDB::readImageFile( *itr );
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
    // set up the Geometry.
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0].set(-1.0f,0.0f,1.0f);
    (*coords)[1].set(-1.0f,0.0f,-1.0f);
    (*coords)[2].set(1.0f,0.0f,-1.0f);
    (*coords)[3].set(1.0f,0.0f,1.0f);
    geom->setVertexArray(coords);

    osg::Vec3Array* norms = new osg::Vec3Array(1);
    (*norms)[0].set(0.0f,-1.0f,0.0f);
    geom->setNormalArray(norms);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec2Array* tcoords = new osg::Vec2Array(4);
    (*tcoords)[0].set(0.0f,textureCoordMax);
    (*tcoords)[1].set(0.0f,0.0f);
    (*tcoords)[2].set(textureCoordMax,0.0f);
    (*tcoords)[3].set(textureCoordMax,textureCoordMax);
    geom->setTexCoordArray(0,tcoords);
    
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

    return geom;
}

osg::Node* createTexturedItem(const osg::Vec3& offset,osg::Texture2D* texture,osg::Node* geometry)
{
    // create a tranform node to position each square in appropriate
    // place and also to add individual texture set to it, so that
    // that state is inherited down to its children.
    osg::MatrixTransform* local_transform = new osg::MatrixTransform;
    local_transform->postMult(osg::Matrix::translate(offset));

    // create the StateSet to store the texture data
    osg::StateSet* stateset = new osg::StateSet;

    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

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
    
    osg::MatrixTransform* top_transform = new osg::MatrixTransform;
    top_transform->postMult(osg::Matrix::translate(offset));

    osg::Vec3 local_offset(0.0f,0.0f,0.0f);
    osg::Vec3 local_delta(3.0f,0.0f,0.0f);

//     // use DrawPixels drawable to draw a pixel image.
//     {
//     
//         osg::DrawPixels* drawimage = new osg::DrawPixels;
//         drawimage->setPosition(local_offset);
//         drawimage->setImage(image);
//         
//         osg::Geode* geode = new osg::Geode;
//         geode->addDrawable(drawimage);
//     
//         // add the transform node to root group node.
//         top_transform->addChild(geode);
// 
//         local_offset += local_delta;
//     }


    // defaults mipmapped texturing.
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;
        
        // top_transform->setUpdateCallback(new TextureCallback(texture));

    }
        
        
    // bilinear
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        
        // set up bilinear filtering.
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_NEAREST);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }

    // trilinear
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        
        // set up trilinear filtering.
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }


    // anisotropic
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        // set up anistropic filtering.
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR_MIPMAP_LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        texture->setMaxAnisotropy(2.0f);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }

    // arb compression
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        texture->setInternalFormatMode(osg::Texture2D::USE_ARB_COMPRESSION);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }

    // s3tc_dxt1 compression
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        texture->setInternalFormatMode(osg::Texture2D::USE_S3TC_DXT1_COMPRESSION);
        
        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometry));

        local_offset += local_delta;

    }
    
    // default wrap mode. (osg::Texture2D::CLAMP)
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometryRep));

        local_offset += local_delta;

    }

    // clamp-to-edge mode.
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_EDGE);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometryRep));

        local_offset += local_delta;

    }

    // repeat wrap mode.
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);

        // add the transform node to root group node.
        top_transform->addChild(createTexturedItem(local_offset,texture,geometryRep));

        local_offset += local_delta;

    }

    // mirror wrap mode.
    {
        // create the texture attribute
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::MIRROR);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::MIRROR);

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


void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] image_infile1 [image_infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName      - load plugin of name libraryName"<< std::endl;
    out <<"                          i.e. -l osgdb_pfb"<< std::endl;
    out <<"                          Useful for loading reader/writers which can load"<< std::endl;
    out <<"                          other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName    - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                          i.e. -e pfb"<< std::endl;
    out <<"                          Useful short hand for specifying full library name as"<< std::endl;
    out <<"                          done with -l above, as it automatically expands to"<< std::endl;
    out <<"                          the full library name appropriate for each platform."<< std::endl;
    out <<std::endl;
    out <<"    -stereo             - switch on stereo rendering, using the default of,"<< std::endl;
    out <<"                          ANAGLYPHIC or the value set in the OSG_STEREO_MODE "<< std::endl;
    out <<"                          environmental variable. See doc/stereo.html for "<< std::endl;
    out <<"                          further details on setting up accurate stereo "<< std::endl;
    out <<"                          for your system. "<< std::endl;
    out <<"    -stereo ANAGLYPHIC  - switch on anaglyphic(red/cyan) stereo rendering."<< std::endl;
    out <<"    -stereo QUAD_BUFFER - switch on quad buffered stereo rendering."<< std::endl;
    out <<std::endl;
    out <<"    -stencil            - use a visual with stencil buffer enabled, this "<< std::endl;
    out <<"                          also allows the depth complexity statistics mode"<< std::endl;
    out <<"                          to be used (press 'p' three times to cycle to it)."<< std::endl;
    out << std::endl;
    out<<"example:"<<std::endl;
    out<<"     osgtexture lz.rgb"<<std::endl;
    out<<std::endl;
}

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 0;
    }

    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);


    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the images specified on command line
    ImageList imageList = getImagesFromFiles(commandLine);
    
    
    if (!imageList.empty())
    {

        // create a model from the images.
        osg::Node* rootNode = createModelFromImages(imageList);
        
        imageList.clear();

        // add model to viewer.
        viewer.addViewport( rootNode );

        // register trackball, flight and drive.
        viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
        viewer.registerCameraManipulator(new osgGA::FlightManipulator);
        viewer.registerCameraManipulator(new osgGA::DriveManipulator);

        viewer.open();

        viewer.run();
    }
    else
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 0;
    }
    
    
    
    return 0;
}
