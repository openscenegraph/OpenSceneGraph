#include <osgGLUT/Viewer>

#include <osg/MatrixTransform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/LineSegment>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>
#include <osgUtil/IntersectVisitor>

#include <osg/OccluderNode>
#include <osg/Geometry>


class OccluderEventHandler : public osgGA::GUIEventHandler
{
    public:
    
        OccluderEventHandler(osgUtil::SceneView* sceneview,osg::Group* rootnode):_sceneview(sceneview),_rootnode(rootnode) {}
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);

        virtual void accept(osgGA::GUIEventHandlerVisitor& v)
        {
            v.visit(*this);
        }
        
        void addPoint(const osg::Vec3& pos);
                
        void endOccluder();
        
        
        osg::ref_ptr<osgUtil::SceneView>        _sceneview;
        osg::ref_ptr<osg::Group>                _rootnode;
        osg::ref_ptr<osg::Group>                _occluders;
        osg::ref_ptr<osg::ConvexPlanarOccluder> _convexPlanarOccluder;
};

bool OccluderEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='a')
            {

                int x = ea.getX();
                int y = ea.getY();
 
                osg::Vec3 near_point,far_point;
                if (!_sceneview->projectWindowXYIntoObject(x,ea.getYmax()-y,near_point,far_point))
                {
                     return true;
                }

                osg::ref_ptr<osg::LineSegment> lineSegment = new osg::LineSegment;
                lineSegment->set(near_point,far_point);

                osgUtil::IntersectVisitor iv;
                iv.addLineSegment(lineSegment.get());

                _rootnode->accept(iv);

                if (iv.hits())
                {
                
                    osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(lineSegment.get());
                    if (!hitList.empty())
                    {
                    
                        osgUtil::Hit& hit = hitList.front();
                        addPoint(hit.getWorldIntersectPoint());
                    }

                }

                return true;
            }
            else if (ea.getKey()=='e')
            {
                endOccluder();
                return true;
            }
            else if (ea.getKey()=='O')
            {
                if (_occluders.valid())
                {
                    std::cout<<"saving occluders to 'saved_occluders.osg'"<<std::endl;
                    osgDB::writeNodeFile(*_occluders,"saved_occluders.osg");
                }
                else
                {
                    std::cout<<"no occluders to save"<<std::endl;
                }
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}

void OccluderEventHandler::addPoint(const osg::Vec3& pos)
{
    std::cout<<"add point "<<pos<<std::endl;
    
    if (!_convexPlanarOccluder.valid()) _convexPlanarOccluder = new osg::ConvexPlanarOccluder;
    
    osg::ConvexPlanarPolygon& occluder = _convexPlanarOccluder->getOccluder();
    occluder.add(pos);
    
}
                
void OccluderEventHandler::endOccluder()
{
    if (_convexPlanarOccluder.valid()) 
    {
        if (_convexPlanarOccluder->getOccluder().getVertexList().size()>=3)
        {
            osg::OccluderNode* occluderNode = new osg::OccluderNode;
            occluderNode->setOccluder(_convexPlanarOccluder.get());

            if (!_occluders.valid())
            {
                _occluders = new osg::Group;
                _rootnode->addChild(_occluders.get());
            }
            _occluders->addChild(occluderNode);

            std::cout<<"created occluder"<<std::endl;
        }
        else
        {
            std::cout<<"Occluder requires at least 3 points to create occluder."<<std::endl;
        }
    }
    else
    {
        std::cout<<"No occluder points to create occluder with."<<std::endl;
    }
    
    // reset current occluder.
    _convexPlanarOccluder = NULL;
}


