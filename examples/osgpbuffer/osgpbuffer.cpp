/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>
#include <osg/CoordinateSystemNode>

#include <osg/Texture2D>
#include <osg/MatrixTransform>

#include <osgUtil/SmoothingVisitor>

#include <Producer/CameraConfig>

class PBufferTexture2D : public osg::Texture2D
{
    public:
        PBufferTexture2D( Producer::RenderSurface *pbuffer ):
            _pbuffer(pbuffer) {}

        virtual void apply(osg::State& state) const
        {
            const unsigned int contextID = state.getContextID();

            TextureObject* textureObject = getTextureObject(contextID);
            if( textureObject == 0 )
            {
                GLuint format = 
                    _pbuffer->getRenderToTextureMode() == Producer::RenderSurface::RenderToRGBTexture ? GL_RGB:
                    _pbuffer->getRenderToTextureMode() == Producer::RenderSurface::RenderToRGBATexture ? GL_RGBA : 0 ;
                unsigned int width  = _pbuffer->getWindowWidth();
                unsigned int height = _pbuffer->getWindowHeight();

                _textureObjectBuffer[contextID] = textureObject = 
                    generateTextureObject( contextID, GL_TEXTURE_2D, 1, format, width, height, 1, 0 );

                textureObject->bind();
                applyTexParameters( GL_TEXTURE_2D, state);

                glTexImage2D( GL_TEXTURE_2D, 0, 
                format, width, height, 0, format, GL_UNSIGNED_BYTE, 0 );
                textureObject->setAllocated(true);
            }
            else
            {
                textureObject->bind();
                
                static unsigned int frameNum = 0;
                
                _pbuffer->bindPBufferToTexture( Producer::RenderSurface::FrontBuffer );
                
                ++frameNum;
                
            }
        }

    private:
        Producer::ref_ptr<Producer::RenderSurface> _pbuffer;
};


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

osg::Node* createTexturedFlag(unsigned int width, unsigned int height, Producer::RenderSurface *pbuffer)
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
    osg::Texture2D* texture = new PBufferTexture2D(pbuffer);
    texture->setInternalFormat(GL_RGB);
    texture->setTextureSize(width,height);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP);
    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP);
    stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    polyGeom->setStateSet(stateset);

    polyGeom->setUpdateCallback(new MyGeometryCallback(origin,xAxis,yAxis,zAxis,1.0,1.0/flag_width,0.2f));

    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(polyGeom);

    osg::Group* parent = new osg::Group;
    parent->addChild(geode);

    return parent;
}


osg::ref_ptr<osg::Node> buildSceneGraphAndSetCameraViews(osg::Node *loadedModel, 
        Producer::Camera *pbufferCamera,
        Producer::Camera *mainCamera)
{
    osg::ref_ptr<osg::Group> group = new osg::Group;

    osg::ref_ptr<osg::MatrixTransform> loadedModelTransform = new osg::MatrixTransform;
    loadedModelTransform->addChild(loadedModel);

    osg::ref_ptr<osg::NodeCallback> nc = new osg::AnimationPathCallback(
            loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setUpdateCallback(nc.get());

    loadedModelTransform->setNodeMask( 0x1 );
    group->addChild( loadedModelTransform.get() );
    osg::ref_ptr<osg::Node> texturedFlag = createTexturedFlag( 1024, 512, pbufferCamera->getRenderSurface() );

    texturedFlag->setNodeMask( 0x2 );
    group->addChild( texturedFlag.get());

    osg::BoundingSphere  bs = loadedModel->getBound();
    pbufferCamera->setLensFrustum( -0.5, 0.5, -0.25, 0.25, 1.0, 1000.0 );
    pbufferCamera->setViewByLookat( 
            bs.center()[0], bs.center()[1] - bs.radius() * 3, bs.center()[2],
            bs.center()[0], bs.center()[1], bs.center()[2],
            0, 0, 1 );

    mainCamera->getRenderSurface()->setReadDrawable( pbufferCamera->getRenderSurface());

    return group.get();
}

osg::ref_ptr<osg::Node> buildSceneGraph(osg::Node *loadedModel, Producer::RenderSurface *pbuffer )
{
    osg::ref_ptr<osg::Group> group = new osg::Group;

    osg::ref_ptr<osg::MatrixTransform> loadedModelTransform = new osg::MatrixTransform;
    loadedModelTransform->addChild(loadedModel);

    osg::ref_ptr<osg::NodeCallback> nc = new osg::AnimationPathCallback(
            loadedModelTransform->getBound().center(),osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0f));
    loadedModelTransform->setUpdateCallback(nc.get());

    // Set the loaded model to render only in the PBuffer camera
    loadedModelTransform->setNodeMask( 0x1 );
    group->addChild( loadedModelTransform.get() );
    osg::ref_ptr<osg::Node> texturedFlag = createTexturedFlag( 1024, 512, pbuffer );

    // Set the textured flag to render only in the Main camera
    loadedModelTransform->setNodeMask( 0x1 );
    texturedFlag->setNodeMask( 0x2 );
    group->addChild( texturedFlag.get());

    return group.get();
}

