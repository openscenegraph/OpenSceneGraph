#include <iostream>

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/Material>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osg/Billboard>
#include <osg/LineWidth>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>


#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/NodeTrackerManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgProducer/Viewer>


static osg::Vec3 defaultPos( 0.0f, 0.0f, 0.0f );
static osg::Vec3 centerScope(0.0f, 0.0f, 0.0f);

/** create quad at specified position. */
osg::Drawable* createSquare(const osg::Vec3& corner,const osg::Vec3& width,const osg::Vec3& height, osg::Image* image=NULL)
{
    // set up the Geometry.
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0] = corner;
    (*coords)[1] = corner+width;
    (*coords)[2] = corner+width+height;
    (*coords)[3] = corner+height;


    geom->setVertexArray(coords);

    osg::Vec3Array* norms = new osg::Vec3Array(1);
    (*norms)[0] = width^height;
    (*norms)[0].normalize();
    
    geom->setNormalArray(norms);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec2Array* tcoords = new osg::Vec2Array(4);
    (*tcoords)[0].set(0.0f,0.0f);
    (*tcoords)[1].set(1.0f,0.0f);
    (*tcoords)[2].set(1.0f,1.0f);
    (*tcoords)[3].set(0.0f,1.0f);
    geom->setTexCoordArray(0,tcoords);
    
    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colours);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);


    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
    
    if (image)
    {
        osg::StateSet* stateset = new osg::StateSet;
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        geom->setStateSet(stateset);
    }
    
    return geom;
}

osg::Image* createBillboardImage(const osg::Vec4& centerColour, unsigned int size, float power)
{
    osg::Vec4 backgroundColour = centerColour;
    backgroundColour[3] = 0.0f;
    
    osg::Image* image = new osg::Image;
    image->allocateImage(size,size,1,
                         GL_RGBA,GL_UNSIGNED_BYTE);
     
     
    float mid = (float(size)-1)*0.5f;
    float div = 2.0f/float(size);
    for(unsigned int r=0;r<size;++r)
    {
        unsigned char* ptr = image->data(0,r,0);
        for(unsigned int c=0;c<size;++c)
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float r = powf(1.0f-sqrtf(dx*dx+dy*dy),power);
            if (r<0.0f) r=0.0f;
            osg::Vec4 color = centerColour*r+backgroundColour*(1.0f-r);
            // color.set(1.0f,1.0f,1.0f,0.5f);
            *ptr++ = (unsigned char)((color[0])*255.0f);
            *ptr++ = (unsigned char)((color[1])*255.0f);
            *ptr++ = (unsigned char)((color[2])*255.0f);
            *ptr++ = (unsigned char)((color[3])*255.0f);
        }
    }
    return image;

    //return osgDB::readImageFile("spot.dds");
}


osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime)
{
    // set up the animation path 
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);
    
    int numSamples = 40;
    float yaw = 0.0f;
    float yaw_delta = 2.0f*osg::PI/((float)numSamples-1.0f);
    float roll = osg::inDegrees(30.0f);
    
    double time=0.0f;
    double time_delta = looptime/(double)numSamples;
    for(int i=0;i<numSamples;++i)
    {
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,cosf(yaw)*radius,0.0f));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));
        
        animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));

        yaw += yaw_delta;
        time += time_delta;

    }
    return animationPath;    
}// end createAnimationPath



class SolarSystem
{

public:
    double _radiusSun;
    double _radiusEarth;
    double _RorbitEarth;
    double _tiltEarth;
    double _rotateSpeedEarthAndMoon;
    double _rotateSpeedEarth;
    double _radiusMoon;
    double _RorbitMoon;
    double _rotateSpeedMoon;
    double _radiusSpace;
    
