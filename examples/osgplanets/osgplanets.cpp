/* details about distances and rotation on http://www.solarviews.com/eng/solarsys.htm */

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
#include <osg/ClearNode>


#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>


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
    
    int numSamples = 1000;
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
    double _radiusSpace;
    double _radiusSun;
    double _radiusMercury;
    double _radiusVenus;
    double _radiusEarth;
    double _radiusMoon;
    double _radiusMars;
    double _radiusJupiter;

    double _RorbitMercury;
    double _RorbitVenus;
    double _RorbitEarth;
    double _RorbitMoon;
    double _RorbitMars;
    double _RorbitJupiter;

    double _rotateSpeedMercury;
    double _rotateSpeedVenus;
    double _rotateSpeedEarthAndMoon;
    double _rotateSpeedEarth;
    double _rotateSpeedMoon;
    double _rotateSpeedMars;
    double _rotateSpeedJupiter;

    double _tiltEarth;

    std::string _mapSpace;
    std::string _mapSun;
    std::string _mapVenus;    
    std::string _mapMercury;    
    std::string _mapEarth;
    std::string _mapEarthNight;
    std::string _mapMoon;
    std::string _mapMars;
    std::string _mapJupiter;
    
    SolarSystem()
    {
        _radiusSpace    = 300.0;
        _radiusSun      = 3.5;
        _radiusMercury  = 0.7;
        _radiusVenus    = 1.2;
        _radiusEarth    = 2.0;
        _radiusMoon     = 0.5;
        _radiusMars     = 1.8;
        _radiusJupiter  = 1.8;
        
        _RorbitMercury  = 11.7;
        _RorbitVenus    = 21.6;
        _RorbitEarth    = 30.0;
        _RorbitMoon     = 2.0;
        _RorbitMars     = 45.0;
        _RorbitJupiter  = 156.0;
        
        _rotateSpeedMercury         = 1.1;
        _rotateSpeedVenus           = 1.3;
        _rotateSpeedEarthAndMoon    = 1.0;
        _rotateSpeedEarth           = 1.0;
        _rotateSpeedMoon            = 1.2;
        _rotateSpeedMars            = 1.2;
        _rotateSpeedJupiter         = 1.2;

        _tiltEarth                  = 18.0; // degrees
        
        _mapSpace       = "Images/spacemap2.jpg";
        _mapSun         = "SolarSystem/sun256128.jpg";
        _mapMercury     = "SolarSystem/mercury256128.jpg";
        _mapVenus       = "SolarSystem/venus256128.jpg";
        _mapEarth       = "Images/land_shallow_topo_2048.jpg";
        _mapEarthNight  = "Images/land_ocean_ice_lights_2048.jpg";
        _mapMoon        = "SolarSystem/moon256128.jpg";
        _mapMars        = "SolarSystem/mars256128.jpg";
        _mapJupiter     = "SolarSystem/jupiter256128.jpg";
    }
    
    osg::MatrixTransform* createTranslationAndTilt( double translation, double tilt );
    osg::MatrixTransform* createRotation( double orbit, double speed );
    
    osg::Geode* createSpace( const std::string& name, const std::string& textureName );
    osg::Geode* createPlanet( double radius, const std::string& name, const osg::Vec4& color , const std::string& textureName );
    osg::Geode* createPlanet( double radius, const std::string& name, const osg::Vec4& color , const std::string& textureName1, const std::string& textureName2);
    osg::Group* createSunLight();
    
    void printParameters();
        
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


osg::MatrixTransform* SolarSystem::createRotation( double orbit, double speed )
{
    osg::Vec3 center( 0.0, 0.0, 0.0 );
    float animationLength = 10.0f;
    osg::AnimationPath* animationPath = createAnimationPath( center, orbit, animationLength );

    osg::MatrixTransform* rotation = new osg::MatrixTransform;
    rotation->setUpdateCallback( new osg::AnimationPathCallback( animationPath, 0.0f, speed ) );

    return rotation;
}// end SolarSystem::createEarthRotation


osg::MatrixTransform* SolarSystem::createTranslationAndTilt( double /*translation*/, double tilt )
{
    osg::MatrixTransform* moonPositioned = new osg::MatrixTransform;
    moonPositioned->setMatrix(osg::Matrix::translate(osg::Vec3( 0.0, _RorbitMoon, 0.0 ) )*
                                 osg::Matrix::scale(1.0, 1.0, 1.0)*
                                 osg::Matrix::rotate(osg::inDegrees( tilt ),0.0f,0.0f,1.0f));

    return moonPositioned;
}// end SolarSystem::createTranslationAndTilt
    
    
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
    
