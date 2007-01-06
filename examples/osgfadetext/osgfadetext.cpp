#include <osgViewer/Viewer>

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/ClusterCullingCallback>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgText/FadeText>

#include <osgTerrain/DataSet>

#include <osgSim/OverlayNode>
#include <osgSim/SphereSegment>

#include <osgGA/TerrainManipulator>

#include <iostream>

class MyGraphicsContext {
    public:
        MyGraphicsContext()
        {
            osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
            traits->x = 0;
            traits->y = 0;
            traits->width = 1;
            traits->height = 1;
            traits->windowDecoration = false;
            traits->doubleBuffer = false;
            traits->sharedContext = 0;
            traits->pbuffer = true;

            _gc = osg::GraphicsContext::createGraphicsContext(traits.get());

            if (!_gc)
            {
                osg::notify(osg::NOTICE)<<"Failed to create pbuffer, failing back to normal graphics window."<<std::endl;
                
                traits->pbuffer = false;
                _gc = osg::GraphicsContext::createGraphicsContext(traits.get());
            }

            if (_gc.valid()) 
            
            
            {
                _gc->realize();
                _gc->makeCurrent();
                std::cout<<"Realized window"<<std::endl;
            }
        }
        
        bool valid() const { return _gc.valid() && _gc->isRealized(); }
        
    private:
        osg::ref_ptr<osg::GraphicsContext> _gc;
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

        MyGraphicsContext context;
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


    osgText::Text* text = new osgText::FadeText;

    osg::Vec3 normal = ellipsoid->computeLocalUpVector( X, Y, Z);
    text->setCullCallback(new osg::ClusterCullingCallback(osg::Vec3(X,Y,Z), normal, 0.0));

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
    double latitude = 0.0;
    double longitude = -100.0;
    double deltaLatitude = 1.0f;
    double deltaLongitude = 1.0f;

    unsigned int t = 0;
    for(unsigned int i = 0; i < numLat; ++i, latitude += deltaLatitude)
    {
        double lgnt = longitude;
        for(unsigned int j = 0; j < numLong; ++j, ++t, lgnt += deltaLongitude)
        {
            geode->addDrawable( createText( ellipsoid, latitude, lgnt, 0, textList[t % textList.size()]) );
        }
    }

    return group;
} 


int main(int, char**)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
    viewer.getCamera()->setNearFarRatio(0.00001f);
    
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

    viewer.setCameraManipulator(new osgGA::TerrainManipulator);

    return viewer.run();
}
