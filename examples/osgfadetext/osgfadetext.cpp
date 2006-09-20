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

#include <osgText/FadeText>

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

osgText::Text* createText(osg::EllipsoidModel* ellipsoid, double latitude, double longitude, double height, const std::string& str)
{
    double X,Y,Z;
    ellipsoid->convertLatLongHeightToXYZ( osg::DegreesToRadians(latitude), osg::DegreesToRadians(longitude), height, X, Y, Z);

    osgText::FadeText* text = new osgText::FadeText;

    text->setText(str);
    text->setFont("fonts/arial.ttf");
    text->setPosition(osg::Vec3(X,Y,Z));
    text->setCharacterSize(300000.0f);
    text->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
    text->setAutoRotateToScreen(true);
    
    return text;
}

osg::Node* createFadeText(osg::EllipsoidModel* ellipsoid)
{
    osg::Group* group = new osg::Group;

    group->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    
    osg::Geode* geode = new osg::Geode;
    group->addChild(geode);
    
    std::vector<std::string> textList;
    textList.push_back("Town");
    textList.push_back("City");
    textList.push_back("Village");
    textList.push_back("River");
    textList.push_back("Mountain");
    textList.push_back("Road");
    textList.push_back("Lake");
    
    unsigned int numLat = 15;
    unsigned int numLong = 20;
    double latitude = 50.0;
    double longitude = 0.0;
    double deltaLatitude = 1.0f;
    double deltaLongitude = 1.0f;

    unsigned int t = 0;
    for(unsigned int i = 0; i < numLat; ++i, latitude += deltaLatitude)
    {
        longitude = 0.0;
        for(unsigned int j = 0; j < numLong; ++j, ++t, longitude += deltaLongitude)
        {
            geode->addDrawable( createText( ellipsoid, latitude, longitude, 0, textList[t % textList.size()]) );
        }
    }

    return group;
} 


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
    osg::ref_ptr<osg::Node> root = createEarth();
    
    if (!root) return 0;

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root.get());

    osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>(root.get());
    if (csn)
    {
        // add fade text around the globe
        csn->addChild(createFadeText(csn->getEllipsoidModel()));
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
