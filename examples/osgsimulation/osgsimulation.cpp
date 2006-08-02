#ifdef WIN32
/////////////////////////////////////////////////////////////////////////////
// Disable unavoidable warning messages:

//  4103: used #pragma pack to change alignment
//  4114: same type qualifier used more than once
//  4201: nonstandard extension used : nameless struct/union
//  4237: "keyword" reserved for future use
//  4251: class needs to have dll-interface to export class
//  4275: non DLL-interface class used as base for DLL-interface class
//  4290: C++ Exception Specification ignored
//  4503: decorated name length exceeded, name was truncated
//  4786: string too long - truncated to 255 characters

#pragma warning(disable : 4103 4114 4201 4237 4251 4275 4290 4503 4335 4786)

#endif // WIN32

#include <osgProducer/Viewer>

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgText/Text>

#include <osgTerrain/DataSet>

#include <osgSim/OverlayNode>
#include <osgSim/SphereSegment>

#include <osgGA/NodeTrackerManipulator>

class GraphicsContext {
    public:
        GraphicsContext()
        {
            rs = new Producer::RenderSurface;
            rs->setWindowRectangle(0,0,1,1);
            rs->useBorder(false);
            rs->useConfigEventThread(false);
            rs->realize();
        }

        virtual ~GraphicsContext()
        {
        }
        
    private:
        Producer::ref_ptr<Producer::RenderSurface> rs;
};

osg::Node* createEarth()
{
    osg::ref_ptr<osg::Node> scene;
    
    {
        std::string filename = osgDB::findDataFile("Images/land_shallow_topo_2048.jpg");

        // make osgTerrain::DataSet quieter..
        osgTerrain::DataSet::setNotifyOffset(1);

        osg::ref_ptr<osgTerrain::DataSet> dataSet = new osgTerrain::DataSet;

        // register the source imagery
        {
            osgTerrain::DataSet::Source* source = new osgTerrain::DataSet::Source(osgTerrain::DataSet::Source::IMAGE, filename);

            source->setCoordinateSystemPolicy(osgTerrain::DataSet::Source::PREFER_CONFIG_SETTINGS);
            source->setCoordinateSystem(osgTerrain::DataSet::coordinateSystemStringToWTK("WGS84"));

             source->setGeoTransformPolicy(osgTerrain::DataSet::Source::PREFER_CONFIG_SETTINGS_BUT_SCALE_BY_FILE_RESOLUTION);
            source->setGeoTransformFromRange(-180.0, 180.0, -90.0, 90.0);

            dataSet->addSource(source);
        }

        // set up destination database paramters.
        dataSet->setDatabaseType(osgTerrain::DataSet::LOD_DATABASE);
        dataSet->setConvertFromGeographicToGeocentric(true);
        dataSet->setDestinationName("test.osg");

        // load the source data and record sizes.
        dataSet->loadSources();

        GraphicsContext context;
        dataSet->createDestination(30);

        if (dataSet->getDatabaseType()==osgTerrain::DataSet::LOD_DATABASE) dataSet->buildDestination();
        else dataSet->writeDestination();
        
        scene = dataSet->getDestinationRootNode();
        
        // now we must get rid of all the old OpenGL objects before we start using the scene graph again 
        // otherwise it'll end up in an inconsistent state.
        scene->releaseGLObjects(dataSet->getState());
        osg::Texture::flushAllDeletedTextureObjects(0);
        osg::Drawable::flushAllDeletedDisplayLists(0);
    }
        
    return scene.release();
    
}


class ModelPositionCallback : public osg::NodeCallback
{
public:

    ModelPositionCallback():
        _latitude(0.0),
        _longitude(0.0),
        _height(100000.0)
    {
        _rotation.makeRotate(osg::DegreesToRadians(90.0),0.0,0.0,1.0);
    }
    
    void updateParameters()
    {
        _longitude += ((2.0*osg::PI)/360.0)/20.0;
    }


    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        updateParameters();
        
        osg::NodePath nodePath = nv->getNodePath();

        osg::MatrixTransform* mt = nodePath.empty() ? 0 : dynamic_cast<osg::MatrixTransform*>(nodePath.back());
        if (mt)
        {
            osg::CoordinateSystemNode* csn = 0;

            // find coordinate system node from our parental chain
            unsigned int i;
            for(i=0; i<nodePath.size() && csn==0; ++i)
            {
                csn = dynamic_cast<osg::CoordinateSystemNode*>(nodePath[i]);
            }
            
            if (csn)
            {


                osg::EllipsoidModel* ellipsoid = csn->getEllipsoidModel();
                if (ellipsoid)
                {
                    osg::Matrix inheritedMatrix;
                    for(i+=1; i<nodePath.size()-1; ++i)
                    {
                        osg::Transform* transform = nodePath[i]->asTransform();
                        if (transform) transform->computeLocalToWorldMatrix(inheritedMatrix, nv);
                    }
                    
                    osg::Matrixd matrix(inheritedMatrix);

                    //osg::Matrixd matrix;
                    ellipsoid->computeLocalToWorldTransformFromLatLongHeight(_latitude,_longitude,_height,matrix);
                    matrix.preMult(osg::Matrix::rotate(_rotation));
                    
                    mt->setMatrix(matrix);
                }

            }        
        }
            
        traverse(node,nv);
    }   
    
    double                  _latitude;
    double                  _longitude;
    double                  _height;
    osg::Quat               _rotation;
};


class FindNamedNodeVisitor : public osg::NodeVisitor
{
public:
    FindNamedNodeVisitor(const std::string& name):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _name(name) {}
    