void SolarSystem::printParameters()
{
    std::cout << "radiusSpace(" << _radiusSpace << ")" << std::endl;
    std::cout << "radiusSun(" << _radiusSun << ")" << std::endl;
    std::cout << "radiusEarth(" << _radiusEarth << ")" << std::endl;
    std::cout << "radiusMoon(" << _radiusMoon << ")" << std::endl;

    std::cout << "RorbitEarth(" << _RorbitEarth << ")" << std::endl;
    std::cout << "RorbitMoon(" << _RorbitMoon << ")" << std::endl;

    std::cout << "rotateSpeedEarthAndMoon(" << _rotateSpeedEarthAndMoon << ")" << std::endl;
    std::cout << "rotateSpeedEarth(" << _rotateSpeedEarth << ")" << std::endl;
    std::cout << "rotateSpeedMoon(" << _rotateSpeedMoon << ")" << std::endl;
    std::cout << "tiltEarth(" << _tiltEarth << ")" << std::endl;

    std::cout << "mapSpace(" << _radiusSpace << ")" << std::endl;
    std::cout << "mapEarth(" << _radiusSpace << ")" << std::endl;
    std::cout << "mapEarthNight(" << _radiusSpace << ")" << std::endl;
    std::cout << "mapMoon(" << _radiusSpace << ")" << std::endl;
}


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of osg::AnimationPath and UpdateCallbacks for adding animation to your scenes.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-o <filename>","Write created model to file");

    // initialize the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE | osgProducer::Viewer::VIEWER_MANIPULATOR | osgProducer::Viewer::STATE_MANIPULATOR);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    SolarSystem solarSystem;

    while (arguments.read("--radiusSpace",solarSystem._radiusSpace)) { }
    while (arguments.read("--radiusSun",solarSystem._radiusSun)) { }
    while (arguments.read("--radiusEarth",solarSystem._radiusEarth)) { }
    while (arguments.read("--radiusMoon",solarSystem._radiusMoon)) { }
    
    while (arguments.read("--RorbitEarth",solarSystem._RorbitEarth)) { }
    while (arguments.read("--RorbitMoon",solarSystem._RorbitMoon)) { }
    
    while (arguments.read("--rotateSpeedEarthAndMoon",solarSystem._rotateSpeedEarthAndMoon)) { }
    while (arguments.read("--rotateSpeedEarth",solarSystem._rotateSpeedEarth)) { }
    while (arguments.read("--rotateSpeedMoon",solarSystem._rotateSpeedMoon)) { }
    while (arguments.read("--tiltEarth",solarSystem._tiltEarth)) { }
    
    while (arguments.read("--mapSpace",solarSystem._mapSpace)) { }
    while (arguments.read("--mapEarth",solarSystem._mapEarth)) { }
    while (arguments.read("--mapEarthNight",solarSystem._mapEarthNight)) { }
    while (arguments.read("--mapMoon",solarSystem._mapMoon)) { }
    
    std::string writeFileName;
    while (arguments.read("-o",writeFileName)) { }
    
    
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
        std::cout << "--radiusSpace: double" << std::endl;
        std::cout << "--radiusSun: double" << std::endl;
        std::cout << "--radiusEarth: double" << std::endl;
        std::cout << "--radiusMoon: double" << std::endl;
        
        std::cout << "--RorbitEarth: double" << std::endl;
        std::cout << "--RorbitMoon: double" << std::endl;
        
        std::cout << "--rotateSpeedEarthAndMoon: double" << std::endl;
        std::cout << "--rotateSpeedEarth: double" << std::endl;
        std::cout << "--rotateSpeedMoon: double" << std::endl;
        std::cout << "--tiltEarth: double" << std::endl;
        
        std::cout << "--mapSpace: string" << std::endl;
        std::cout << "--mapEarth: string" << std::endl;
        std::cout << "--mapEarthNight: string" << std::endl;
        std::cout << "--mapMoon: string" << std::endl;
                        
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
    
    osg::ClearNode* clearNode = new osg::ClearNode;
    clearNode->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    root->addChild(clearNode);

    osg::Group* sunLight = solarSystem.createSunLight();
    root->addChild(sunLight);

    // create the sun
    osg::Node* solarSun = solarSystem.createPlanet( solarSystem._radiusSun, "Sun", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f), solarSystem._mapSun );
    osg::StateSet* sunStateSet = solarSun->getOrCreateStateSet();
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
    sunLight->addChild( solarSun );

    // create light source in the sun

