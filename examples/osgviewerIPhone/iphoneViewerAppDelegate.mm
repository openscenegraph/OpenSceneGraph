// Created by Thomas Hogarth 2009
// Cleaned up by Stephan Huber 2013
// Added gles3 support 2017 TH
//

// this example will create a fullscreen window showing a box. You can interact with it via
// multi-touch gestures.

#import "iphoneViewerAppDelegate.h"

#include <osg/ShapeDrawable>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgGA/MultiTouchTrackballManipulator>

//include the iphone specific windowing stuff
#include <osgViewer/api/IOS/GraphicsWindowIOS> 

#include "shaders.h"

// global programs
osg::ref_ptr<osg::Program> _vertColorProgram;
osg::ref_ptr<osg::Program> _textureProgram;


@interface MyViewController : UIViewController

- (BOOL)shouldAutorotate;


@end

@implementation MyViewController

- (BOOL)shouldAutorotate
{
    return YES;
}
@end


@implementation iphoneViewerAppDelegate

@synthesize _window;

//
// Shape drawables sometimes use gl_quads so use this function to convert
//
void optimizeNode(osg::Node* node) {
    osgUtil::Optimizer optimizer;
    optimizer.optimize(node, osgUtil::Optimizer::TRISTRIP_GEOMETRY);
}

//
// create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
//
osg::Camera* createHUD(unsigned int w, unsigned int h)
{
    osg::Camera* camera = new osg::Camera;
    
    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,w,0,h));
    
    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    
    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    
    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    
    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);
    
    // add to this camera a subgraph to render
    {
        osg::Geode* geode = new osg::Geode();
        
        osg::Vec3 position(50.0f,h-50,0.0f);
        
        {
            osgText::Text* text = new  osgText::Text();
            text->setUseVertexBufferObjects(true);
            text->setFont(0);//"fonts/arial.ttf");
            text->setPosition(position);
            text->setText("A simple multi-touch-example\n1 touch = rotate, \n2 touches = drag + scale, \n3 touches = home");
            text->setColor(osg::Vec4(0.9,0.1,0.1,1.0));

            geode->addDrawable( text );
        }
        
        camera->addChild(geode);
    }
    
    return camera;
}


class TestMultiTouchEventHandler : public osgGA::GUIEventHandler {
public:
    TestMultiTouchEventHandler(osg::Group* parent_group)
    :   osgGA::GUIEventHandler(),
    _cleanupOnNextFrame(false)
    {
        createTouchRepresentations(parent_group, 10);
    }
    
private:
    void createTouchRepresentations(osg::Group* parent_group, unsigned int num_objects)
    {
        // create some geometry which is shown for every touch-point
        for(unsigned int i = 0; i != num_objects; ++i)
        {
            std::ostringstream ss;
            
            osg::Geode* geode = new osg::Geode();
            
            osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0,0,0), 100));
            drawable->setUseVertexBufferObjects(true);
            drawable->setDataVariance(osg::Object::DYNAMIC);
#if defined(OSG_GLES2_AVAILABLE) || defined(OSG_GLES3_AVAILABLE)
            drawable->getOrCreateStateSet()->setAttributeAndModes(_vertColorProgram, osg::StateAttribute::ON);
            optimizeNode(drawable);