    SolarSystem()
    {
        _radiusSun = 5.0;
        _radiusEarth = 2.0;
        _RorbitEarth = 10.0;
        _tiltEarth = 18.0;
        _rotateSpeedEarthAndMoon = 1.0;
        _rotateSpeedEarth = 1.0;
        _radiusMoon = 0.5;
        _RorbitMoon = 2.0;
        _rotateSpeedMoon = 1.0;
        _radiusSpace = 300.0;
    }
    
    osg::MatrixTransform* createEarthTranslationAndTilt();
    osg::MatrixTransform* createRotation( double orbit, double speed );
    osg::MatrixTransform* createMoonTranslation();
    osg::Geode* createSpace( const std::string& name, const std::string& textureName );
    osg::Geode* createPlanet( double radius, const std::string& name, const osg::Vec4& color , const std::string& textureName );
    osg::Geode* createPlanet( double radius, const std::string& name, const osg::Vec4& color , const std::string& textureName1, const std::string& textureName2);
    osg::Group* createSunLight();
    osg::Group* built();
    
    void printParameters()
    {
        std::cout << "radiusSun\t= " << _radiusSun << std::endl;
        std::cout << "radiusEarth\t= " << _radiusEarth << std::endl;
        std::cout << "RorbitEarth\t= " << _RorbitEarth << std::endl;
        std::cout << "tiltEarth\t= " << _tiltEarth << std::endl;
        std::cout << "rotateSpeedEarthAndMoon= " << _rotateSpeedEarthAndMoon     << std::endl;
        std::cout << "rotateSpeedEarth= " << _rotateSpeedEarth << std::endl;
        std::cout << "radiusMoon\t= " << _radiusMoon << std::endl;
        std::cout << "RorbitMoon\t= " << _RorbitMoon << std::endl;
        std::cout << "rotateSpeedMoon\t= " << _rotateSpeedMoon << std::endl;
        std::cout << "radiusSpace\t= " << _radiusSpace << std::endl;
        
    }
    
};  // end SolarSystem

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

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of osg::AnimationPath and UpdateCallbacks for adding animation to your scenes.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

    // initialize the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE | osgProducer::Viewer::VIEWER_MANIPULATOR | osgProducer::Viewer::STATE_MANIPULATOR);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    SolarSystem solarSystem;

    while (arguments.read("--radiusSun",solarSystem._radiusSun)) { }
    while (arguments.read("--radiusEarth",solarSystem._radiusEarth)) { }
    while (arguments.read("--RorbitEarth",solarSystem._RorbitEarth)) { }
    while (arguments.read("--tiltEarth",solarSystem._tiltEarth)) { }
    while (arguments.read("--rotateSpeedEarthAndMoon",solarSystem._rotateSpeedEarthAndMoon)) { }
    while (arguments.read("--rotateSpeedEarth",solarSystem._rotateSpeedEarth)) { }
    while (arguments.read("--radiusMoon",solarSystem._radiusMoon)) { }
    while (arguments.read("--RorbitMoon",solarSystem._RorbitMoon)) { }
    while (arguments.read("--rotateSpeedMoon",solarSystem._rotateSpeedMoon)) { }
    while (arguments.read("--radiusSpace",solarSystem._radiusSpace)) { }
    
    
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
    

    solarSystem.printParameters();

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        std::cout << "setup the following arguments: " << std::endl;
        std::cout << "--radiusSun: double" << std::endl;
        std::cout << "--radiusEarth: double" << std::endl;
        std::cout << "--RorbitEarth: double" << std::endl;
        std::cout << "--tiltEarth: double" << std::endl;
        std::cout << "--rotateSpeedEarthAndMoon: double" << std::endl;
        std::cout << "--rotateSpeedEarth: double" << std::endl;
        std::cout << "--radiusMoon: double" << std::endl;
        std::cout << "--RorbitMoon: double" << std::endl;
        std::cout << "--rotateSpeedMoon: double" << std::endl;
        std::cout << "--radiusSpace: double" << std::endl;
        
                
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
    
    
    osg::Group* root = new osg::Group;

    osg::Group* sunLight = solarSystem.createSunLight();
    root->addChild(sunLight);

    // create the sun
    osg::Node* sun = solarSystem.createPlanet( solarSystem._radiusSun, "Sun", osg::Vec4( 1.0, 1.0, 0, 1.0f), "" );
    osg::StateSet* sunStateSet = sun->getOrCreateStateSet();
    osg::Material* material = new osg::Material;
    material->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 0.0f, 0.0f ) );
    sunStateSet->setAttributeAndModes( material, osg::StateAttribute::ON );
    
    osg::Billboard* sunBillboard = new osg::Billboard();
    sunBillboard->setMode(osg::Billboard::POINT_ROT_EYE);
    sunBillboard->addDrawable(
        createSquare(osg::Vec3(-5.0f,0.0f,-5.0f),osg::Vec3(10.0f,0.0f,0.0f),osg::Vec3(0.0f,0.0f,10.0f),createBillboardImage( osg::Vec4( 1.0, 1.0, 0, 1.0f), 64, 1.0) ),
        osg::Vec3(0.0f,0.0f,0.0f));
        
    sunLight->addChild( sunBillboard );
    
    
    // stick sun right under root, no transformations for the sun
    sunLight->addChild( sun );

    // create light source in the sun

    // create earth and moon
    osg::Node* earth = solarSystem.createPlanet( solarSystem._radiusEarth, "Earth", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f), "Images/land_shallow_topo_2048.jpg", "Images/land_ocean_ice_lights_2048.jpg" );
    osg::Node* moon = solarSystem.createPlanet( solarSystem._radiusMoon, "Moon", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f), "Images/moon256128.TGA" );

    // create transformations for the earthMoonGroup
    osg::MatrixTransform* aroundSunRotation = solarSystem.createRotation( solarSystem._RorbitEarth, solarSystem._rotateSpeedEarthAndMoon );
    osg::MatrixTransform* earthPosition = solarSystem.createEarthTranslationAndTilt();


    //Group with earth and moon under it
    osg::Group* earthMoonGroup = new osg::Group;
    

    //transformation to rotate the earth around itself
    osg::MatrixTransform* earthRotationAroundItself = solarSystem.createRotation ( 0.0, solarSystem._rotateSpeedEarth );

    //transformations for the moon
    osg::MatrixTransform* moonAroundEarthXform = solarSystem.createRotation( solarSystem._RorbitMoon, solarSystem._rotateSpeedMoon );
    osg::MatrixTransform* moonTranslation = solarSystem.createMoonTranslation();


    moonTranslation->addChild( moon );
    moonAroundEarthXform->addChild( moonTranslation );
    earthMoonGroup->addChild( moonAroundEarthXform );

    earthRotationAroundItself->addChild( earth );

    earthMoonGroup->addChild( earthRotationAroundItself );

    earthPosition->addChild( earthMoonGroup );

    aroundSunRotation->addChild( earthPosition );

    sunLight->addChild( aroundSunRotation );

