#include <osg/GL>
#include <osgProducer/Viewer>

#include <osg/MatrixTransform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/ShapeDrawable>
#include <osg/Notify>
#include <osg/PointSprite>
#include <osg/Texture2D>
#include <osg/BlendFunc>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>

#include <osgSim/LightPointNode>



#define INTERPOLATE(member) lp.member = start.member*rstart + end.member*rend;

void addToLightPointNode(osgSim::LightPointNode& lpn,osgSim::LightPoint& start,osgSim::LightPoint& end,unsigned int noSteps)
{
    if (noSteps<=1)
    {
        lpn.addLightPoint(start);
        return;
    }
    
    float rend = 0.0f;
    float rdelta = 1.0f/((float)noSteps-1.0f);
    
    lpn.getLightPointList().reserve(noSteps);
    
    for(unsigned int i=0;i<noSteps;++i,rend+=rdelta)
    {
        float rstart = 1.0f-rend;
        osgSim::LightPoint lp(start);
        INTERPOLATE(_position)
        INTERPOLATE(_intensity);
        INTERPOLATE(_color);
        INTERPOLATE(_radius);

        lpn.addLightPoint(lp);
        
   }
}

#undef INTERPOLATE

bool usePointSprites;

osg::Node* createLightPointsDatabase()
{
    osgSim::LightPoint start;
    osgSim::LightPoint end;

    start._position.set(-500.0f,-500.0f,0.0f);
    start._color.set(1.0f,0.0f,0.0f,1.0f);
    
    end._position.set(500.0f,-500.0f,0.0f);
    end._color.set(1.0f,1.0f,1.0f,1.0f);
    
    osg::MatrixTransform* transform = new osg::MatrixTransform;
    
    transform->setDataVariance(osg::Object::STATIC);
    transform->setMatrix(osg::Matrix::scale(0.1,0.1,0.1));

    osg::Vec3 start_delta(0.0f,10.0f,0.0f);
    osg::Vec3 end_delta(0.0f,10.0f,1.0f);

    int noStepsX = 100;
    int noStepsY = 100;

//     osgSim::BlinkSequence* bs = new osgSim::BlinkSequence;
//     bs->addPulse(1.0,osg::Vec4(1.0f,0.0f,0.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
//     bs->addPulse(1.5,osg::Vec4(1.0f,1.0f,0.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
//     bs->addPulse(1.0,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
//     bs->addPulse(0.5,osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // off
    

//    osgSim::Sector* sector = new osgSim::ConeSector(osg::Vec3(0.0f,0.0f,1.0f),osg::inDegrees(45.0),osg::inDegrees(45.0));
//    osgSim::Sector* sector = new osgSim::ElevationSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(45.0));
//    osgSim::Sector* sector = new osgSim::AzimSector(-osg::inDegrees(45.0),osg::inDegrees(45.0),osg::inDegrees(90.0));
//     osgSim::Sector* sector = new osgSim::AzimElevationSector(osg::inDegrees(180),osg::inDegrees(90), // azim range
//                                                                 osg::inDegrees(0.0),osg::inDegrees(90.0), // elevation range
//                                                                 osg::inDegrees(5.0));

    for(int i=0;i<noStepsY;++i)
    {

//         osgSim::BlinkSequence* local_bs = new osgSim::BlinkSequence(*bs);
//         local_bs->setSequenceGroup(new osgSim::BlinkSequence::SequenceGroup((double)i*0.1));        
//         start._blinkSequence = local_bs;

//        start._sector = sector;

        osgSim::LightPointNode* lpn = new osgSim::LightPointNode;

        //
        osg::StateSet* set = lpn->getOrCreateStateSet();

        if (usePointSprites)
        {
            lpn->setPointSprite();

            // Set point sprite texture in LightPointNode StateSet.
            osg::Texture2D *tex = new osg::Texture2D();
            tex->setImage(osgDB::readImageFile("Images/particle.rgb"));
            set->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
        }

        //set->setMode(GL_BLEND, osg::StateAttribute::ON);
        //osg::BlendFunc *fn = new osg::BlendFunc();
        //fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::DST_ALPHA);
        //set->setAttributeAndModes(fn, osg::StateAttribute::ON);
        //

        addToLightPointNode(*lpn,start,end,noStepsX);
        
        start._position += start_delta;
        end._position += end_delta;
        
        transform->addChild(lpn);    
    }
        
    osg::Group* group = new osg::Group;
    group->addChild(transform);
    
    
    return group;
}

static osg::Node* CreateBlinkSequenceLightNode()
{
   osgSim::LightPointNode*      lightPointNode = new osgSim::LightPointNode;;

   osgSim::LightPointNode::LightPointList       lpList;

   osg::ref_ptr<osgSim::SequenceGroup>   seq_0;
   seq_0 = new osgSim::SequenceGroup;
   seq_0->_baseTime = 0.0;

   osg::ref_ptr<osgSim::SequenceGroup>   seq_1;
   seq_1 = new osgSim::SequenceGroup;
   seq_1->_baseTime = 0.5;

   const int max_points = 32;
   for( int i = 0; i < max_points; ++i )
   {
      osgSim::LightPoint   lp;
      double x = cos( (2.0*osg::PI*i)/max_points );
      double z = sin( (2.0*osg::PI*i)/max_points );
      lp._position.set( x, 0.0f, z + 30.0f );
      lp._blinkSequence = new osgSim::BlinkSequence;
      for( int j = 10; j > 0; --j )
      {
         float  intensity = j/10.0f;
         lp._blinkSequence->addPulse( 1.0/max_points,
                                     osg::Vec4( intensity, intensity, intensity, intensity ) );
      }
      if( max_points > 10 )
      {
         lp._blinkSequence->addPulse( 1.0 - 10.0/max_points,
                                     osg::Vec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
      }

      if( i & 1 )
      {
         lp._blinkSequence->setSequenceGroup( seq_1.get() );
      }
      else
      {
         lp._blinkSequence->setSequenceGroup( seq_0.get() );
      }
      lp._blinkSequence->setPhaseShift( i/(static_cast<double>(max_points)) );
      lpList.push_back( lp );
   }

   lightPointNode->setLightPointList( lpList );

   return lightPointNode;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use high quality light point, typically used for naviagional lights.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--sprites","Point sprites.");

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

    usePointSprites = false;
    while (arguments.read("--sprites")) { usePointSprites = true; };

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    osg::Group* rootnode = new osg::Group;

    // load the nodes from the commandline arguments.
    rootnode->addChild(osgDB::readNodeFiles(arguments));
    rootnode->addChild(createLightPointsDatabase());
    rootnode->addChild(CreateBlinkSequenceLightNode());
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    
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
