/* OpenSceneGraph example, osgkeyboardmouse.
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

// Simple example of use of osgViewer::GraphicsWindow + Viewer
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

#include <osgViewer/Viewer>

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
                (*pitr)->removeChild(node);
            }
        }
    }

    typedef std::vector< osg::ref_ptr<osgFX::Scribe> > SelectedNodes;
    SelectedNodes _selectedNodes;

};

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler
{
public:

    PickHandler():
        _mx(0.0),_my(0.0),
        _usePolytopeIntersector(false),
        _useWindowCoordinates(false) {}

    ~PickHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
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
                    osg::notify(osg::NOTICE)<<"Saved model to file 'saved_model.osgt'"<<std::endl;
                    osgDB::writeNodeFile(*(viewer->getSceneData()), "saved_model.osgt");
                }
                else if (ea.getKey()=='p')
                {
                    _usePolytopeIntersector = !_usePolytopeIntersector;
                    if (_usePolytopeIntersector)
                    {
                        osg::notify(osg::NOTICE)<<"Using PolytopeIntersector"<<std::endl;
                    } else {
                        osg::notify(osg::NOTICE)<<"Using LineSegmentIntersector"<<std::endl;
                    }
                }
                else if (ea.getKey()=='c')
                {
                    _useWindowCoordinates = !_useWindowCoordinates;
                    if (_useWindowCoordinates)
                    {
                        osg::notify(osg::NOTICE)<<"Using window coordinates for picking"<<std::endl;
                    } else {
                        osg::notify(osg::NOTICE)<<"Using projection coordiates for picking"<<std::endl;
                    }
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

    void pick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
    {
        osg::Node* scene = viewer->getSceneData();
        if (!scene) return;

        osg::notify(osg::NOTICE)<<std::endl;

        osg::Node* node = 0;
        osg::Group* parent = 0;

        if (_usePolytopeIntersector)
        {
            osgUtil::PolytopeIntersector* picker;
            if (_useWindowCoordinates)
            {
                // use window coordinates
                // remap the mouse x,y into viewport coordinates.
                osg::Viewport* viewport = viewer->getCamera()->getViewport();
                double mx = viewport->x() + (int)((double )viewport->width()*(ea.getXnormalized()*0.5+0.5));
                double my = viewport->y() + (int)((double )viewport->height()*(ea.getYnormalized()*0.5+0.5));

                // half width, height.
                double w = 5.0f;
                double h = 5.0f;
                picker = new osgUtil::PolytopeIntersector( osgUtil::Intersector::WINDOW, mx-w, my-h, mx+w, my+h );
            } else {
                double mx = ea.getXnormalized();
                double my = ea.getYnormalized();
                double w = 0.05;
                double h = 0.05;
                picker = new osgUtil::PolytopeIntersector( osgUtil::Intersector::PROJECTION, mx-w, my-h, mx+w, my+h );
            }
            osgUtil::IntersectionVisitor iv(picker);

            viewer->getCamera()->accept(iv);

            if (picker->containsIntersections())
            {
                osgUtil::PolytopeIntersector::Intersection intersection = picker->getFirstIntersection();

                osg::notify(osg::NOTICE)<<"Picked "<<intersection.localIntersectionPoint<<std::endl
                    <<"  Distance to ref. plane "<<intersection.distance
                    <<", max. dist "<<intersection.maxDistance
                    <<", primitive index "<<intersection.primitiveIndex
                    <<", numIntersectionPoints "
                    <<intersection.numIntersectionPoints
                    <<std::endl;

                osg::NodePath& nodePath = intersection.nodePath;
                node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
                parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

                if (node) std::cout<<"  Hits "<<node->className()<<" nodePath size "<<nodePath.size()<<std::endl;
                toggleScribe(parent, node);
            }

        }
        else
        {
            osgUtil::LineSegmentIntersector* picker;
            if (!_useWindowCoordinates)
            {
                // use non dimensional coordinates - in projection/clip space
                picker = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::PROJECTION, ea.getXnormalized(),ea.getYnormalized() );
            } else {
                // use window coordinates
                // remap the mouse x,y into viewport coordinates.
                osg::Viewport* viewport = viewer->getCamera()->getViewport();
                float mx = viewport->x() + (int)((float)viewport->width()*(ea.getXnormalized()*0.5f+0.5f));
                float my = viewport->y() + (int)((float)viewport->height()*(ea.getYnormalized()*0.5f+0.5f));
                picker = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::WINDOW, mx, my );
            }
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
                toggleScribe(parent, node);
            }
        }

        // now we try to decorate the hit node by the osgFX::Scribe to show that its been "picked"
    }

    void toggleScribe(osg::Group* parent, osg::Node* node) {
        if (!parent || !node) return;

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

    void saveSelectedModel(osg::Node* scene)
    {
        if (!scene) return;

        CreateModelToSaveVisitor cmtsv;
        scene->accept(cmtsv);

        if (cmtsv._group->getNumChildren()>0)
        {
            std::cout<<"Writing selected compoents to 'selected_model.osgt'"<<std::endl;
            osgDB::writeNodeFile(*cmtsv._group, "selected_model.osgt");
        }
    }

protected:

    float _mx,_my;
    bool _usePolytopeIntersector;
    bool _useWindowCoordinates;
};

int main( int argc, char **argv )
{
    osg::ref_ptr<osg::Node> loadedModel;

    // load the scene.
    if (argc>1) loadedModel = osgDB::readNodeFile(argv[1]);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readNodeFile("dumptruck.osgt");

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

    // create the view of the scene.
    osgViewer::Viewer viewer;
    viewer.getCamera()->setGraphicsContext(gc.get());
    viewer.getCamera()->setViewport(0,0,800,600);
    viewer.setSceneData(loadedModel.get());

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
    viewer.addEventHandler(statesetManipulator.get());

    // add the pick handler
    viewer.addEventHandler(new PickHandler());

    viewer.realize();

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while(!viewer.done())
    {
        viewer.frame();
    }

    return 0;
}