#if 0
    // add space, but don't light it, as its not illuminated by our sun
    osg::Node* space = solarSystem.createSpace( "Space", "Images/spacemap.jpg" );
    space->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    root->addChild( space );
#endif
   
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    //optimzer.optimize( dynamic_cast<osg::CoordinateSystemNode*>( root.get() ) );
    optimzer.optimize( root );
     
    // set the scene to render
    viewer.setSceneData( root );


    // set up tracker manipulators, once for each astral body
    {
        FindNamedNodeVisitor fnnv("Sun");
        root->accept(fnnv);

        if (!fnnv._foundNodes.empty())
        {
            // set up the node tracker.
            osgGA::NodeTrackerManipulator* tm = new osgGA::NodeTrackerManipulator;
            tm->setTrackerMode( trackerMode );
            tm->setRotationMode( rotationMode );
            tm->setTrackNode( fnnv._foundNodes.front().get() );

            unsigned int num = viewer.addCameraManipulator( tm );
            viewer.selectCameraManipulator( num );
        }
    }    

    {
        FindNamedNodeVisitor fnnv("Moon");
        root->accept(fnnv);

        if (!fnnv._foundNodes.empty())
        {
            // set up the node tracker.
            osgGA::NodeTrackerManipulator* tm = new osgGA::NodeTrackerManipulator;
            tm->setTrackerMode( trackerMode );
            tm->setRotationMode( rotationMode );
            tm->setTrackNode( fnnv._foundNodes.front().get() );

            unsigned int num = viewer.addCameraManipulator( tm );
            viewer.selectCameraManipulator( num );
        }
    }    

    {
        FindNamedNodeVisitor fnnv("Earth");
        root->accept(fnnv);

        if (!fnnv._foundNodes.empty())
        {
            // set up the node tracker.
            osgGA::NodeTrackerManipulator* tm = new osgGA::NodeTrackerManipulator;
            tm->setTrackerMode( trackerMode );
            tm->setRotationMode( rotationMode );
            tm->setTrackNode( fnnv._foundNodes.front().get() );

            unsigned int num = viewer.addCameraManipulator( tm );
            viewer.selectCameraManipulator( num );
        }
    }    
    
    // create the windows and run the threads.
    viewer.realize();
    
    viewer.setClearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));

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
}// end main


