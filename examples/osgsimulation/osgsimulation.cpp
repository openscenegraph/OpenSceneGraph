#include <osgProducer/Viewer>

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>

#include <osgDB/ReadFile>

#include <osgSim/SphereSegment>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/ParticleSystemUpdater>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

osg::Vec3 computeTerrainIntersection(osg::Node* subgraph,float x,float y)
{
    osgUtil::IntersectVisitor iv;
    osg::ref_ptr<osg::LineSegment> segDown = new osg::LineSegment;

    const osg::BoundingSphere& bs = subgraph->getBound();
    float zMax = bs.center().z()+bs.radius();
    float zMin = bs.center().z()-bs.radius();
    
    segDown->set(osg::Vec3(x,y,zMin),osg::Vec3(x,y,zMax));
    iv.addLineSegment(segDown.get());

    subgraph->accept(iv);

    if (iv.hits())
    {
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
        if (!hitList.empty())
        {
            osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
            return  ip;
        }
    }

    return osg::Vec3(x,y,0.0f);
}


//////////////////////////////////////////////////////////////////////////////
// MAIN SCENE GRAPH BUILDING FUNCTION
//////////////////////////////////////////////////////////////////////////////

void build_world(osg::Group *root)
{

    osg::Geode* terrainGeode = new osg::Geode;
    // create terrain
    {
        osg::StateSet* stateset = new osg::StateSet();
        osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
        if (image)
        {
	    osg::Texture2D* texture = new osg::Texture2D;
	    texture->setImage(image);
	    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        }

        terrainGeode->setStateSet( stateset );

        float size = 1000; // 10km;
        float scale = size/39.0f; // 10km;
        float z_scale = scale*3.0f;

        osg::HeightField* grid = new osg::HeightField;
        grid->allocateGrid(38,39);
        grid->setXInterval(scale);
        grid->setYInterval(scale);

        for(unsigned int r=0;r<39;++r)
        {
	    for(unsigned int c=0;c<38;++c)
	    {
	        grid->setHeight(c,r,z_scale*vertex[r+c*39][2]);
	    }
        }
        terrainGeode->addDrawable(new osg::ShapeDrawable(grid));
        
        root->addChild(terrainGeode);
    }    

    // create sphere segment
    {
        osgSim::SphereSegment* ss = new osgSim::SphereSegment(
                        computeTerrainIntersection(terrainGeode,550.0f,780.0f), // center
                        500.0f, // radius
                        osg::DegreesToRadians(135.0f),
                        osg::DegreesToRadians(245.0f),
                        osg::DegreesToRadians(-10.0f),
                        osg::DegreesToRadians(30.0f),
                        60);
        ss->setAllColors(osg::Vec4(1.0f,1.0f,1.0f,0.5f));
        ss->setSideColor(osg::Vec4(0.0f,1.0f,1.0f,0.1f));

        root->addChild(ss);        
    }


    // create particle effects
    {    
        osg::PositionAttitudeTransform* positionEffects = new osg::PositionAttitudeTransform;
        positionEffects->setPosition(computeTerrainIntersection(terrainGeode,100.0f,100.0f));
        root->addChild(positionEffects);

        osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect;
        osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect;
        osgParticle::FireEffect* fire = new osgParticle::FireEffect;

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),10.0f)));
        positionEffects->addChild(geode);

        positionEffects->addChild(explosion);
        positionEffects->addChild(smoke);
        positionEffects->addChild(fire);

        osgParticle::ParticleSystemUpdater *psu = new osgParticle::ParticleSystemUpdater;

        psu->addParticleSystem(explosion->getParticleSystem());
        psu->addParticleSystem(smoke->getParticleSystem());
        psu->addParticleSystem(fire->getParticleSystem());

        // add the updater node to the scene graph
        root->addChild(psu);
    }
}


//////////////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////////////


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
    
    osg::Group *root = new osg::Group;
    build_world(root);
   
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);
        
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
