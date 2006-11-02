// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface + KeyboardMouseCallback + SimpleViewer
// example that provides the user with control over view position with basic picking.

#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osg/Timer>
#include <osg/io_utils>
#include <osg/observer_ptr>

#include <osgUtil/IntersectionVisitor>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osgViewer/SimpleViewer>

#include <osgFX/Scribe>

// ----------- Begining of glue classes to adapter Producer's keyboard mouse events to osgGA's abstraction events.
class MyKeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
public:

    MyKeyboardMouseCallback(osgGA::EventQueue* eventQueue) :
        _done(false),
        _eventQueue(eventQueue)
    {
    }

    virtual void shutdown()
    {
        _done = true; 
    }

    virtual void specialKeyPress( Producer::KeyCharacter key )
    {
        if (key==Producer::KeyChar_Escape)
                    shutdown();

        _eventQueue->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void specialKeyRelease( Producer::KeyCharacter key )
    {
        _eventQueue->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void keyPress( Producer::KeyCharacter key)
    {
        _eventQueue->keyPress( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void keyRelease( Producer::KeyCharacter key)
    {
        _eventQueue->keyRelease( (osgGA::GUIEventAdapter::KeySymbol) key );
    }

    virtual void mouseMotion( float mx, float my ) 
    {
        _eventQueue->mouseMotion( mx, my );
    }

    virtual void buttonPress( float mx, float my, unsigned int mbutton ) 
    {
        _eventQueue->mouseButtonPress(mx, my, mbutton);
    }
    
    virtual void buttonRelease( float mx, float my, unsigned int mbutton ) 
    {
        _eventQueue->mouseButtonRelease(mx, my, mbutton);
    }

    bool done() { return _done; }

private:

    bool                                _done;
    osg::ref_ptr<osgGA::EventQueue>     _eventQueue;
};

class MyActionAdapter : public osgGA::GUIActionAdapter, public osg::Referenced
{
public:
    // Override from GUIActionAdapter
    virtual void requestRedraw() {}

    // Override from GUIActionAdapter
    virtual void requestContinuousUpdate(bool =true) {}

    // Override from GUIActionAdapter
    virtual void requestWarpPointer(float ,float ) {}
    
};

// ----------- End of glue classes to adapter Producer's keyboard mouse events to osgGA's abstraction events.


class CreateModelToSaveVisitor : public osg::NodeVisitor
{
public:

    CreateModelToSaveVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)        
    {
        _group = new osg::Group;
        _addToModel = false;
    }
    
    virtual void apply(osg::Node& node)
    {
        osgFX::Scribe* scribe = dynamic_cast<osgFX::Scribe*>(&node);
        if (scribe)
        {
            for(unsigned int i=0; i<scribe->getNumChildren(); ++i)
            {
                _group->addChild(scribe->getChild(i));
            }
        }
        else
        {
            traverse(node);
        }
    }
    
    osg::ref_ptr<osg::Group> _group;
    bool _addToModel;
};

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public: 

    PickHandler():
        _mx(0.0),_my(0.0) {}

    ~PickHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::SimpleViewer* viewer = dynamic_cast<osgViewer::SimpleViewer*>(&aa);

        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()=='s' && viewer)
                {
                    saveSelectedModel(viewer->getSceneData());
                }
                return false;
            }
            case(osgGA::GUIEventAdapter::PUSH):
            case(osgGA::GUIEventAdapter::MOVE):
            {
                _mx = ea.getX();
                _my = ea.getY();
                return false;
            }
            case(osgGA::GUIEventAdapter::RELEASE):
            {
                if (_mx == ea.getX() && _my == ea.getY())
                {
                    // only do a pick if the mouse hasn't moved
                    pick(ea,viewer);
                }
                return true;
            }    

            default:
                return false;
        }
    }

    void pick(const osgGA::GUIEventAdapter& ea, osgViewer::SimpleViewer* viewer)
    {
        osg::Node* scene = viewer->getSceneData();
        if (!scene) return;

        

       osg::notify(osg::NOTICE)<<std::endl;

#if 0
        // use non dimension coordinates - in projection/clip space
        osgUtil::LineSegmentIntersector* picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::PROJECTION, osg::Vec3d(ea.getXnormalized(),ea.getYnormalized(),-1.0), osg::Vec3d(ea.getXnormalized(),ea.getYnormalized(),1.0) );