osg::MatrixTransform* SolarSystem::createEarthTranslationAndTilt() 
{
        osg::MatrixTransform* earthPositioned = new osg::MatrixTransform;
        earthPositioned->setMatrix(osg::Matrix::translate(osg::Vec3( 0.0, _RorbitEarth, 0.0 ) )*
                                     osg::Matrix::scale(1.0, 1.0, 1.0)*
                                     osg::Matrix::rotate(osg::inDegrees( _tiltEarth ),0.0f,0.0f,1.0f));

        return earthPositioned;
}// end SolarSystem::createEarthTranslationAndTilt


osg::MatrixTransform* SolarSystem::createRotation( double orbit, double speed )
{
    osg::Vec3 center( 0.0, 0.0, 0.0 );
    float animationLength = 10.0f;
    osg::AnimationPath* animationPath = createAnimationPath( center, orbit, animationLength );

    osg::MatrixTransform* rotation = new osg::MatrixTransform;
    rotation->setUpdateCallback( new osg::AnimationPathCallback( animationPath, 0.0f, speed ) );

    return rotation;
}// end SolarSystem::createEarthRotation


osg::MatrixTransform* SolarSystem::createMoonTranslation()
{
    osg::MatrixTransform* moonPositioned = new osg::MatrixTransform;
    moonPositioned->setMatrix(osg::Matrix::translate(osg::Vec3( 0.0, _RorbitMoon, 0.0 ) )*
                                 osg::Matrix::scale(1.0, 1.0, 1.0)*
                                 osg::Matrix::rotate(osg::inDegrees(0.0f),0.0f,0.0f,1.0f));

    return moonPositioned;
}// end SolarSystem::createMoonTranslation
    
    
osg::Geode* SolarSystem::createSpace( const std::string& name, const std::string& textureName )
{
    osg::Sphere *spaceSphere = new osg::Sphere( osg::Vec3( 0.0, 0.0, 0.0 ), _radiusSpace );

    osg::ShapeDrawable *sSpaceSphere = new osg::ShapeDrawable( spaceSphere );

    if( !textureName.empty() )
    {
        osg::Image* image = osgDB::readImageFile( textureName );
        if ( image )
        {
            sSpaceSphere->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::Texture2D( image ), osg::StateAttribute::ON );

            // reset the object color to white to allow the texture to set the colour.
            sSpaceSphere->setColor( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
        }
    }

    osg::Geode* geodeSpace = new osg::Geode();
    geodeSpace->setName( name );

    geodeSpace->addDrawable( sSpaceSphere );

    return( geodeSpace );

}// end SolarSystem::createSpace

    
osg::Geode* SolarSystem::createPlanet( double radius, const std::string& name, const osg::Vec4& color , const std::string& textureName)
{
    // create a container that makes the sphere drawable
    osg::Geometry *sPlanetSphere = new osg::Geometry();

    {
        // set the single colour so bind overall
        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0] = color;
        sPlanetSphere->setColorArray(colours);
        sPlanetSphere->setColorBinding(osg::Geometry::BIND_OVERALL);


        // now set up the coords, normals and texcoords for geometry 
        unsigned int numX = 100;
        unsigned int numY = 50;
        unsigned int numVertices = numX*numY;

        osg::Vec3Array* coords = new osg::Vec3Array(numVertices);
        sPlanetSphere->setVertexArray(coords);

        osg::Vec3Array* normals = new osg::Vec3Array(numVertices);
        sPlanetSphere->setNormalArray(normals);
        sPlanetSphere->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

        osg::Vec2Array* texcoords = new osg::Vec2Array(numVertices);
        sPlanetSphere->setTexCoordArray(0,texcoords);
        sPlanetSphere->setTexCoordArray(1,texcoords);

        double delta_elevation = osg::PI / (double)(numY-1);
        double delta_azim = 2.0*osg::PI / (double)(numX-1);
        float delta_tx = 1.0 / (float)(numX-1);
        float delta_ty = 1.0 / (float)(numY-1);

        double elevation = -osg::PI*0.5;
        float ty = 0.0;
        unsigned int vert = 0;
        unsigned j;
        for(j=0;
            j<numY;
            ++j, elevation+=delta_elevation, ty+=delta_ty )
        {
            double azim = 0.0;
            float tx = 0.0;
            for(unsigned int i=0;
                i<numX;
                ++i, ++vert, azim+=delta_azim, tx+=delta_tx)
            {
                osg::Vec3 direction(cos(azim)*cos(elevation), sin(azim)*cos(elevation), sin(elevation));
                (*coords)[vert].set(direction*radius);
                (*normals)[vert].set(direction);
                (*texcoords)[vert].set(tx,ty);
            }
        }

        for(j=0;
            j<numY-1;
            ++j)
        {
            unsigned int curr_row = j*numX;
            unsigned int next_row = curr_row+numX;
            osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(GL_QUAD_STRIP);
            for(unsigned int i=0;
                i<numX;
                ++i)
            {
                elements->push_back(next_row + i);
                elements->push_back(curr_row + i);
            }
            sPlanetSphere->addPrimitiveSet(elements);
        }
    }
    

    // set the object color
    //sPlanetSphere->setColor( color );

    // create a geode object to as a container for our drawable sphere object
    osg::Geode* geodePlanet = new osg::Geode();
    geodePlanet->setName( name );

    if( !textureName.empty() )
    {
        osg::Image* image = osgDB::readImageFile( textureName );
        if ( image )
        {
            geodePlanet->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::Texture2D( image ), osg::StateAttribute::ON );

            // reset the object color to white to allow the texture to set the colour.
            //sPlanetSphere->setColor( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
        }
    }

    // add our drawable sphere to the geode container
    geodePlanet->addDrawable( sPlanetSphere );

    return( geodePlanet );

}// end SolarSystem::createPlanet
    