Producer::ref_ptr<Producer::CameraConfig> buildCameraConfig( osg::Node &loadedModel )
{
    // Set up the PBuffer camera
    Producer::ref_ptr<Producer::Camera> pbufferCamera = new  Producer::Camera;
    pbufferCamera->getRenderSurface()->setWindowRectangle( 0, 0, 1024, 512 );
    pbufferCamera->getRenderSurface()->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );
    pbufferCamera->getRenderSurface()->setRenderToTextureMode(Producer::RenderSurface::RenderToRGBATexture);

    // We will manually set the PBuffer camera's Lens and View, 
    // so do not share with the rest of the Camera config.
    pbufferCamera->setShareLens(false);
    pbufferCamera->setShareView(false);

    // Set the Lens and View for the  Pbuffer camera according to the loaded model's size
    osg::BoundingSphere  bs = loadedModel.getBound();
    pbufferCamera->setLensFrustum( -0.5, 0.5, -0.25, 0.25, 1.0, 1000.0 );
    pbufferCamera->setViewByLookat( 
            bs.center()[0], bs.center()[1] - bs.radius() * 3, bs.center()[2],
            bs.center()[0], bs.center()[1], bs.center()[2],
            0, 0, 1 );

    // Set up the main camera
    Producer::ref_ptr<Producer::Camera> mainCamera = new  Producer::Camera;
    mainCamera->getRenderSurface()->setWindowRectangle( 0, 0, 800, 600 );

    // Create the Camera config
    Producer::ref_ptr<Producer::CameraConfig> cfg = new Producer::CameraConfig;

    // Cameras added in "alphabetical" order
    cfg->addCamera("A_PBufferCamera", pbufferCamera.get());
    cfg->addCamera("B_MainCamera",    mainCamera.get());

    // Set up the input area
    Producer::ref_ptr<Producer::InputArea> inputArea = new Producer::InputArea;
    inputArea->addRenderSurface( mainCamera->getRenderSurface());
    cfg->setInputArea( inputArea.get() );


    // Set the main Camera's read drawable to be the pbufferCamera's drawable
    mainCamera->getRenderSurface()->setReadDrawable( pbufferCamera->getRenderSurface());

    return cfg;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example shows how to set up a Producer pbuffer for render to texture effects.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line parameters");
    arguments.getApplicationUsage()->addCommandLineOption("--help-env","Display environment variables available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-keys","Display keyboard & mouse bindings available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-all","Display all command line, env vars and keyboard & mouse bindings");

    /// Load models from command line and build scene graph
    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // Create the camera config.  We use the loaded model to set the Lens and View on the PBuffer camera
    Producer::ref_ptr<Producer::CameraConfig> cameraConfig = buildCameraConfig( *(loadedModel.get()) );

    // We need to build the rest of the scene graph after the camera config has been created because we need 
    // to pass it the PBuffer, which will be the texture for the textured flag
    Producer::RenderSurface *pbuffer = cameraConfig->getCamera(0)->getRenderSurface();
    osg::ref_ptr<osg::Node >root = buildSceneGraph(loadedModel.get(), pbuffer );
    

    // construct the viewer with the camera config.
    osgProducer::Viewer viewer(cameraConfig.get());

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    bool helpAll = arguments.read("--help-all");
    unsigned int helpType = ((helpAll || arguments.read("-h") || arguments.read("--help"))? osg::ApplicationUsage::COMMAND_LINE_OPTION : 0 ) |
                            ((helpAll ||  arguments.read("--help-env"))? osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE : 0 ) |
                            ((helpAll ||  arguments.read("--help-keys"))? osg::ApplicationUsage::KEYBOARD_MOUSE_BINDING : 0 );
    if (helpType)
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

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

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

    osg::Timer_t end_tick = osg::Timer::instance()->tick();

    std::cout << "Time to load = "<<osg::Timer::instance()->delta_s(start_tick,end_tick)<<std::endl;

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(root.get());

    // pass the loaded scene graph to the viewer.
    viewer.setSceneData(root.get());

    // create the windows and run the threads.
    viewer.realize();

    // The loaded model is to be viewed in the pbuffer camera, but 
    // the textured flag is viewed in the main camera.  The nodes have
    // been given node masks, now set the traversal masks on the cameras.
    // Note that this must be done after viewer.realize().
    {
        osgProducer::Viewer::SceneHandlerList shl = viewer.getSceneHandlerList();
        osgProducer::Viewer::SceneHandlerList::iterator p;
        unsigned int n = 0;
        for( p = shl.begin(); p != shl.end(); p++ )
        {
            int inheritanceMask = (*p)->getSceneView()->getInheritanceMask();
            inheritanceMask &= ~(osg::CullSettings::CULL_MASK);
            (*p)->getSceneView()->setInheritanceMask( inheritanceMask );
            (*p)->getSceneView()->setCullMask( 1<<(n));
            n++;
        }
    }

    // printf( "PBuffer window: 0x%x\n", pbuffer->getWindow() );

    viewer.getCamera(0)->setClearColor( 0.1f,0.9f,0.3f,1.0f );

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
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}