#else
        // use window coordinates
        // remap the mouse x,y into viewport coordinates.
        osg::Viewport* viewport = viewer->getCamera()->getViewport();
        float mx = viewport->x() + (int)((float)viewport->width()*(ea.getXnormalized()*0.5f+0.5f));
        float my = viewport->y() + (int)((float)viewport->height()*(ea.getYnormalized()*0.5f+0.5f));
        osgUtil::LineSegmentIntersector* picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, osg::Vec3d(mx,my,0.0), osg::Vec3d(mx,my,1.0) );

#endif

        osgUtil::IntersectionVisitor iv(picker);
        
        viewer->getCamera()->accept(iv);
        

        if (picker->containsIntersections())
        {
            osgUtil::LineSegmentIntersector::Intersection intersection = picker->getFirstIntersection();
            osg::notify(osg::NOTICE)<<"Picked "<<intersection.localIntersectionPoint<<std::endl;

#if 1
            osg::NodePath& nodePath = intersection.nodePath;
            osg::Node* node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
            osg::Group* parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

            if (node) std::cout<<"  Hits "<<node->className()<<" nodePath size"<<nodePath.size()<<std::endl;

            // now we try to decorate the hit node by the osgFX::Scribe to show that its been "picked"
            if (parent && node)
            {

                std::cout<<"  parent "<<parent->className()<<std::endl;

                osgFX::Scribe* parentAsScribe = dynamic_cast<osgFX::Scribe*>(parent);
                if (!parentAsScribe)
                {
                    // node not already picked, so highlight it with an osgFX::Scribe
                    osgFX::Scribe* scribe = new osgFX::Scribe();
                    scribe->addChild(node);
                    parent->replaceChild(node,scribe);
                }
                else
                {
                    // node already picked so we want to remove scribe to unpick it.
                    osg::Node::ParentList parentList = parentAsScribe->getParents();
                    for(osg::Node::ParentList::iterator itr=parentList.begin();
                        itr!=parentList.end();
                        ++itr)
                    {
                        (*itr)->replaceChild(parentAsScribe,node);
                    }
                }
            }
#endif

        }
        
    }

    void saveSelectedModel(osg::Node* scene)
    {
        if (!scene) return;
    
        CreateModelToSaveVisitor cmtsv;
        scene->accept(cmtsv);
        
        if (cmtsv._group->getNumChildren()>0)
        {
            std::cout<<"Writing selected compoents to 'selected_model.osg'"<<std::endl;
            osgDB::writeNodeFile(*cmtsv._group, "selected_model.osg");
        }
    }

protected:

    float _mx,_my;
};

int main( int argc, char **argv )
{
    if (argc<2) 
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }
    
    // create the window to draw to.
    osg::ref_ptr<Producer::RenderSurface> renderSurface = new Producer::RenderSurface;
    renderSurface->setWindowName("osgkeyboardmouse");
    renderSurface->setWindowRectangle(100,100,800,600);
    renderSurface->useBorder(true);
    renderSurface->realize();
    

    // create the view of the scene.
    osgViewer::SimpleViewer viewer;
    viewer.setSceneData(loadedModel.get());
    
    // set up a KeyboardMouse to manage the events comming in from the RenderSurface
    osg::ref_ptr<Producer::KeyboardMouse>  kbm = new Producer::KeyboardMouse(renderSurface.get());

    // create a KeyboardMouseCallback to handle the mouse events within this applications
    osg::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(viewer.getEventQueue());

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
    statesetManipulator->setStateSet(viewer.getSceneView()->getGlobalStateSet());
    viewer.addEventHandler(statesetManipulator.get());

    // add the pick handler
    viewer.addEventHandler(new PickHandler());
    
    // set the window dimensions 
    viewer.getEventQueue()->getCurrentEventState()->setWindowRectangle(100,100,800,600);

    // set the mouse input range.
    // Producer defaults to using non-dimensional units, so we pass this onto osgGA, most windowing toolkits use pixel coords so use the window size instead.
    viewer.getEventQueue()->getCurrentEventState()->setInputRange(-1.0, -1.0, 1.0, 1.0);

    // Producer has the y axis increase upwards, like OpenGL, and contary to most Windowing toolkits.
    // we need to construct the event queue so that it knows about this convention.
    viewer.getEventQueue()->getCurrentEventState()->setMouseYOrientation(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    viewer.init();

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( renderSurface->isRealized() && !kbmcb->done())
    {
        // update the window dimensions, in case the window has been resized.
         viewer.getEventQueue()->windowResize(0,0,renderSurface->getWindowWidth(),renderSurface->getWindowHeight(), false);

        // pass any keyboard mouse events onto the local keyboard mouse callback.
        kbm->update( *kbmcb );
        
        viewer.frame();

        // Swap Buffers
        renderSurface->swapBuffers();
    }

    return 0;
}

