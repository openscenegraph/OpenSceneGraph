/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgUtil/CullVisitor>
#include <osgProducer/Viewer>

#include <osgParticle/PrecipitationEffect>

#if 0
osg::Node* createModel(osg::Node* loadedModel, osgParticle::PrecipitationParameters& parameters)
{
    osg::Group* group = new osg::Group;

    osg::BoundingBox bb(0.0, 0.0, 0.0, 100.0, 100.0, 100.0);
    
    if (loadedModel)
    {
        group->addChild(loadedModel);
        
        osg::BoundingSphere bs = loadedModel->getBound();

        bb.set( -500, -500, 0, +500, +500, 10);
        
        parameters.boundingBox = bb;
        
        osg::StateSet* stateset = loadedModel->getOrCreateStateSet();
        
        osg::Fog* fog = new osg::Fog;
        
        if (parameters.fogExponent<1.0)
        {
            fog->setMode(osg::Fog::LINEAR);
        }
        else if (parameters.fogExponent<2.0)
        {
            fog->setMode(osg::Fog::EXP);
        }
        else
        {
            fog->setMode(osg::Fog::EXP2);
        }
        
        fog->setDensity(parameters.fogDensity);
        fog->setStart(0.0f);
        fog->setEnd(parameters.fogEnd);
        fog->setColor(parameters.fogColour);
        stateset->setAttributeAndModes(fog, osg::StateAttribute::ON);
        
        osg::LightSource* lightSource = new osg::LightSource;
        group->addChild(lightSource);

        osg::Light* light = lightSource->getLight();
        light->setLightNum(0);
        light->setPosition(osg::Vec4(0.0f,0.0f,1.0f,0.0f)); // directional light from above
        light->setAmbient(osg::Vec4(0.8f,0.8f,0.8f,1.0f));
        light->setDiffuse(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
        light->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));

                
    }
    
    group->addChild(createCellRainEffect(parameters));

    return group;    
}
#endif

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" example provides an interactive viewer for visualising point clouds..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    arguments.getApplicationUsage()->addCommandLineOption("","");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    osg::ref_ptr<osgParticle::PrecipitationEffect> precipitationEffect = new osgParticle::PrecipitationEffect;
    osgParticle::PrecipitationParameters& parameters = *precipitationEffect->getParameters();

    float intensity;
    while (arguments.read("--snow", intensity)) parameters.snow(intensity);
    while (arguments.read("--rain", intensity)) parameters.rain(intensity);

    while (arguments.read("--particleSize", parameters.particleSize)) {}
    while (arguments.read("--particleColor", parameters.particleColour.r(), parameters.particleColour.g(), parameters.particleColour.b(), parameters.particleColour.a())) {}
    while (arguments.read("--particleColour", parameters.particleColour.r(), parameters.particleColour.g(), parameters.particleColour.b(), parameters.particleColour.a())) {}

    osg::Vec3 particleVelocity;
    while (arguments.read("--particleVelocity", particleVelocity.x(), particleVelocity.y(), particleVelocity.z() )) parameters.particleVelocity = particleVelocity;

    while (arguments.read("--nearTransition", parameters.nearTransition )) {}
    while (arguments.read("--farTransition", parameters.farTransition )) {}

    while (arguments.read("--particleDensity", parameters.particleDensity )) {}

    while (arguments.read("--cellSizeX", parameters.cellSizeX )) {}
    while (arguments.read("--cellSizeY", parameters.cellSizeY )) {}
    while (arguments.read("--cellSizeZ", parameters.cellSizeZ )) {}

    while (arguments.read("--boundingBox", parameters.boundingBox.xMin(),
                                           parameters.boundingBox.yMin(),
                                           parameters.boundingBox.zMin(),
                                           parameters.boundingBox.xMax(),
                                           parameters.boundingBox.yMax(),
                                           parameters.boundingBox.zMax())) {}

    while (arguments.read("--fogDensity", parameters.fogDensity )) {}
    while (arguments.read("--fogExponent", parameters.fogExponent )) {}
    while (arguments.read("--fogEnd", parameters.fogEnd )) {}
    while (arguments.read("--fogColor", parameters.fogColour.r(), parameters.fogColour.g(), parameters.fogColour.b(), parameters.fogColour.a())) {}
    while (arguments.read("--fogColour", parameters.fogColour.r(), parameters.fogColour.g(), parameters.fogColour.b(), parameters.fogColour.a())) {}
 
    while (arguments.read("--useFarLineSegments")) { parameters.useFarLineSegments = true; }

    


    viewer.setClearColor(parameters.clearColour);

    // now force the effect to update all its internal state.
    precipitationEffect->update();


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
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
    
#if 1
    
    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(precipitationEffect.get());
    group->addChild(loadedModel.get());
    
    // set the scene to render
    viewer.setSceneData(group.get());

#else    

        loadedModel = createModel(loadedModel.get(), parameters);

        // set the scene to render
        viewer.setSceneData(loadedModel.get());
#endif

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

