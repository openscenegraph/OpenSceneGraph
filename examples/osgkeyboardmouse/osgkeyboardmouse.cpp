// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface + KeyboardMouseCallback + SceneView
// example that provides the user with control over view position with basic picking.

#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osg/Timer>
#include <osg/io_utils>
#include <osg/observer_ptr>

#include <osgUtil/SceneView>
#include <osgUtil/IntersectVisitor>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/EventQueue>
#include <osgGA/EventVisitor>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

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

    PickHandler(osgUtil::SceneView* sceneView):
        _sceneView(sceneView),
        _mx(0.0),_my(0.0) {}

    ~PickHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()=='s')
                {
                    saveSelectedModel();
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
                    pick(ea);
                }
                return true;
            }    

            default:
                return false;
        }
    }

    void pick(const osgGA::GUIEventAdapter& ea)
    {

        osg::Node* scene = _sceneView.valid() ? _sceneView->getSceneData() : 0;
        if (!scene) return;

        // remap the mouse x,y into viewport coordinates.
        
        float mx = _sceneView->getViewport()->x() + (int)((float)_sceneView->getViewport()->width()*(ea.getXnormalized()*0.5f+0.5f));
        float my = _sceneView->getViewport()->y() + (int)((float)_sceneView->getViewport()->height()*(ea.getYnormalized()*0.5f+0.5f));
   
        // do the pick traversal.
        osgUtil::PickVisitor pick(_sceneView->getViewport(),
                                  _sceneView->getProjectionMatrix(), 
                                  _sceneView->getViewMatrix(), mx, my);
        scene->accept(pick);

        osgUtil::PickVisitor::LineSegmentHitListMap& segHitList = pick.getSegHitList();
        if (!segHitList.empty() && !segHitList.begin()->second.empty())
        {
            std::cout<<"Got hits"<<std::endl;

            // get the hits for the first segment
            osgUtil::PickVisitor::HitList& hits = segHitList.begin()->second;

            // just take the first hit - nearest the eye point.
            osgUtil::Hit& hit = hits.front();

            osg::NodePath& nodePath = hit._nodePath;
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

        }
    }

    void saveSelectedModel()
    {
        CreateModelToSaveVisitor cmtsv;
        _sceneView->getSceneData()->accept(cmtsv);
        
        if (cmtsv._group->getNumChildren()>0)
        {
            std::cout<<"Writing selected compoents to 'selected_model.osg'"<<std::endl;
            osgDB::writeNodeFile(*cmtsv._group, "selected_model.osg");
        }
    }

