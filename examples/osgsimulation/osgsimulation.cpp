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
    }
        
    return scene.release();
    
}


class ModelPositionCallback : public osg::NodeCallback
{
public:

    ModelPositionCallback():
        _latitude(0.0),
        _longitude(0.0),
        _height(1000.0)
         {}

    void updateParameters()
    {
        _longitude += (2.0*osg::PI)/360.0;
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
                    osg::Matrixd matrix;
                    for(i+=1; i<nodePath.size()-1; ++i)
                    {
                        osg::Transform* transform = nodePath[i]->asTransform();
                        if (transform) transform->computeLocalToWorldMatrix(matrix, nv);
                    }

                    //osg::Matrixd matrix;
                    ellipsoid->computeLocalToWorldTransformFromLatLongHeight(_latitude,_longitude,_height,matrix);
                    matrix.preMult(osg::Matrixd::rotate(_rotation));
                    
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


void addModel(osgProducer::Viewer* viewer,osg::Node* model)
{
    
}


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of particle systems.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file_left_eye image_file_right_eye");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);



    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

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
    
    osg::Node *root = createEarth();

    if (!root) return 0;
   
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);
        
        
    FindNamedNodeVisitor fnnv("cessna");
    root->accept(fnnv);
    
    if (!fnnv._foundNodes.empty())
    {
        osgGA::NodeTrackerManipulator* tm = new osgGA::NodeTrackerManipulator;
        tm->setTrackNode(fnnv._foundNodes[0].get());
        
        std::cout<<"Found "<<std::endl;
        
        unsigned int num = viewer.addCameraManipulator(tm);
        viewer.selectCameraManipulator(num);
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
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}
