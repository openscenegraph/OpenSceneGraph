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
#include <osg/KdTree>
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
        _useWindowCoordinates(false),
        _precisionHint(osgUtil::Intersector::USE_DOUBLE_CALCULATIONS),
        _primitiveMask(osgUtil::PolytopeIntersector::ALL_PRIMITIVES) {}

    ~PickHandler() {}

    void setPrimitiveMask(unsigned int primitiveMask) { _primitiveMask = primitiveMask; }


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
                else if (ea.getKey()=='a')
                {
                    fullWindowIntersectionTest(viewer);
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

    void fullWindowIntersectionTest(osgViewer::Viewer* viewer)
    {

        osg::ref_ptr<osgUtil::IntersectorGroup> intersectors = new osgUtil::IntersectorGroup;

        osg::Viewport* viewport = viewer->getCamera()->getViewport();
        unsigned int numX = 100;
        unsigned int numY = 100;
        double dx = viewport->width()/double(numX-1);
        double dy = viewport->width()/double(numX-1);


        double y = viewport->x();
        for(unsigned int r=0; r<numY; ++r)
        {
            double x = viewport->x();
            for(unsigned int c=0; c<numX; ++c)
            {
                osg::ref_ptr<osgUtil::Intersector> intersector;


                if (_usePolytopeIntersector)
                {
                    osg::ref_ptr<osgUtil::PolytopeIntersector> pi = new osgUtil::PolytopeIntersector( osgUtil::Intersector::WINDOW, x-dx*0.5, y-dy*0.5, x+dx*0.5, y+dy*0.5);
                    pi->setPrimitiveMask(_primitiveMask);
                    intersector = pi.get();
                }
                else
                {
                    intersector = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::WINDOW, x, y);
                }

                intersector->setPrecisionHint(_precisionHint);
                intersectors->getIntersectors().push_back(intersector);

                x += dx;
            }
            y += dy;
        }


        osgUtil::IntersectionVisitor iv(intersectors.get());

        osg::ElapsedTime elapsedTime;
        viewer->getCamera()->accept(iv);

        OSG_NOTICE<<"Intersection traversal took "<<elapsedTime.elapsedTime_m()<<"ms for "<<intersectors->getIntersectors().size()<<" intersectors"<<std::endl;

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

            picker->setPrecisionHint(_precisionHint);
            picker->setPrimitiveMask(_primitiveMask);

            osgUtil::IntersectionVisitor iv(picker);

            osg::ElapsedTime elapsedTime;

            viewer->getCamera()->accept(iv);

            OSG_NOTICE<<"PoltyopeIntersector traversal took "<<elapsedTime.elapsedTime_m()<<"ms"<<std::endl;

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
            picker->setPrecisionHint(_precisionHint);

            osgUtil::IntersectionVisitor iv(picker);

            osg::ElapsedTime elapsedTime;

            viewer->getCamera()->accept(iv);

            OSG_NOTICE<<"LineSegmentIntersector traversal took "<<elapsedTime.elapsedTime_m()<<"ms"<<std::endl;

            if (picker->containsIntersections())
            {
                osgUtil::LineSegmentIntersector::Intersection intersection = picker->getFirstIntersection();
                osg::notify(osg::NOTICE)<<"Picked "<<intersection.localIntersectionPoint<<std::endl
                <<"  primitive index "<<intersection.primitiveIndex
                <<std::endl;

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

    void setPrecisionHint(osgUtil::Intersector::PrecisionHint hint) { _precisionHint = hint; }

protected:

    float _mx,_my;
    bool _usePolytopeIntersector;
    bool _useWindowCoordinates;
    osgUtil::Intersector::PrecisionHint _precisionHint;
    unsigned int _primitiveMask;

};

class ConvertPrimitives : public osg::NodeVisitor
{
public:

    osg::PrimitiveSet::Mode _mode;

    ConvertPrimitives(osg::PrimitiveSet::Mode mode):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _mode(mode) {}

        void apply(osg::Geometry& geometry)
        {
            if (!geometry.getVertexArray()) return;

            unsigned int numVertices = geometry.getVertexArray()->getNumElements();

            if (_mode==osg::PrimitiveSet::POINTS)
            {
                // remove previous primitive sets.
                geometry.removePrimitiveSet(0, geometry.getNumPrimitiveSets());
                geometry.addPrimitiveSet(new osg::DrawArrays(_mode, 0,numVertices));
            }
            else if (_mode==osg::PrimitiveSet::LINES)
            {
                geometry.removePrimitiveSet(0, geometry.getNumPrimitiveSets());
                geometry.addPrimitiveSet(new osg::DrawArrays(_mode, 0,numVertices));
            }
        }
};

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    bool useKdTree = false;
    while (arguments.read("--kdtree")) { useKdTree = true; }

    osg::ref_ptr<PickHandler> pickhandler = new PickHandler;
    while (arguments.read("--double")) { pickhandler->setPrecisionHint(osgUtil::Intersector::USE_DOUBLE_CALCULATIONS); }
    while (arguments.read("--float")) { pickhandler->setPrecisionHint(osgUtil::Intersector::USE_FLOAT_CALCULATIONS); }

    unsigned int mask = osgUtil::PolytopeIntersector::ALL_PRIMITIVES;
    while (arguments.read("--prim-mask", mask) || arguments.read("--pm", mask)) { pickhandler->setPrimitiveMask(mask); }

    // load model
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readRefNodeFile("dumptruck.osgt");

    if (!loadedModel)
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }

    while(arguments.read("--points")) { ConvertPrimitives cp(osg::PrimitiveSet::POINTS); loadedModel->accept(cp); }
    while(arguments.read("--lines")) { ConvertPrimitives cp(osg::PrimitiveSet::LINES); loadedModel->accept(cp); }

    if (useKdTree)
    {
        OSG_NOTICE<<"Building KdTrees"<<std::endl;
        osg::ref_ptr<osg::KdTreeBuilder> builder = new osg::KdTreeBuilder;
        loadedModel->accept(*builder);
    }


    // assign the scene graph to viewer
    viewer.setSceneData(loadedModel);

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
    viewer.addEventHandler(statesetManipulator.get());

    // add the pick handler
    viewer.addEventHandler(pickhandler.get());

    viewer.realize();

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while(!viewer.done())
    {
        viewer.frame();
    }

    return 0;
}

