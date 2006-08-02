#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture3D>
#include <osg/TexGen>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>


//
// A simple demo demonstrating different texturing modes, 
// including using of texture extensions.
//


typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;


class ConstructStateCallback : public osgProducer::OsgCameraGroup::RealizeCallback
{
    public:
        ConstructStateCallback(osg::Node* node):_node(node),_initialized(false) {}
        
        osg::StateSet* constructState()
        {
        
            // read 4 2d images
            osg::ref_ptr<osg::Image> image_0 = osgDB::readImageFile("Images/lz.rgb");
            osg::ref_ptr<osg::Image> image_1 = osgDB::readImageFile("Images/reflect.rgb");
            osg::ref_ptr<osg::Image> image_2 = osgDB::readImageFile("Images/tank.rgb");
            osg::ref_ptr<osg::Image> image_3 = osgDB::readImageFile("Images/skymap.jpg");

            if (!image_0 || !image_1 || !image_2 || !image_3)
            {
                std::cout << "Warning: could not open files."<<std::endl;
                return new osg::StateSet;
            }

            if (image_0->getPixelFormat()!=image_1->getPixelFormat() || image_0->getPixelFormat()!=image_2->getPixelFormat() || image_0->getPixelFormat()!=image_3->getPixelFormat())
            {
                std::cout << "Warning: image pixel formats not compatible."<<std::endl;
                return new osg::StateSet;
            }

            // get max 3D texture size
            GLint textureSize = osg::Texture3D::getExtensions(0,true)->maxTexture3DSize();
            if (textureSize > 256)
                textureSize = 256;

            // scale them all to the same size.
            image_0->scaleImage(textureSize,textureSize,1);
            image_1->scaleImage(textureSize,textureSize,1);
            image_2->scaleImage(textureSize,textureSize,1);
            image_3->scaleImage(textureSize,textureSize,1);


            // then allocated a 3d image to use for texturing.
            osg::Image* image_3d = new osg::Image;
            image_3d->allocateImage(textureSize,textureSize,4,
                                    image_0->getPixelFormat(),image_0->getDataType());

            // copy the 2d images into the 3d image.
            image_3d->copySubImage(0,0,0,image_0.get());
            image_3d->copySubImage(0,0,1,image_1.get());
            image_3d->copySubImage(0,0,2,image_2.get());
            image_3d->copySubImage(0,0,3,image_3.get());

            image_3d->setInternalTextureFormat(image_0->getInternalTextureFormat());        

            // set up the 3d texture itself,
            // note, well set the filtering up so that mip mapping is disabled,
            // gluBuild3DMipsmaps doesn't do a very good job of handled the
            // inbalanced dimensions of the 256x256x4 texture.
            osg::Texture3D* texture3D = new osg::Texture3D;
            texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::LINEAR);
            texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::LINEAR);
            texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::REPEAT);
            texture3D->setImage(image_3d);


            // create a texgen to generate a R texture coordinate, the geometry
            // itself will supply the S & T texture coordinates.
            // in the animateStateSet callback well alter this R value to
            // move the texture through the 3d texture, 3d texture filtering
            // will do the blending for us.
            osg::TexGen* texgen = new osg::TexGen;
            texgen->setMode(osg::TexGen::OBJECT_LINEAR);
            texgen->setPlane(osg::TexGen::R, osg::Vec4(0.0f,0.0f,0.0f,0.2f));

            // create the StateSet to store the texture data
            osg::StateSet* stateset = new osg::StateSet;
            stateset->setTextureMode(0,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
            stateset->setTextureAttribute(0,texgen);
            stateset->setTextureAttributeAndModes(0,texture3D,osg::StateAttribute::ON);

            return stateset;
        }

        virtual void operator()( osgProducer::OsgCameraGroup&, osgProducer::OsgSceneHandler& sh, const Producer::RenderSurface& )
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

                if (!_initialized)
                {

                    // only initialize state once, only need for cases where multiple graphics contexts are
                    // if which case this callback can get called multiple times.
                    _initialized = true;

                    if (_node) _node->setStateSet(constructState());
                }            

            }
            
            // now safe to con
            sh.init();
            
        }
        
        
        OpenThreads::Mutex  _mutex;
        osg::Node*          _node;
        bool                _initialized;
        
};

class UpdateStateCallback : public osg::NodeCallback
{
    public:
        UpdateStateCallback() {}
        
        void animateState(osg::StateSet* stateset)
        {
            // here we simply get any existing texgen, and then increment its
            // plane, pushing the R coordinate through the texture.
            osg::StateAttribute* attribute = stateset->getTextureAttribute(0,osg::StateAttribute::TEXGEN);
            osg::TexGen* texgen = dynamic_cast<osg::TexGen*>(attribute);
            if (texgen)
            {
                texgen->getPlane(osg::TexGen::R)[3] += 0.001f;
            }

        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        { 

            osg::StateSet* stateset = node->getStateSet();
            if (stateset)
            {
                // we have an exisitng stateset, so lets animate it.
                animateState(stateset);
            }

            // note, callback is repsonsible for scenegraph traversal so
            // should always include call the traverse(node,nv) to ensure 
            // that the rest of cullbacks and the scene graph are traversed.
            traverse(node,nv);
        }      
};

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

osg::Node* createModel()
{

    // create the geometry of the model, just a simple 2d quad right now.    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(createSquare());

    // normally we'd create the stateset's to contain all the textures
    // etc here, but, the above technique uses osg::Image::scaleImage and
    // osg::Image::copySubImage() which are implemented with OpenGL utility
    // library, which unfortunately can't be used until we have a valid
    // OpenGL context, and at this point in initilialization we don't have
    // a valid OpenGL context, so we have to delay creation of state until
    // there is a valid OpenGL context.  I'll manage this by using an
    // app callback which will create the state during the first traversal.
    // A bit hacky, and my plan is to reimplement the osg::scaleImage and
    // osg::Image::copySubImage() without using GLU which will get round
    // this current limitation.
    geode->setUpdateCallback(new UpdateStateCallback());
    
    return geode;

}


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of 3D textures.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    // create a model from the images.
    osg::Node* rootNode = createModel();

    if (rootNode) 
    {

        // set the scene to render
        viewer.setSceneData(rootNode);
        
        // the construct state uses gl commands to resize images so we are forced
        // to only call it once a valid graphics context has been established,
        // for that we use a realize callback.
        viewer.setRealizeCallback(new ConstructStateCallback(rootNode));

        // create the windows and run the threads.
        viewer.realize();

        while( !viewer.done() )
        {
            // wait for all cull and draw threads to complete.
            viewer.sync();

            // update the scene by traversing it with the the update visitor which will
            // call all node update callbacks and animations.
            viewer.update();

            // fire off the cull and draw traversals of the scene.
            viewer.frame();

        }
        
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // run a clean up frame to delete all OpenGL objects.
        viewer.cleanup_frame();

        // wait for all the clean up frame to complete.
        viewer.sync();
    }    
    
    return 0;
}