/*
*********************************************
**  earthMoonGroup and Transformations 
*********************************************
*/
    // create earth and moon
    osg::Node* earth = solarSystem.createPlanet( solarSystem._radiusEarth, "Earth", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f), solarSystem._mapEarth, solarSystem._mapEarthNight );
    osg::Node* moon = solarSystem.createPlanet( solarSystem._radiusMoon, "Moon", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f), solarSystem._mapMoon );

    // create transformations for the earthMoonGroup
    osg::MatrixTransform* aroundSunRotationEarthMoonGroup = solarSystem.createRotation( solarSystem._RorbitEarth, solarSystem._rotateSpeedEarthAndMoon );
    osg::MatrixTransform* earthMoonGroupPosition = solarSystem.createTranslationAndTilt( solarSystem._RorbitEarth, solarSystem._tiltEarth );


    //Group with earth and moon under it
    osg::Group* earthMoonGroup = new osg::Group;
    

    //transformation to rotate the earth around itself
    osg::MatrixTransform* earthAroundItselfRotation = solarSystem.createRotation ( 0.0, solarSystem._rotateSpeedEarth );

    //transformations for the moon
    osg::MatrixTransform* moonAroundEarthRotation = solarSystem.createRotation( solarSystem._RorbitMoon, solarSystem._rotateSpeedMoon );
    osg::MatrixTransform* moonTranslation = solarSystem.createTranslationAndTilt( solarSystem._RorbitMoon, 0.0f );


    moonTranslation->addChild( moon );
    moonAroundEarthRotation->addChild( moonTranslation );
    earthMoonGroup->addChild( moonAroundEarthRotation );

    earthAroundItselfRotation->addChild( earth );

    earthMoonGroup->addChild( earthAroundItselfRotation );

    earthMoonGroupPosition->addChild( earthMoonGroup );

    aroundSunRotationEarthMoonGroup->addChild( earthMoonGroupPosition );

    sunLight->addChild( aroundSunRotationEarthMoonGroup );
/*
*********************************************
**  end earthMoonGroup and Transformations 
*********************************************
*/

/*
*********************************************
**  Mercury and Transformations 
*********************************************
*/
    osg::Node* mercury = solarSystem.createPlanet( solarSystem._radiusMercury, "Mercury", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ), solarSystem._mapMercury, "" );
    
    osg::MatrixTransform* aroundSunRotationMercury = solarSystem.createRotation( solarSystem._RorbitMercury, solarSystem._rotateSpeedMercury );
    osg::MatrixTransform* mercuryPosition = solarSystem.createTranslationAndTilt( solarSystem._RorbitMercury, 0.0f );
    
    mercuryPosition->addChild( mercury );
    aroundSunRotationMercury->addChild( mercuryPosition );
    
    sunLight->addChild( aroundSunRotationMercury );
/*
*********************************************
**  end Mercury and Transformations 
*********************************************
*/

/*
*********************************************
**  Venus and Transformations 
*********************************************
*/
    osg::Node* venus = solarSystem.createPlanet( solarSystem._radiusVenus, "Venus", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ), solarSystem._mapVenus, "" );
    
    osg::MatrixTransform* aroundSunRotationVenus = solarSystem.createRotation( solarSystem._RorbitVenus, solarSystem._rotateSpeedVenus );
    osg::MatrixTransform* venusPosition = solarSystem.createTranslationAndTilt( solarSystem._RorbitVenus, 0.0f );
    
    venusPosition->addChild( venus );
    aroundSunRotationVenus->addChild( venusPosition );
    
    sunLight->addChild( aroundSunRotationVenus );
/*
*********************************************
**  end Venus and Transformations 
*********************************************
*/

/*
*********************************************
**  Mars and Transformations 
*********************************************
*/
    osg::Node* mars = solarSystem.createPlanet( solarSystem._radiusMars, "Mars", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ), solarSystem._mapMars, "" );
    
    osg::MatrixTransform* aroundSunRotationMars = solarSystem.createRotation( solarSystem._RorbitMars, solarSystem._rotateSpeedMars );
    osg::MatrixTransform* marsPosition = solarSystem.createTranslationAndTilt( solarSystem._RorbitMars, 0.0f );
    
    marsPosition->addChild( mars );
    aroundSunRotationMars->addChild( marsPosition );
    
    sunLight->addChild( aroundSunRotationMars );
/*
*********************************************
**  end Mars and Transformations 
*********************************************
*/

/*
*********************************************
**  Jupiter and Transformations 
*********************************************
*/
    osg::Node* jupiter = solarSystem.createPlanet( solarSystem._radiusJupiter, "Jupiter", osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ), solarSystem._mapJupiter, "" );
    
    osg::MatrixTransform* aroundSunRotationJupiter = solarSystem.createRotation( solarSystem._RorbitJupiter, solarSystem._rotateSpeedJupiter );
    osg::MatrixTransform* jupiterPosition = solarSystem.createTranslationAndTilt( solarSystem._RorbitJupiter, 0.0f );
    
    jupiterPosition->addChild( jupiter );
    aroundSunRotationJupiter->addChild( jupiterPosition );
    
    sunLight->addChild( aroundSunRotationJupiter );
/*
*********************************************
**  end Jupiter and Transformations 
*********************************************
*/

/*
    // add space, but don't light it, as its not illuminated by our sun
    osg::Node* space = solarSystem.createSpace( "Space", solarSystem._mapSpace );
    space->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    root->addChild( space );
*/    
     
    if (!writeFileName.empty())
    {
        osgDB::writeNodeFile(*root, writeFileName);
        std::cout<<"Written solar system to \""<<writeFileName<<"\""<<std::endl;
        return 0;
    }


    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize( root );
     
    // set the scene to render
    viewer.setSceneData( root );


    // set up tracker manipulators, once for each astral body
    
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
}// end main