#endif
            drawable->setColor(osg::Vec4(0.5, 0.5, 0.5,1));
            
            geode->addDrawable(drawable);
            
            ss << "Touch " << i;
            
            osgText::Text* text = new  osgText::Text;
            text->setUseVertexBufferObjects(true);
            text->setFont(0); //"fonts/arial.ttf");
            text->setPosition(osg::Vec3(110,0,0));
            text->setDataVariance(osg::Object::DYNAMIC);
            text->setText(ss.str());
            
            geode->addDrawable( text );
            
            _drawables.push_back(drawable);
            _texts.push_back(text);
            
            osg::MatrixTransform* mat = new osg::MatrixTransform();
            mat->addChild(geode);
            mat->setNodeMask(0x0);
            
            _mats.push_back(mat);
            
            parent_group->addChild(mat);
        }
    }
    
    virtual bool handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *)
    {
        switch(ea.getEventType())
        {
            case osgGA::GUIEventAdapter::FRAME:
                if (_cleanupOnNextFrame) {
                    cleanup(0);
                    _cleanupOnNextFrame = false;
                }
                break;
                
            case osgGA::GUIEventAdapter::PUSH:
            case osgGA::GUIEventAdapter::DRAG:
            case osgGA::GUIEventAdapter::RELEASE:
            {
                // is this a multi-touch event?
                if (!ea.isMultiTouchEvent())
                    return false;
                
                unsigned int j(0);
                
                // iterate over all touch-points and update the geometry
                unsigned num_touch_ended(0);
                
                for(osgGA::GUIEventAdapter::TouchData::iterator i = ea.getTouchData()->begin(); i != ea.getTouchData()->end(); ++i, ++j)
                {
                    const osgGA::GUIEventAdapter::TouchData::TouchPoint& tp = (*i);
                    if (ea.getMouseYOrientation() == osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS)
                        _mats[j]->setMatrix(osg::Matrix::translate(tp.x, ea.getWindowHeight() - tp.y, 0));
                    else
                        _mats[j]->setMatrix(osg::Matrix::translate(tp.x, tp.y, 0));
                    
                    _mats[j]->setNodeMask(0xffff);
                    
                    std::ostringstream ss;
                    ss << "Touch " << tp.id;
                    _texts[j]->setText(ss.str());
                    
                    switch (tp.phase)
                    {
                        case osgGA::GUIEventAdapter::TOUCH_BEGAN:
                            _drawables[j]->setColor(osg::Vec4(0,1,0,1));
                            OSG_INFO << "touch began: " << ss.str() << std::endl;
                            break;
                            
                        case osgGA::GUIEventAdapter::TOUCH_MOVED:
                            //OSG_INFO << "touch moved: " << ss.str() << std::endl;
                            _drawables[j]->setColor(osg::Vec4(1,1,1,1));
                            break;
                            
                        case osgGA::GUIEventAdapter::TOUCH_ENDED:
                            _drawables[j]->setColor(osg::Vec4(1,0,0,1));
                            OSG_INFO << "touch ended: " << ss.str() << std::endl;
                            ++num_touch_ended;
                            break;
                            
                        case osgGA::GUIEventAdapter::TOUCH_STATIONERY:
                            _drawables[j]->setColor(osg::Vec4(0.5,0.5,0.5,1));
                            break;
                            
                        default:
                            break;
                            
                    }

                }
                
                // hide unused geometry
                cleanup(j);
                
                //check if all touches ended
                if ((ea.getTouchData()->getNumTouchPoints() > 0) && (ea.getTouchData()->getNumTouchPoints() == num_touch_ended))
                {
                    _cleanupOnNextFrame = true;
                }
                
                // reposition mouse-pointer
                aa.requestWarpPointer((ea.getWindowX() + ea.getWindowWidth()) / 2.0, (ea.getWindowY() + ea.getWindowHeight()) / 2.0);
            }
                break;
                
            default:
                break;
        }
        
        return false;
    }
    
    void cleanup(unsigned int j)
    {
        for(unsigned k = j; k < _mats.size(); ++k) {
            _mats[k]->setNodeMask(0x0);
        }
    }
    
    std::vector<osg::ShapeDrawable*> _drawables;
    std::vector<osg::MatrixTransform*> _mats;
    std::vector<osgText::Text*> _texts;
    bool _cleanupOnNextFrame;
    
};


