#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>

#include <osgProducer/Viewer>

#include <osgDB/ReadFile>

#include <osg/Math>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

osg::Geode* createShapes()
{
    osg::Geode* geode = new osg::Geode();

    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet* stateset = new osg::StateSet();

    osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
    if (image)
    {
	osg::Texture2D* texture = new osg::Texture2D;
	texture->setImage(image);
	stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    
    geode->setStateSet( stateset );
    
    float radius = 0.8f;
    float height = 1.0f;
    
    geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius)));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(2.0f,0.0f,0.0f),2*radius)));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Cone(osg::Vec3(4.0f,0.0f,0.0f),radius,height)));
    geode->addDrawable(new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(6.0f,0.0f,0.0f),radius,height)));

    osg::Grid* grid = new osg::Grid;
    grid->allocateGrid(38,39);
    grid->setXInterval(0.28f);
    grid->setYInterval(0.28f);
    
    for(unsigned int r=0;r<39;++r)
    {
	for(unsigned int c=0;c<38;++c)
	{
	    grid->setHeight(c,r,vertex[r+c*39][2]);
	}
    }
    geode->addDrawable(new osg::ShapeDrawable(grid));
    
    osg::ConvexHull* mesh = new osg::ConvexHull;
    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0].set(7.0+0.0f,-1.0f+2.0f,-1.0f+0.0f);
    (*vertices)[1].set(7.0+1.0f,-1.0f+0.0f,-1.0f+0.0f);
    (*vertices)[2].set(7.0+2.0f,-1.0f+2.0f,-1.0f+0.0f);
    (*vertices)[3].set(7.0+1.0f,-1.0f+1.0f,-1.0f+2.0f);
    osg::UByteArray* indices = new osg::UByteArray(12);
    (*indices)[0]=0;
    (*indices)[1]=2;
    (*indices)[2]=1;
    (*indices)[3]=0;
    (*indices)[4]=1;
    (*indices)[5]=3;
    (*indices)[6]=1;
    (*indices)[7]=2;
    (*indices)[8]=3;
    (*indices)[9]=2;
    (*indices)[10]=0;
    (*indices)[11]=3;
    mesh->setVertices(vertices);
    mesh->setIndices(indices);
    geode->addDrawable(new osg::ShapeDrawable(mesh));
    
    return geode;
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
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
    
    osg::Node* node = createShapes();

    // add model to viewer.
    viewer.setSceneData( node );

    // create the windows and run the threads.
    viewer.realize(Producer::CameraGroup::ThreadPerCamera);

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