protected:

    osg::observer_ptr<osgUtil::SceneView> _sceneView;
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
    osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView;
    sceneView->setDefaults();
    sceneView->setSceneData(loadedModel.get());
    
    // create the event queue, note that Producer has the y axis increase upwards, like OpenGL, and contary to most Windowing toolkits, so
    // we need to construct the event queue so that it knows about this convention.
    osg::ref_ptr<osgGA::EventQueue> eventQueue = new osgGA::EventQueue(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    // set up a KeyboardMouse to manage the events comming in from the RenderSurface
    osg::ref_ptr<Producer::KeyboardMouse>  kbm = new Producer::KeyboardMouse(renderSurface.get());

    // create a KeyboardMouseCallback to handle the mouse events within this applications
    osg::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(eventQueue.get());

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    osg::ref_ptr<osgGA::TrackballManipulator> cameraManipulator = new osgGA::TrackballManipulator;

    // keep a list of event handlers to manipulate the application/scene with in response to keyboard/mouse events
    typedef std::list< osg::ref_ptr<osgGA::GUIEventHandler> > EventHandlers;
    EventHandlers eventHandlers;
    
    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
    statesetManipulator->setStateSet(sceneView->getGlobalStateSet());
    eventHandlers.push_back(statesetManipulator.get());
    eventHandlers.push_back(new PickHandler(sceneView.get()));
    
    // create an event visitor to pass the events down to the scene graph nodes
    osg::ref_ptr<osgGA::EventVisitor> eventVisitor = new osgGA::EventVisitor;

    // create an action adapter to allow event handlers to request actions from the GUI.
    osg::ref_ptr<MyActionAdapter> actionAdapter = new MyActionAdapter;

    // record the timer tick at the start of rendering.    
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    
    unsigned int frameNum = 0;

    eventQueue->setStartTick(start_tick);

    // set the mouse input range (note WindowSize name in appropriate here so osgGA::GUIEventAdapter API really needs looking at, Robert Osfield, June 2006).
    // Producer defaults to using non-dimensional units, so we pass this onto osgGA, most windowing toolkits use pixel coords so use the window size instead.
    eventQueue->getCurrentEventState()->setWindowSize(-1.0, -1.0, 1.0, 1.0);


    // home the manipulator.
    osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent =  eventQueue->createEvent();
    cameraManipulator->setNode(sceneView->getSceneData());
    cameraManipulator->home(*dummyEvent, *actionAdapter);


    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( renderSurface->isRealized() && !kbmcb->done())
    {
        // set up the frame stamp for current frame to record the current time and frame number so that animtion code can advance correctly
        osg::FrameStamp* frameStamp = new osg::FrameStamp;
        frameStamp->setReferenceTime(osg::Timer::instance()->delta_s(start_tick,osg::Timer::instance()->tick()));
        frameStamp->setFrameNumber(frameNum++);
        
        // pass frame stamp to the SceneView so that the update, cull and draw traversals all use the same FrameStamp
        sceneView->setFrameStamp(frameStamp);

        // pass any keyboard mouse events onto the local keyboard mouse callback.
        kbm->update( *kbmcb );
        
        // create an event to signal the new frame.
        eventQueue->frame(frameStamp->getReferenceTime());

        // get the event since the last frame.
        osgGA::EventQueue::Events events;
        eventQueue->takeEvents(events);

        if (eventVisitor.valid())
        {
            eventVisitor->setTraversalNumber(frameStamp->getFrameNumber());
        }

        // dispatch the events in order of arrival.
        for(osgGA::EventQueue::Events::iterator event_itr = events.begin();
            event_itr != events.end();
            ++event_itr)
        {
            bool handled = false;

            if (eventVisitor.valid() && sceneView->getSceneData())
            {
                eventVisitor->reset();
                eventVisitor->addEvent(event_itr->get());
                sceneView->getSceneData()->accept(*eventVisitor);
                if (eventVisitor->getEventHandled())
                    handled = true;
            }

            if (cameraManipulator.valid() && !handled)
            {
                /*handled =*/ cameraManipulator->handle(*(*event_itr), *actionAdapter);
            }

            for(EventHandlers::iterator handler_itr=eventHandlers.begin();
                handler_itr!=eventHandlers.end() && !handled;
                ++handler_itr)
            {   
                handled = (*handler_itr)->handle(*(*event_itr),*actionAdapter,0,0);
            }

            // osg::notify(osg::NOTICE)<<"  Handled event "<<(*event_itr)->getTime()<<" "<< handled<<std::endl;

        }


        // update view matrices
        if (cameraManipulator.valid())
        {
            sceneView->setViewMatrix(cameraManipulator->getInverseMatrix());
        }

        // update the viewport dimensions, incase the window has been resized.
        sceneView->setViewport(0,0,renderSurface->getWindowWidth(),renderSurface->getWindowHeight());

        // do the update traversal the scene graph - such as updating animations
        sceneView->update();
        
        // do the cull traversal, collect all objects in the view frustum into a sorted set of rendering bins
        sceneView->cull();
        
        // draw the rendering bins.
        sceneView->draw();

        // Swap Buffers
        renderSurface->swapBuffers();
    }

    return 0;
}