osg::Node* createOccluder(const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3,const osg::Vec3& v4,float holeRatio=-1.0f)
{
   // create and occluder which will site along side the loadmodel model.
    osg::OccluderNode* occluderNode = new osg::OccluderNode;

    // create the convex planer occluder 
    osg::ConvexPlanarOccluder* cpo = new osg::ConvexPlanarOccluder;

    // attach it to the occluder node.
    occluderNode->setOccluder(cpo);
    occluderNode->setName("occluder");
    
    // set the occluder up for the front face of the bounding box.
    osg::ConvexPlanarPolygon& occluder = cpo->getOccluder();
    occluder.add(v1);
    occluder.add(v2);
    occluder.add(v3);
    occluder.add(v4);

    // create a whole at the center of the occluder if needed.
    if (holeRatio>0.0f)
    {
        // create hole.
        float ratio = holeRatio;
        float one_minus_ratio = 1-ratio;
        osg::Vec3 center = (v1+v2+v3+v4)*0.25f;
        osg::Vec3 v1dash = v1*ratio + center*one_minus_ratio;
        osg::Vec3 v2dash = v2*ratio + center*one_minus_ratio;
        osg::Vec3 v3dash = v3*ratio + center*one_minus_ratio;
        osg::Vec3 v4dash = v4*ratio + center*one_minus_ratio;

        osg::ConvexPlanarPolygon hole;
        hole.add(v1dash);
        hole.add(v2dash);
        hole.add(v3dash);
        hole.add(v4dash);

        cpo->addHole(hole);
    }    
    

   // create a drawable for occluder.
    osg::Geometry* geom = new osg::Geometry;
    
    osg::Vec3Array* coords = new osg::Vec3Array(occluder.getVertexList().begin(),occluder.getVertexList().end());
    geom->setVertexArray(coords);
    
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,0.5f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    
    osg::StateSet* stateset = new osg::StateSet;
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    geom->setStateSet(stateset);
    
    // add the occluder geode as a child of the occluder,
    // as the occluder can't self occlude its subgraph the
    // geode will never be occluder by this occluder.
    occluderNode->addChild(geode);    
    
    return occluderNode;
 
 }

osg::Group* createOccludersAroundModel(osg::Node* model)
{
    osg::Group* scene = new osg::Group;
    scene->setName("rootgroup");


    // add the loaded model into a the scene group.
    scene->addChild(model);
    model->setName("model");

    // get the bounding volume of the model.
    const osg::BoundingSphere bs = model->getBound();
    
    // create a bounding box around the sphere.
    osg::BoundingBox bb;
    bb.expandBy(bs);

   // front
   scene->addChild(createOccluder(bb.corner(0),
                                  bb.corner(1),
                                  bb.corner(5),
                                  bb.corner(4)));

   // right side
   scene->addChild(createOccluder(bb.corner(1),
                                  bb.corner(3),
                                  bb.corner(7),
                                  bb.corner(5)));

   // left side
   scene->addChild(createOccluder(bb.corner(2),
                                  bb.corner(0),
                                  bb.corner(4),
                                  bb.corner(6)));

   // back side
   scene->addChild(createOccluder(bb.corner(3),
                                  bb.corner(2),
                                  bb.corner(6),
                                  bb.corner(7),
                                  0.5f)); // create a hole half the size of the occluder.

    return scene;
} 


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-c","Mannually create occluders");
   
    // initialize the viewer.
    osgGLUT::Viewer viewer(arguments);

    bool manuallyCreateOccluders = false;
    while (arguments.read("-c")) { manuallyCreateOccluders = true; }

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

    // load the nodes from the commandline arguments.
    osg::Node* loadedmodel = osgDB::readNodeFiles(arguments);
    if (!loadedmodel)
    {
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(loadedmodel);

    // add the occluders to the loaded model.
    osg::Group* rootnode = NULL;
    
    if (manuallyCreateOccluders)
    {
        rootnode = new osg::Group;
        rootnode->addChild(loadedmodel);
    }
    else    
    {
        rootnode = createOccludersAroundModel(loadedmodel);
    }
    
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    if (manuallyCreateOccluders)
    {
        viewer.prependEventHandler(new OccluderEventHandler(viewer.getViewportSceneView(0),rootnode));
    }

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