osg::Geode* SolarSystem::createPlanet( double radius, const std::string& name, const osg::Vec4& color , const std::string& textureName1, const std::string& textureName2)
{
    osg::Geode* geodePlanet = createPlanet( radius, name, color , textureName1);
    
    if( !textureName2.empty() )
    {
        osg::Image* image = osgDB::readImageFile( textureName2 );
        if ( image )
        {
            osg::StateSet* stateset = geodePlanet->getOrCreateStateSet();
            
            osg::TexEnvCombine* texenv = new osg::TexEnvCombine;
            
            texenv->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
            texenv->setSource0_RGB(osg::TexEnvCombine::PREVIOUS);
            texenv->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
            texenv->setSource1_RGB(osg::TexEnvCombine::TEXTURE);
            texenv->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
            texenv->setSource2_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
            texenv->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);

            stateset->setTextureAttribute( 1, texenv );
            stateset->setTextureAttributeAndModes( 1, new osg::Texture2D( image ), osg::StateAttribute::ON );
        }
    }

    return( geodePlanet );

}// end SolarSystem::createPlanet

osg::Group* SolarSystem::createSunLight()
{

    osg::LightSource* sunLightSource = new osg::LightSource;

    osg::Light* sunLight = sunLightSource->getLight();
    sunLight->setPosition( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
    sunLight->setAmbient( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

    sunLightSource->setLight( sunLight );
    sunLightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    sunLightSource->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

    osg::LightModel* lightModel = new osg::LightModel;
    lightModel->setAmbientIntensity(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    sunLightSource->getOrCreateStateSet()->setAttribute(lightModel);


    return sunLightSource;
}// end SolarSystem::createSunLight
    

/*
osg::Group* SolarSystem::built()
{
    osg::Group* thisSystem = new osg::Group;

    // create light source in the sun
    osg::Group* sunLight = createSunLight();
    thisSystem->addChild( sunLight );


    // create the sun
    osg::Node* sun = createPlanet( _radiusSun, "Sun", osg::Vec4( 0, 0, 0, 1.0f), "" );
    osg::StateSet* sunStateSet = sun->getOrCreateStateSet();
    osg::Material* material = new osg::Material;
    material->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 0.0f, 0.0f ) );
    sunStateSet->setAttributeAndModes( material, osg::StateAttribute::ON );

    if( !sun )
    {
        std::cout << "Sonne konnte nicht erstellt werden!" << std::endl;
        exit(0);
    }
    sun->setStateSet( sunStateSet );

    // stick sun right under root, no transformations for the sun
    sunLight->addChild(sun);


    //creating right side of the graph with earth and moon and the rotations above it

    // create earth and moon
    osg::Node* earth = createPlanet( _radiusEarth, "Earth", osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f), "Images/land_shallow_topo_2048.jpg" );
    osg::Node* moon = createPlanet( _radiusMoon, "Moon", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f), "Images/moon256128.TGA" );

    // create transformations for the earthMoonGroup
    osg::MatrixTransform* aroundSunRotation = createRotation( _RorbitEarth, _rotateSpeedEarthAndMoon );
    osg::MatrixTransform* earthPosition = createEarthTranslationAndTilt( _RorbitEarth, _tiltEarth );

    //Group with earth and moon under it
    osg::Group* earthMoonGroup = new osg::Group;

    //transformation to rotate the earth around itself
    osg::MatrixTransform* earthRotationAroundItself = createRotation ( 0.0, _rotateSpeedEarth );

    //transformations for the moon
    osg::MatrixTransform* moonAroundEarthXform = createRotation( _RorbitMoon, _rotateSpeedMoon );
    osg::MatrixTransform* moonTranslation = createMoonTranslation( _RorbitMoon );


    moonTranslation->addChild( moon );
    moonAroundEarthXform->addChild( moonTranslation );
    earthMoonGroup->addChild( moonAroundEarthXform );

    earthRotationAroundItself->addChild( earth );

    earthMoonGroup->addChild( earthRotationAroundItself );

    earthPosition->addChild( earthMoonGroup );


    aroundSunRotation->addChild( earthPosition );

    sunLight->addChild( aroundSunRotation );

    // add space, but don't light it, as its not illuminated by our sun
    osg::Node* space = createSpace( _radiusSpace, "Space", "Images/spacemap.jpg" );
    space->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    thisSystem->addChild( space );

    return( thisSystem );
}// end SolarSystem::built()
*/