//
//Called once app has finished launching, create the viewer then realize. Can't call viewer->run as will
//block the final inialization of the windowing system
//
- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
    //get the screen size
    CGRect lFrame = [[UIScreen mainScreen] bounds];
    unsigned int w = lFrame.size.width * [[UIScreen mainScreen] scale];
    unsigned int h = lFrame.size.height  * [[UIScreen mainScreen] scale];
    
    //create the viewer
    _viewer = new osgViewer::Viewer();
    
    
    if(1) {
     
        // If you want full control over the graphics context / window creation, please uncomment this section
         
        // create the main window at screen size
        self._window = [[UIWindow alloc] initWithFrame: lFrame]; 
        
        //show window
        [_window makeKeyAndVisible];
        
        UIView* parent_view = [[UIView alloc] initWithFrame: CGRectMake(0,0, w, h)];
        parent_view.backgroundColor = [UIColor redColor];
        [self._window addSubview: parent_view];
        MyViewController* view_controller = [[MyViewController alloc] init];
        view_controller.view = parent_view;
        self._window.rootViewController = view_controller;
        
        
        //create our graphics context directly so we can pass our own window
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        
        // Init the Windata Variable that holds the handle for the Window to display OSG in.
        osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowIOS::WindowData(parent_view);
        
        // Setup the traits parameters
        traits->x = 0;
        traits->y = 0;
        traits->width = w;
        traits->height = h;
        traits->depth = 16; //can be 16 or 24
        traits->windowDecoration = false;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->setInheritedWindowPixelFormat = true;
        traits->samples = 4;
        traits->sampleBuffers = 1;
        
        traits->inheritedWindowData = windata;

        // Create the Graphics Context
        osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(traits.get());
        
        // if the context was created then attach to our viewer
        if(graphicsContext)
        {
            _viewer->getCamera()->setGraphicsContext(graphicsContext);
            _viewer->getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
            _viewer->getCamera()->setProjectionMatrixAsPerspective(60.0, (double)traits->width/(double)traits->height, 0.1, 1000.0);
            _viewer->getCamera()->setDrawBuffer(GL_BACK);
            _viewer->getCamera()->setReadBuffer(GL_BACK);
        }
    }
    
    // create our default programs
#if defined(OSG_GLES2_AVAILABLE) || defined(OSG_GLES3_AVAILABLE)
    _vertColorProgram = new osg::Program();
    _vertColorProgram->addShader( new osg::Shader(osg::Shader::VERTEX, ColorShaderVert));
    _vertColorProgram->addShader( new osg::Shader(osg::Shader::FRAGMENT, ColorShaderFrag));
    
    _textureProgram = new osg::Program();
    _textureProgram->addShader( new osg::Shader(osg::Shader::VERTEX, TextureShaderVert));
    _textureProgram->addShader( new osg::Shader(osg::Shader::FRAGMENT, TextureShaderFrag));
#endif
    
    //create root
    _root = new osg::MatrixTransform();
    _root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    
    //load and attach scene model
    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile(osgDB::findDataFile("lz.osg"));
    if (!model) {
        osg::Geode* geode = new osg::Geode();
        osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0,0,0), 1));
        drawable->setColor(osg::Vec4(0.1,0.1,0.9,1.0));
        geode->addDrawable(drawable);
        model = geode;
    }
    
    // attach shader program if needed
#if defined(OSG_GLES2_AVAILABLE) || defined(OSG_GLES3_AVAILABLE)
    //model->getOrCreateStateSet()->setAttributeAndModes(_vertColorProgram, osg::StateAttribute::ON);
    model->getOrCreateStateSet()->setAttributeAndModes(_textureProgram, osg::StateAttribute::ON);
    optimizeNode(model);
#endif

    _root->addChild(model);

    // create text hud
    osg::Camera* hud_camera = createHUD(w,h);
    _root->addChild(hud_camera);
    
    
    // attach root to viewer and add event handlers
    _viewer->setSceneData(_root.get());
    _viewer->setCameraManipulator(new osgGA::MultiTouchTrackballManipulator());
    _viewer->addEventHandler(new TestMultiTouchEventHandler(hud_camera));

    // run single-threaded
    _viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    
    osg::setNotifyLevel(osg::INFO);
    
    _viewer->realize();
    
    // render a frame so the window-manager shows some content and not only an empty + black window
    _viewer->frame();
    
    osg::setNotifyLevel(osg::WARN);
    
    // create a display link, which will update our scene on every screen-refresh
    _displayLink = [application.keyWindow.screen displayLinkWithTarget:self selector:@selector(updateScene)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
}


//
//Timer called function to update our scene and render the viewer
//
- (void)updateScene {
    _viewer->frame();
}


- (void)applicationWillResignActive:(UIApplication *)application {
    
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    
}


-(void)applicationWillTerminate:(UIApplication *)application {
    if (_displayLink)
        [_displayLink invalidate];
    _displayLink = NULL;
    _root = NULL;
    _viewer = NULL;
} 




- (void)dealloc {
    _root = NULL;
    _viewer = NULL;
    [super dealloc];
}

@end