    virtual void apply(osg::Node& node)
    {
        if (node.getName()==_name)
        {
            _foundNodes.push_back(&node);
        }
        traverse(node);
    }
    
    typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;

    std::string _name;
    NodeList _foundNodes;
};


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of node tracker.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName());
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    viewer.getCullSettings().setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
    viewer.getCullSettings().setNearFarRatio(0.00001f);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    osg::Quat rotation;
    osg::Vec4 vec4;
    while (arguments.read("--rotate-model",vec4[0],vec4[1],vec4[2],vec4[3]))
    {
        osg::Quat local_rotate;
        local_rotate.makeRotate(osg::DegreesToRadians(vec4[0]),vec4[1],vec4[2],vec4[3]);
        
        rotation = rotation * local_rotate;
    }

    osg::NodeCallback* nc = 0;
    std::string flightpath_filename;
    while (arguments.read("--flight-path",flightpath_filename))
    {
        std::ifstream fin(flightpath_filename.c_str());
        if (fin)
        {
            osg::AnimationPath* path = new osg::AnimationPath;
            path->read(fin);
            nc = new osg::AnimationPathCallback(path);
        }
    }
    
    osgGA::NodeTrackerManipulator::TrackerMode trackerMode = osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION;
    std::string mode;
    while (arguments.read("--tracker-mode",mode))
    {
        if (mode=="NODE_CENTER_AND_ROTATION") trackerMode = osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION;
        else if (mode=="NODE_CENTER_AND_AZIM") trackerMode = osgGA::NodeTrackerManipulator::NODE_CENTER_AND_AZIM;
        else if (mode=="NODE_CENTER") trackerMode = osgGA::NodeTrackerManipulator::NODE_CENTER;
        else
        {
            std::cout<<"Unrecognized --tracker-mode option "<<mode<<", valid options are:"<<std::endl;
            std::cout<<"    NODE_CENTER_AND_ROTATION"<<std::endl;
            std::cout<<"    NODE_CENTER_AND_AZIM"<<std::endl;
            std::cout<<"    NODE_CENTER"<<std::endl;
            return 1;
        }
    }
    
    
    osgGA::NodeTrackerManipulator::RotationMode rotationMode = osgGA::NodeTrackerManipulator::TRACKBALL;
    while (arguments.read("--rotation-mode",mode))
    {
        if (mode=="TRACKBALL") rotationMode = osgGA::NodeTrackerManipulator::TRACKBALL;
        else if (mode=="ELEVATION_AZIM") rotationMode = osgGA::NodeTrackerManipulator::ELEVATION_AZIM;
        else
        {
            std::cout<<"Unrecognized --rotation-mode option "<<mode<<", valid options are:"<<std::endl;
            std::cout<<"    TRACKBALL"<<std::endl;
            std::cout<<"    ELEVATION_AZIM"<<std::endl;
            return 1;
        }
    }

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
    
    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(arguments);

    if (!root) root = createEarth();
    
    if (!root) return 0;

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root.get());

    osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>(root.get());
    if (csn)
    {

        bool insertOverlayNode = true;
        osg::ref_ptr<osgSim::OverlayNode> overlayNode;
        if (insertOverlayNode)
        {
        
            overlayNode = new osgSim::OverlayNode;
            
            // insert the OverlayNode between the coordinate system node and its children.
            for(unsigned int i=0; i<csn->getNumChildren(); ++i)
            {
                overlayNode->addChild( csn->getChild(i) );
            }

            csn->removeChildren(0, csn->getNumChildren());
            csn->addChild(overlayNode.get());
            
            // tell the overlay node to continously update its overlay texture
            // as we know we'll be tracking a moving target.
            overlayNode->setContinuousUpdate(true);
        }
        
        
        osg::Node* cessna = osgDB::readNodeFile("cessna.osg");
        if (cessna)
        {
            double s = 200000.0 / cessna->getBound().radius();
        
            osg::MatrixTransform* scaler = new osg::MatrixTransform;
            scaler->addChild(cessna);
            scaler->setMatrix(osg::Matrixd::scale(s,s,s)*osg::Matrixd::rotate(rotation));
            scaler->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL,osg::StateAttribute::ON);        
        
            if (false)
            {
                osgSim::SphereSegment* ss = new osgSim::SphereSegment(
                                    osg::Vec3(0.0f,0.0f,0.0f), // center
                                    19.9f, // radius
                                    osg::DegreesToRadians(135.0f),
                                    osg::DegreesToRadians(240.0f),
                                    osg::DegreesToRadians(-10.0f),
                                    osg::DegreesToRadians(30.0f),
                                    60);
                                    
                scaler->addChild(ss);
            }

            osg::MatrixTransform* mt = new osg::MatrixTransform;
            mt->addChild(scaler);


            if (!nc) nc = new ModelPositionCallback;

            mt->setUpdateCallback(nc);

            csn->addChild(mt);
            
            // if we are using an overaly node, use the cessna subgraph as the overlay subgraph
            if (overlayNode.valid())
            {
                overlayNode->setOverlaySubgraph(mt);
            }

            osgGA::NodeTrackerManipulator* tm = new osgGA::NodeTrackerManipulator;
            tm->setTrackerMode(trackerMode);
            tm->setRotationMode(rotationMode);
            tm->setTrackNode(scaler);

            unsigned int num = viewer.addCameraManipulator(tm);
            viewer.selectCameraManipulator(num);
        }
        else
        {
             std::cout<<"Failed to read cessna.osg"<<std::endl;
        }
        
    }    

        

    // create the windows and run the threads.
    viewer.realize();

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
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
