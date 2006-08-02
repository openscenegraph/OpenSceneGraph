#include <osg/Notify>
#include <osg/Texture1D>
#include <osg/TexGen>
#include <osg/Material>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>


// Creates a stateset which contains a 1D texture which is populated by contour banded color
// this is then used in conjunction with TexGen to create contoured models, either in 
// object linear coords - like contours on a map, or eye linear which contour the distance from
// the eye. An app callback toggles between the two tex gen modes.
osg::StateSet* create1DTextureStateToDecorate(osg::Node* loadedModel)
{
    
    const osg::BoundingSphere& bs = loadedModel->getBound();
    
    osg::Image* image = new osg::Image;

    int noPixels = 1024;
    
    // allocate the image data, noPixels x 1 x 1 with 4 rgba floats - equivilant to a Vec4!
    image->allocateImage(noPixels,1,1,GL_RGBA,GL_FLOAT);
    image->setInternalTextureFormat(GL_RGBA);
    
    typedef std::vector<osg::Vec4> ColorBands;
    ColorBands colorbands;
    colorbands.push_back(osg::Vec4(0.0f,0.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,0.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,1.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(0.0f,1.0,0.0,1.0f));
    colorbands.push_back(osg::Vec4(0.0f,1.0,1.0,1.0f));
    colorbands.push_back(osg::Vec4(0.0f,0.0,1.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,0.0,1.0,1.0f));
    colorbands.push_back(osg::Vec4(1.0f,1.0,1.0,1.0f));

    float nobands = colorbands.size();
    float delta = nobands/(float)noPixels;
    float pos = 0.0f;

    // fill in the image data.    
    osg::Vec4* dataPtr = (osg::Vec4*)image->data();
    for(int i=0;i<noPixels;++i,pos+=delta)
    {
        //float p = floorf(pos);
        //float r = pos-p;
        //osg::Vec4 color = colorbands[(int)p]*(1.0f-r);
        //if (p+1<colorbands.size()) color += colorbands[(int)p+1]*r;
        osg::Vec4 color = colorbands[(int)pos];
        *dataPtr++ = color;
    }
    
    osg::Texture1D* texture = new osg::Texture1D;
    texture->setWrap(osg::Texture1D::WRAP_S,osg::Texture1D::MIRROR);
    texture->setFilter(osg::Texture1D::MIN_FILTER,osg::Texture1D::LINEAR);
    texture->setImage(image);

    float zBase = bs.center().z()-bs.radius();
    float zScale = 2.0f/bs.radius();
    
    osg::TexGen* texgen = new osg::TexGen;
    texgen->setMode(osg::TexGen::OBJECT_LINEAR);
    texgen->setPlane(osg::TexGen::S,osg::Vec4(0.0f,0.0f,zScale,-zBase));
    
    osg::Material* material = new osg::Material;
    
    osg::StateSet* stateset = new osg::StateSet;
    
    stateset->setTextureAttribute(0,texture,osg::StateAttribute::OVERRIDE);
    stateset->setTextureMode(0,GL_TEXTURE_1D,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
    stateset->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE);
    stateset->setTextureMode(0,GL_TEXTURE_3D,osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE);

    stateset->setTextureAttribute(0,texgen,osg::StateAttribute::OVERRIDE);
    stateset->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
    
    stateset->setAttribute(material,osg::StateAttribute::OVERRIDE);
    
    return stateset;
}


// An app callback which alternates the tex gen mode between object linear and eye linear to illustrate what differences it makes.
class AnimateStateCallback : public osg::NodeCallback
{
    public:
        AnimateStateCallback() {}
        
        void animateState(osg::StateSet* stateset,double time)
        {
            // here we simply get any existing texgen, and then increment its
            // plane, pushing the R coordinate through the texture.
            osg::StateAttribute* attribute = stateset->getTextureAttribute(0,osg::StateAttribute::TEXGEN);
            osg::TexGen* texgen = dynamic_cast<osg::TexGen*>(attribute);
            if (texgen)
            {
                const double timeInterval = 1.0f;
                
                static double previousTime = time;
                static bool state = false;
                while (time>previousTime+timeInterval)
                {
                    previousTime+=timeInterval;
                    state = !state;
                }
            
                if (state)
                {
                    texgen->setMode(osg::TexGen::OBJECT_LINEAR);
                }
                else
                {
                    texgen->setMode(osg::TexGen::EYE_LINEAR);
                }
            }
            
        }

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        { 

            osg::StateSet* stateset = node->getStateSet();
            if (stateset && nv->getFrameStamp())
            {
                // we have an exisitng stateset, so lets animate it.
                animateState(stateset,nv->getFrameStamp()->getReferenceTime());
            }

            // note, callback is repsonsible for scenegraph traversal so
            // should always include call the traverse(node,nv) to ensure 
            // that the rest of cullbacks and the scene graph are traversed.
            traverse(node,nv);
        } 
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of 1D textures.");
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
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // load the images specified on command line
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);

    
    if (loadedModel)
    {

        osg::StateSet* stateset = create1DTextureStateToDecorate(loadedModel);
        if (!stateset)
        {
            std::cout<<"Error: failed to create 1D texture state."<<std::endl;
            return 1;
        }


        loadedModel->setStateSet(stateset);
        loadedModel->setUpdateCallback(new AnimateStateCallback());

        // add model to viewer.
        viewer.setSceneData( loadedModel );

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
    else
    {
        osg::notify(osg::NOTICE)<<arguments.getApplicationUsage()->getCommandLineUsage()<<std::endl;
        return 0;
    }
    
    
    
    return 0;
}
