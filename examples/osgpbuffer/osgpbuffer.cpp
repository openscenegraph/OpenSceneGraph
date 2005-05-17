#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/Billboard>
#include <osg/Material>

#include <osgUtil/TransformCallback>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/UpdateVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>
#include <Producer/Camera>
#include <Producer/CameraConfig>

osg::ref_ptr<Producer::Camera> gPBufferCamera;
osg::ref_ptr<osgProducer::OsgSceneHandler> gPBufferSceneHandler;

// call back which cretes a deformation field to oscilate the model.
class MyGeometryCallback : 
    public osg::Drawable::UpdateCallback, 
    public osg::Drawable::AttributeFunctor
{
    public:
    
        MyGeometryCallback(const osg::Vec3& o,
                           const osg::Vec3& x,const osg::Vec3& y,const osg::Vec3& z,
                           double period,double xphase,double amplitude):
            _firstCall(true),
            _startTime(0.0),
            _time(0.0),
            _period(period),
            _xphase(xphase),
            _amplitude(amplitude),
            _origin(o),
            _xAxis(x),
            _yAxis(y),
            _zAxis(z) {}
    
        virtual void update(osg::NodeVisitor* nv,osg::Drawable* drawable)
        {
            const osg::FrameStamp* fs = nv->getFrameStamp();
            double referenceTime = fs->getReferenceTime();
            if (_firstCall)
            {
                _firstCall = false;
                _startTime = referenceTime;
            }
            
            _time = referenceTime-_startTime;
            
            drawable->accept(*this);
            drawable->dirtyBound();
            
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(drawable);
            if (geometry)
            {
                osgUtil::SmoothingVisitor::smooth(*geometry);
            }
            
        }
        
        virtual void apply(osg::Drawable::AttributeType type,unsigned int count,osg::Vec3* begin) 
        {
            if (type == osg::Drawable::VERTICES)
            {
                const float TwoPI=2.0f*osg::PI;
                const float phase = -_time/_period;
                
                osg::Vec3* end = begin+count;
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    osg::Vec3 dv(*itr-_origin);
                    osg::Vec3 local(dv*_xAxis,dv*_yAxis,dv*_zAxis);
                    
                    local.z() = local.x()*_amplitude*
                                sinf(TwoPI*(phase+local.x()*_xphase)); 
                    
                    (*itr) = _origin + 
                             _xAxis*local.x()+
                             _yAxis*local.y()+
                             _zAxis*local.z();
                }
            }
        }

        bool    _firstCall;

        double  _startTime;
        double  _time;
        
        double  _period;
        double  _xphase;
        float   _amplitude;

        osg::Vec3   _origin;
        osg::Vec3   _xAxis;
        osg::Vec3   _yAxis;
        osg::Vec3   _zAxis;
        
};


// Custom Texture subload callback, just acts the the standard subload modes in osg::Texture right now
// but code be used to define your own style callbacks.
class MyTextureSubloadCallback : public osg::Texture2D::SubloadCallback
{
    public:
    
        MyTextureSubloadCallback() {}
        
        virtual ~MyTextureSubloadCallback() {}

        virtual void load(const osg::Texture2D& texture,osg::State&) const
        {
            osg::notify(osg::INFO)<<"doing load"<<std::endl;
            glTexImage2D( GL_TEXTURE_2D, 0, texture.getInternalFormat(), texture.getTextureWidth(), texture.getTextureHeight(), 0, GL_RGB, GL_FLOAT, 0 );
        }
        
        virtual void subload(const osg::Texture2D& ,osg::State&) const
        {
            osg::notify(osg::INFO)<<"doing subload"<<std::endl;
            gPBufferCamera->getRenderSurface()->bindPBufferToTexture(Producer::RenderSurface::FrontBuffer);
        }
};



osg::Node* createTexturedFlag(unsigned int width, unsigned int height)
{
    // create the quad to visualize.
    osg::Geometry* polyGeom = new osg::Geometry();

    polyGeom->setSupportsDisplayList(false);

    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 xAxis(1.0f,0.0f,0.0f);
    osg::Vec3 yAxis(0.0f,0.0f,1.0f);
    osg::Vec3 zAxis(0.0f,-1.0f,0.0f);
    float flag_height = 100.0f;
    float flag_width = 200.0f;
    int noSteps = 20;
    
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec3 bottom = origin;
    osg::Vec3 top = origin; top.z()+= flag_height;
    osg::Vec3 dv = xAxis*(flag_width/((float)(noSteps-1)));
    
    osg::Vec2Array* texcoords = new osg::Vec2Array;
    osg::Vec2 bottom_texcoord(0.0f,0.0f);
    osg::Vec2 top_texcoord(0.0f,1.0f);
    osg::Vec2 dv_texcoord(1.0f/(float)(noSteps-1),0.0f);

    for(int i=0;i<noSteps;++i)
    {
        vertices->push_back(top);
        vertices->push_back(bottom);
        top+=dv;
        bottom+=dv;

        texcoords->push_back(top_texcoord);
        texcoords->push_back(bottom_texcoord);
        top_texcoord+=dv_texcoord;
        bottom_texcoord+=dv_texcoord;
    }
    

    // pass the created vertex array to the points geometry object.
    polyGeom->setVertexArray(vertices);
    
    polyGeom->setTexCoordArray(0,texcoords);

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    polyGeom->setColorArray(colors);
    polyGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP,0,vertices->size()));

    // new we need to add the texture to the Drawable, we do so by creating a 
    // StateSet to contain the Texture StateAttribute.
    osg::StateSet* stateset = new osg::StateSet;

    // Dynamic texture filled with data from pbuffer.
    osg::Texture2D* texture = new osg::Texture2D;
    //texture->setSubloadMode(osg::Texture::IF_DIRTY);
    texture->setInternalFormat(GL_RGB);
    texture->setTextureSize(width,height);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP);
    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP);
    texture->setSubloadCallback(new MyTextureSubloadCallback());
    stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    polyGeom->setStateSet(stateset);

    polyGeom->setUpdateCallback(new MyGeometryCallback(origin,xAxis,yAxis,zAxis,1.0,1.0/flag_width,0.2f));

    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(polyGeom);

    osg::Group* parent = new osg::Group;
    parent->addChild(geode);

    return parent;
}

