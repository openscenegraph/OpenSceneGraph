/* OpenSceneGraph example, osgprecipitation.
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

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgUtil/CullVisitor>
#include <osg/ClipNode>


#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgViewer/Viewer>

#include <osg/MatrixTransform>
#include <osgUtil/TransformCallback>
#include <osgParticle/PrecipitationEffect>

#include <iostream>

class MyGustCallback : public osg::NodeCallback
{

    public:

        MyGustCallback() {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osgParticle::PrecipitationEffect* pe = dynamic_cast<osgParticle::PrecipitationEffect*>(node);
            
            float value = sin(nv->getFrameStamp()->getSimulationTime());
            if (value<-0.5)
            {
                pe->snow(1.0);
            }
            else
            {
                pe->rain(0.5);
            }
        
            traverse(node, nv);
        }
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example provides an interactive viewer for visualising point clouds..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--snow <density>","Set the snow with a density between 0 and 1.0");
    arguments.getApplicationUsage()->addCommandLineOption("--rain <density>","");
    arguments.getApplicationUsage()->addCommandLineOption("--particleSize <size>","");
    arguments.getApplicationUsage()->addCommandLineOption("--particleColour <red> <green> <blue> <alpha>","");
    arguments.getApplicationUsage()->addCommandLineOption("--wind <x> <y> <z>","Set the wind speed in model coordinates");
    arguments.getApplicationUsage()->addCommandLineOption("--particleSpeed <float>","Set the particle speed");
    arguments.getApplicationUsage()->addCommandLineOption("--nearTransition <distance>","Set the near transistion distance");
    arguments.getApplicationUsage()->addCommandLineOption("--farTransition  <distance>","Set the far transistion distance");
    arguments.getApplicationUsage()->addCommandLineOption("--particleDensity <density>","Set the particle density");
    arguments.getApplicationUsage()->addCommandLineOption("--cellSize <x> <y> <z>","Set the cell size in model coordinates");
    arguments.getApplicationUsage()->addCommandLineOption("--fogDensity <density>","Set the fog density");
    arguments.getApplicationUsage()->addCommandLineOption("--fogColour <red> <green> <blue> <alpha>","Set fog colour.");
    arguments.getApplicationUsage()->addCommandLineOption("-useFarLineSegments","Switch on the use of line segments");
    

    // construct the viewer.
    osgViewer::Viewer viewer;

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        std::string pathfile;
        char keyForAnimationPath = '5';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid()) 
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;

    float intensity;
    while (arguments.read("--snow", intensity)) precipitationEffect->snow(intensity);
    while (arguments.read("--rain", intensity)) precipitationEffect->rain(intensity);

    float value;
    while (arguments.read("--particleSize", value)) precipitationEffect->setParticleSize(value);

    osg::Vec4 color;
    while (arguments.read("--particleColor", color.r(), color.g(), color.b(), color.a())) precipitationEffect->setParticleColor(color);
    while (arguments.read("--particleColour", color.r(), color.g(), color.b(), color.a())) precipitationEffect->setParticleColor(color);

    osg::Vec3 wind;
    while (arguments.read("--wind", wind.x(), wind.y(), wind.z())) precipitationEffect->setWind(wind);
    
    while (arguments.read("--particleSpeed", value)) precipitationEffect->setParticleSpeed(value);

    while (arguments.read("--nearTransition", value )) precipitationEffect->setNearTransition(value);
    while (arguments.read("--farTransition", value )) precipitationEffect->setFarTransition(value);

    while (arguments.read("--particleDensity", value )) precipitationEffect->setMaximumParticleDensity(value);

    osg::Vec3 cellSize;
    while (arguments.read("--cellSize", cellSize.x(), cellSize.y(), cellSize.z())) precipitationEffect->setCellSize(cellSize); 

    double clipDistance = 0.0;
    while (arguments.read("--clip",clipDistance)) {}

    osg::BoundingBox bb;
    while (arguments.read("--boundingBox", bb.xMin(),
                                           bb.yMin(),
                                           bb.zMin(),
                                           bb.xMax(),
                                           bb.yMax(),
                                           bb.zMax())) {}

    while (arguments.read("--fogDensity", value )) precipitationEffect->getFog()->setDensity(value);
    while (arguments.read("--fogColor", color.r(), color.g(), color.b(), color.a() ))  precipitationEffect->getFog()->setColor(color);
    while (arguments.read("--fogColour", color.r(), color.g(), color.b(), color.a() ))  precipitationEffect->getFog()->setColor(color);
 
    while (arguments.read("--useFarLineSegments")) { precipitationEffect->setUseFarLineSegments(true); }

    
    viewer.getCamera()->setClearColor( precipitationEffect->getFog()->getColor() );


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    
    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
    
    // precipitationEffect->setUpdateCallback(new MyGustCallback);
    
    osg::ref_ptr<osg::Group> group = new osg::Group;
    
    if (clipDistance!=0.0)
    {    
        osg::ref_ptr<osg::ClipNode> clipNode = new osg::ClipNode;
        clipNode->addClipPlane( new osg::ClipPlane( 0 ) );
        clipNode->getClipPlane(0)->setClipPlane( 0.0, 0.0, -1.0, -clipDistance );
        clipNode->setReferenceFrame(osg::ClipNode::ABSOLUTE_RF);
        clipNode->addChild(precipitationEffect.get());

        group->addChild(clipNode.get());
    }
    else
    {
        group->addChild(precipitationEffect.get());
    }
    
    group->addChild(loadedModel.get());

    loadedModel->getOrCreateStateSet()->setAttributeAndModes(precipitationEffect->getFog());
    
    // create the light    
    osg::LightSource* lightSource = new osg::LightSource;
    group->addChild(lightSource);

    osg::Light* light = lightSource->getLight();
    light->setLightNum(0);
    light->setPosition(osg::Vec4(0.0f,0.0f,1.0f,0.0f)); // directional light from above
    light->setAmbient(osg::Vec4(0.8f,0.8f,0.8f,1.0f));
    light->setDiffuse(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    light->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));


    // set the scene to render
    viewer.setSceneData(group.get());

    return viewer.run();
}

