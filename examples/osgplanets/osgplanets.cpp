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


#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgProducer/Viewer>


static osg::Vec3 defaultPos( 0.0f, 0.0f, 0.0f );
static osg::Vec3 centerScope(0.0f, 0.0f, 0.0f);


osg::Group* createSunLight()
{
    osg::Group* lightNode = new osg::Group;

    osg::Light* sunLight = new osg::Light;
    sunLight->setPosition( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
    sunLight->setAmbient( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );

    osg::LightSource* sunLightSource = new osg::LightSource;
    sunLightSource->setLight( sunLight );
    
    lightNode->addChild( sunLightSource );
    
    osg::LightModel* lightModel = new osg::LightModel;
    lightModel->setAmbientIntensity(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    lightNode->getOrCreateStateSet()->setAttribute(lightModel);


    return lightNode;        
}// end createSunLight


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


osg::MatrixTransform* createEarthTranslationAndTilt( double RorbitEarth, double tiltEarth ) 
{
        osg::MatrixTransform* earthPositioned = new osg::MatrixTransform;
        //earthPositioned->setDataVariance(osg::Object::STATIC);
        earthPositioned->setMatrix(osg::Matrix::translate(osg::Vec3( 0.0, RorbitEarth, 0.0 ) )*
                                     osg::Matrix::scale(1.0, 1.0, 1.0)*
                                     osg::Matrix::rotate(osg::inDegrees( tiltEarth ),0.0f,0.0f,1.0f));
                                     
        return earthPositioned;
}// end createEarthTranslationAndTilt


osg::MatrixTransform* createRotation( double orbit, double speed )
{
    osg::Vec3 center( 0.0, 0.0, 0.0 );
    float animationLength = 10.0f;
    osg::AnimationPath* animationPath = createAnimationPath( center, orbit, animationLength );
    
    osg::MatrixTransform* rotation = new osg::MatrixTransform;
    rotation->setUpdateCallback( new osg::AnimationPathCallback( animationPath, 0.0f, speed ) );
    
    return rotation;
}// end createEarthRotation


osg::MatrixTransform* createMoonTranslation( double RorbitMoon )
{
    osg::MatrixTransform* moonPositioned = new osg::MatrixTransform;
    //earthPositioned->setDataVariance(osg::Object::STATIC);
    //earthPositioned->setMatrix(osg::Matrix::translate(osg::Vec3( RorbitEarth, 0.0, 0.0 ) )*
    moonPositioned->setMatrix(osg::Matrix::translate(osg::Vec3( 0.0, RorbitMoon, 0.0 ) )*
    //earthPositioned->setMatrix(osg::Matrix::translate(osg::Vec3( 0.0, 0.0, RorbitEarth ) )*
                                 osg::Matrix::scale(1.0, 1.0, 1.0)*
                                 osg::Matrix::rotate(osg::inDegrees(0.0f),0.0f,0.0f,1.0f));

    return moonPositioned;
}// end createMoonTranslation


osg::Geode* createSpace( double radius, const std::string& name, const std::string& textureName )
{
    osg::Sphere *spaceSphere = new osg::Sphere( osg::Vec3( 0.0, 0.0, 0.0 ), radius );
    
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
    
}// end createSpace


osg::Geode* createPlanet( double radius, const std::string& name, const osg::Vec4& color , const std::string& textureName )
{
    // create a cube shape
    osg::Sphere *planetSphere = new osg::Sphere( osg::Vec3( 0.0, 0.0, 0.0 ), radius );

    // create a container that makes the sphere drawable
    osg::ShapeDrawable *sPlanetSphere = new osg::ShapeDrawable( planetSphere );

    // set the object color
    sPlanetSphere->setColor( color );

    if( !textureName.empty() )
    {
        osg::Image* image = osgDB::readImageFile( textureName );
        if ( image )
        {
            sPlanetSphere->getOrCreateStateSet()->setTextureAttributeAndModes( 0, new osg::Texture2D( image ), osg::StateAttribute::ON );

            // reset the object color to white to allow the texture to set the colour.
            sPlanetSphere->setColor( osg::Vec4(1.0f,1.0f,1.0f,1.0f) );
        }
    }
 
   
    // create a geode object to as a container for our drawable sphere object
    osg::Geode* geodePlanet = new osg::Geode();
    geodePlanet->setName( name );
    
    // add our drawable sphere to the geode container
    geodePlanet->addDrawable( sPlanetSphere );

    return( geodePlanet );
    
}// end createPlanet


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
    
    osg::Group* built()
    {
        osg::Group* thisSystem = new osg::Group;
        
        osg::StateSet* sunStateSet = new osg::StateSet;
        osg::Material* material = new osg::Material;
        material->setEmission( osg::Material::FRONT_AND_BACK, osg::Vec4( 1.0f, 1.0f, 0.0f, 0.0f ) );
        sunStateSet->setAttributeAndModes( material, osg::StateAttribute::ON );
        
        
        
        // create the sun
        osg::Node* sun = createPlanet( _radiusSun, "Sun", osg::Vec4( 0, 0, 0, 1.0f), "" );
        sun->setStateSet( sunStateSet );
        
        // stick sun right under root, no transformations for the sun
        thisSystem->addChild( sun );
        
        
        // create light source in the sun
        osg::Group* sunLight = createSunLight();
        
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
        
        thisSystem->addChild( sunLight );

#if 0
        // add space, but don't light it, as its not illuminated by our sun
        osg::Node* space = createSpace( _radiusSpace, "Space", "Images/spacemap1.jpg" );
        space->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        thisSystem->addChild( space );
#endif        
                
        return( thisSystem );
    }
    
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
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

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
    
    
    osg::Group* root = solarSystem.built();
        
    /*
    // tilt the scene so the default eye position is looking down on the model.
    osg::MatrixTransform* rootnode = new osg::MatrixTransform;
    rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(30.0f),1.0f,0.0f,0.0f));
    rootnode->addChild(model);
    */

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( root );
     
    // set the scene to render
    viewer.setSceneData( root );

    // create the windows and run the threads.
    viewer.realize();
    
    osg::Matrix lookAt;
    lookAt.makeLookAt( osg::Vec3(0.0f, -80.0f, 0.0f), centerScope, osg::Vec3(0.0f, 0.0f, 1.0f ) );

    viewer.setView( lookAt );
    
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
}
