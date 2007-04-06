// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of osgViewer::GraphicsWindow + SimpleViewer
// example that provides the user with control over view position with basic picking.

#include <osg/Timer>
#include <osg/io_utils>
#include <osg/observer_ptr>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>
#include <osgUtil/LineSegmentIntersector>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osgViewer/SimpleViewer>
#include <osgViewer/GraphicsWindow>

#include <osgFX/Scribe>

#include <iostream>

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

class DeleteSelectedNodesVisitor : public osg::NodeVisitor
{
public:

    DeleteSelectedNodesVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)        
    {
    }
    
    virtual void apply(osg::Node& node)
    {
        osgFX::Scribe* scribe = dynamic_cast<osgFX::Scribe*>(&node);
        if (scribe)
        {
            _selectedNodes.push_back(scribe);
        }
        else
        {
            traverse(node);
        }
    }
    
    void pruneSelectedNodes()
    {
        for(SelectedNodes::iterator itr = _selectedNodes.begin();
            itr != _selectedNodes.end();
            ++itr)
        {
            osg::Node* node = itr->get();
            osg::Node::ParentList parents = node->getParents();
            for(osg::Node::ParentList::iterator pitr = parents.begin();
                pitr != parents.end();
                ++pitr)
            {
                osg::Group* parent = *pitr;
                parent->removeChild(node);
            }
        }
    }
    
    typedef std::vector< osg::ref_ptr<osgFX::Scribe> > SelectedNodes;
    SelectedNodes _selectedNodes;
    
};

class ExitHandler : public osgGA::GUIEventHandler 
{
public: 

    ExitHandler():
        _done(false) {}
        
    bool done() const { return _done; }    

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Escape)
                {
                    _done = true;
                }
                return false;
            }
            case(osgGA::GUIEventAdapter::CLOSE_WINDOW):
            case(osgGA::GUIEventAdapter::QUIT_APPLICATION):
            {
                _done = true;
            }
            default: break;
        }
        
        return false;
    }
    
    bool _done;
};



// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler 
{
public: 

    PickHandler():
        _mx(0.0),_my(0.0) {}

    ~PickHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::SimpleViewer* viewer = dynamic_cast<osgViewer::SimpleViewer*>(&aa);
        if (!viewer) return false;

        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()=='s')
                {
                    saveSelectedModel(viewer->getSceneData());
                }                
                else if (ea.getKey()=='o')
                {
                    osg::notify(osg::NOTICE)<<"Saved model to file 'saved_model.osg'"<<std::endl;
                    osgDB::writeNodeFile(*(viewer->getSceneData()), "saved_model.osg");
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete || ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
                {
                    osg::notify(osg::NOTICE)<<"Delete"<<std::endl;
                    DeleteSelectedNodesVisitor dsnv;
                    viewer->getSceneData()->accept(dsnv);
                    dsnv.pruneSelectedNodes();
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

        osg::Node* node = 0;
        osg::Group* parent = 0;

        bool usePolytopePicking = false;
        if (usePolytopePicking)
        {

#if 0
            // use window coordinates
            // remap the mouse x,y into viewport coordinates.
            osg::Viewport* viewport = viewer->getCamera()->getViewport();
            double mx = viewport->x() + (int)((double )viewport->width()*(ea.getXnormalized()*0.5+0.5));
            double my = viewport->y() + (int)((double )viewport->height()*(ea.getYnormalized()*0.5+0.5));

            // half width, height.
            double w = 5.0f;
            double h = 5.0f;
            osgUtil::PolytopeIntersector* picker = new osgUtil::PolytopeIntersector( osgUtil::Intersector::WINDOW, mx-w, my-h, mx+w, my+h );
#else
            double mx = ea.getXnormalized();
            double my = ea.getYnormalized();
            double w = 0.05;
            double h = 0.05;
            osgUtil::PolytopeIntersector* picker = new osgUtil::PolytopeIntersector( osgUtil::Intersector::PROJECTION, mx-w, my-h, mx+w, my+h );
#endif
            osgUtil::IntersectionVisitor iv(picker);

            viewer->getCamera()->accept(iv);

            if (picker->containsIntersections())
            {
                osgUtil::PolytopeIntersector::Intersection intersection = picker->getFirstIntersection();

                osg::NodePath& nodePath = intersection.nodePath;
                node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
                parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

                if (node) std::cout<<"  Hits "<<node->className()<<" nodePath size"<<nodePath.size()<<std::endl;

            }

        }
        else
        {

            #if 0
            // use non dimensional coordinates - in projection/clip space
            osgUtil::LineSegmentIntersector* picker = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::PROJECTION, ea.getXnormalized(),ea.getYnormalized() );
            #else
            // use window coordinates
            // remap the mouse x,y into viewport coordinates.
            osg::Viewport* viewport = viewer->getCamera()->getViewport();
            float mx = viewport->x() + (int)((float)viewport->width()*(ea.getXnormalized()*0.5f+0.5f));
            float my = viewport->y() + (int)((float)viewport->height()*(ea.getYnormalized()*0.5f+0.5f));
            osgUtil::LineSegmentIntersector* picker = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::WINDOW, mx, my );
            #endif

            osgUtil::IntersectionVisitor iv(picker);

            viewer->getCamera()->accept(iv);

            if (picker->containsIntersections())
            {
                osgUtil::LineSegmentIntersector::Intersection intersection = picker->getFirstIntersection();
                osg::notify(osg::NOTICE)<<"Picked "<<intersection.localIntersectionPoint<<std::endl;

                osg::NodePath& nodePath = intersection.nodePath;
                node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
                parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

                if (node) std::cout<<"  Hits "<<node->className()<<" nodePath size"<<nodePath.size()<<std::endl;

            }
        }        

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
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 200;
    traits->y = 200;
    traits->width = 800;
    traits->height = 600;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (!gw)
    {
        osg::notify(osg::NOTICE)<<"Error: unable to create graphics window."<<std::endl;
        return 1;
    }

    gw->realize();
    gw->makeCurrent();

    // create the view of the scene.
    osgViewer::SimpleViewer viewer;
    viewer.setSceneData(loadedModel.get());
    
    viewer.setEventQueue(gw->getEventQueue());
    viewer.getEventQueue()->windowResize(traits->x,traits->y,traits->width,traits->height);

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
    statesetManipulator->setStateSet(viewer.getSceneView()->getGlobalStateSet());
    viewer.addEventHandler(statesetManipulator.get());

    // add the pick handler
    viewer.addEventHandler(new PickHandler());


    // add the exit handler'
    ExitHandler* exitHandler = new ExitHandler;
    viewer.addEventHandler(exitHandler);

    viewer.init();

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( gw->isRealized() && !exitHandler->done())
    {
        gw->checkEvents();
        
        if (gw->isRealized() && !exitHandler->done())
        {      
            viewer.frame();

            // Swap Buffers
            gw->swapBuffers();
        }
    }

    return 0;
}