void InitPBufferCamera(osg::Node* subgraph, unsigned int width, unsigned int height)
{
    if (!subgraph) return;

    gPBufferCamera = new Producer::Camera;
    gPBufferCamera->getRenderSurface()->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );
    gPBufferCamera->getRenderSurface()->setWindowRectangle( 0, 0, width, height );
    gPBufferCamera->getRenderSurface()->setRenderToTextureMode(Producer::RenderSurface::RenderToRGBTexture);

    gPBufferSceneHandler = new osgProducer::OsgSceneHandler;
    gPBufferSceneHandler->getSceneView()->setDefaults();
    gPBufferSceneHandler->getSceneView()->setSceneData(subgraph);

    gPBufferCamera->setSceneHandler( gPBufferSceneHandler.get());

    gPBufferCamera->setClearColor( 0.1f,0.9f,0.3f,1.0f );

    const osg::BoundingSphere& bs = subgraph->getBound();
    if (!bs.valid())
    {
        osg::notify(osg::WARN) << "bb invalid"<<subgraph<<std::endl;
        return;
    }

    float znear = 1.0f*bs.radius();
    float zfar  = 3.0f*bs.radius();
        
    // 2:1 aspect ratio as per flag geomtry below.
    float top   = 0.25f*znear;
    float right = 0.5f*znear;

    znear *= 0.9f;
    zfar *= 1.1f;

    osg::Vec3 eye(bs.center() + osg::Vec3(0.0f, 2.0f, 0.0f)*bs.radius());

    gPBufferCamera->setViewByLookat(
        eye.x(), eye.y(), eye.z(),
        bs.center().x(), bs.center().y(), bs.center().z(),
        0.0f, 0.0f, 1.0f);


    gPBufferCamera->setLensFrustum(-right,right,-top,top,znear,zfar);

    // The Producer PBuffer example says:
    //
    //      "This line is not necessary on glX pbuffer examples, but Windows
    //       seems to  not like rendering to the back buffer on pbuffers"
    //
    // but I've found that doing this causes a flickering texture.  Everything
    // works fine for me (on Windows) if I just comment it out.

    //gPBufferSceneHandler->getSceneView()->setDrawBufferValue( GL_FRONT );
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osg::DisplaySettings::instance()->readCommandLine(arguments);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use pbuffers and render to texture..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--width <integer>","Set the width of the pbuffer & texture");
    arguments.getApplicationUsage()->addCommandLineOption("--height <integer>","Set the height of the pbuffer & texture");
    
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());
    
    unsigned int width=1024;
    unsigned int height=512;

    while (arguments.read("--width",width)) {}
    while (arguments.read("--height",height)) {}

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

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    

    if (!loadedModel)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }

    // create a transform to spin the model.
    osg::MatrixTransform* loadedModelTransform = new osg::MatrixTransform;
    loadedModelTransform->addChild(loadedModel.get());

    osg::NodeCallback* nc = new osgUtil::TransformCallback(loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setUpdateCallback(nc);

    osg::Group* rootNode = new osg::Group();

    // Create a waving rectangle that will use the pbuffer as a texture.
    rootNode->addChild(createTexturedFlag(width,height));

    // Set up a camera that will render a view of the model to the pbuffer.
    InitPBufferCamera(loadedModelTransform,width, height);

    // set the scene to render
    viewer.setSceneData(rootNode);


    // create the windows and run the threads.
    viewer.realize();

#if defined(GLX_VERSION_1_1)
    // This determins where Pixel reads occur from.  The main Camera will
    // set the pBuffer camera's rendersurface as the buffer to read from.
    Producer::Camera* camera = viewer.getCamera(0);
    camera->getRenderSurface()->setReadDrawable( gPBufferCamera->getRenderSurface());
#endif

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        loadedModelTransform->accept(*viewer.getUpdateVisitor());
        viewer.update();

        // fire off the cull and draw traversals of the scene.
        // first the pbuffer draw
        gPBufferCamera->frame();

        // then the main view draw.
        viewer.frame();
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    gPBufferCamera = 0;
    gPBufferSceneHandler = 0;

    return 0;
}
