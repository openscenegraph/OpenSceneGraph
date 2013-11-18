/* OpenSceneGraph example, osghud.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/PolygonOffset>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/RenderInfo>

#include <osgDB/WriteFile>

#include <osgText/Text>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osg/ShapeDrawable>

#ifdef __APPLE__
#include <osgViewer/api/Cocoa/GraphicsWindowCocoa>
#endif


osg::Camera* createHUD(unsigned int w, unsigned int h)
{
    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
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

        std::string timesFont("fonts/arial.ttf");

        // turn lighting off for the text and disable depth test to ensure it's always ontop.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::Vec3 position(50.0f,h-50,0.0f);

        {
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );

            text->setFont(timesFont);
            text->setPosition(position);
            text->setText("A simple multi-touch-example\n1 touch = rotate, \n2 touches = drag + scale, \n3 touches = home");
        }    

        camera->addChild(geode);
    }

    return camera;
}


class TestMultiTouchEventHandler : public osgGA::GUIEventHandler {
public:
    TestMultiTouchEventHandler(osg::Group* parent_group, float w, float h)
    :   osgGA::GUIEventHandler(),
        _cleanupOnNextFrame(false),
        _w(w),
        _h(h)
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
            drawable->setColor(osg::Vec4(0.5, 0.5, 0.5,1));
            geode->addDrawable(drawable);
            
            ss << "Touch " << i;
            
            osgText::Text* text = new  osgText::Text;
            geode->addDrawable( text );
            drawable->setDataVariance(osg::Object::DYNAMIC);
            _drawables.push_back(drawable);
            
            
            text->setFont("fonts/arial.ttf");
            text->setPosition(osg::Vec3(110,0,0));
            text->setText(ss.str());
            _texts.push_back(text);
            text->setDataVariance(osg::Object::DYNAMIC);
            
            
            
            osg::MatrixTransform* mat = new osg::MatrixTransform();
            mat->addChild(geode);
            mat->setNodeMask(0x0);
            
            _mats.push_back(mat);
            
            parent_group->addChild(mat);
        } 
        
        parent_group->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    }
    
    virtual bool handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *)
    {
        if (ea.getEventType() != osgGA::GUIEventAdapter::FRAME) {
            std::cout << ea.getTime() << ": ";
            switch(ea.getEventType()) {
                case osgGA::GUIEventAdapter::PUSH: std::cout << "PUSH"; break;
                case osgGA::GUIEventAdapter::DRAG:  std::cout << "DRAG"; break;
                case osgGA::GUIEventAdapter::MOVE:  std::cout << "MOVE"; break;
                case osgGA::GUIEventAdapter::RELEASE:  std::cout << "RELEASE"; break;
                default: std::cout << ea.getEventType();
            }
            std::cout << std::endl;
        }
        
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
                        float x = ea.getTouchPointNormalizedX(j);
                        float y = ea.getTouchPointNormalizedY(j);
                        
                        // std::cout << j << ": " << tp.x << "/" << tp.y <<" "<< x << " " << y << " " << _w << " " << _h << std::endl;
                        
                        _mats[j]->setMatrix(osg::Matrix::translate((1+x) * 0.5 * _w, (1+y) * 0.5 * _h, 0));
                        _mats[j]->setNodeMask(0xffff);
                        
                        std::ostringstream ss;
                        ss << "Touch " << tp.id;
                        _texts[j]->setText(ss.str());
                        
                        switch (tp.phase) 
                        {
                            case osgGA::GUIEventAdapter::TOUCH_BEGAN:
                                    _drawables[j]->setColor(osg::Vec4(0,1,0,1));
                                    break;
                                    
                            case osgGA::GUIEventAdapter::TOUCH_MOVED:
                                    _drawables[j]->setColor(osg::Vec4(1,1,1,1));
                                    break;
                                    
                            case osgGA::GUIEventAdapter::TOUCH_ENDED:
                                    _drawables[j]->setColor(osg::Vec4(1,0,0,1));
                                    ++num_touch_ended;
                                    break;
                                    
                            case osgGA::GUIEventAdapter::TOUCH_STATIONERY:
                                    _drawables[j]->setColor(osg::Vec4(0.8,0.8,0.8,1));
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
    
    float _w, _h;

};


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);
    
    // if not loaded assume no arguments passed in, try use default model instead.
    if (!scene) scene = osgDB::readNodeFile("dumptruck.osgt");
    
    if (!scene) 
    {
            osg::Geode* geode = new osg::Geode();
            osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0,0,0), 100));
            drawable->setColor(osg::Vec4(0.5, 0.5, 0.5,1));
            geode->addDrawable(drawable);
            scene = geode;
    }


    // construct the viewer.
    osgViewer::Viewer viewer(arguments);
    
    
    //opening devices
    std::string device;
    while(arguments.read("--device", device))
    {
        osg::ref_ptr<osgGA::Device> dev = osgDB::readFile<osgGA::Device>(device);
        if (dev.valid())
        {
            viewer.addDevice(dev.get());
        }
    }
    
    
    osg::ref_ptr<osg::Group> group  = new osg::Group;

    // add the HUD subgraph.    
    if (scene.valid()) group->addChild(scene.get());
    
    viewer.setCameraManipulator(new osgGA::MultiTouchTrackballManipulator());
    viewer.realize();
    
    osg::GraphicsContext* gc = viewer.getCamera()->getGraphicsContext();
    
    #ifdef __APPLE__
        // as multitouch is disabled by default, enable it now
        osgViewer::GraphicsWindowCocoa* win = dynamic_cast<osgViewer::GraphicsWindowCocoa*>(gc);
        if (win) win->setMultiTouchEnabled(true);
    #endif
    
    std::cout << "creating hud with " << gc->getTraits()->width << "x" <<  gc->getTraits()->height << std::endl;
    osg::Camera* hud_camera = createHUD(gc->getTraits()->width, gc->getTraits()->height);
    
    
    viewer.addEventHandler(new TestMultiTouchEventHandler(hud_camera, gc->getTraits()->width, gc->getTraits()->height));
    
    
    group->addChild(hud_camera);

    // set the scene to render
    viewer.setSceneData(group.get());

    return viewer.run();

    
}
