#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>

#include <osgGA/TrackballManipulator>

#include <osgGLUT/Viewer>
#include <osgGLUT/glut>

#include <osgDB/ReadFile>

#include <osg/Math>

#include "../osghangglide/terrain_coords.h"

osg::Geode* createShapes()
{
    osg::Geode* geode = osgNew osg::Geode();

    // ---------------------------------------
    // Set up a StateSet to make the cube red
    // ---------------------------------------
    osg::StateSet* stateset = osgNew osg::StateSet();

   
    osg::Image* image = osgDB::readImageFile("lz.rgb");
    if (image)
    {
	osg::Texture2D* texture = osgNew osg::Texture2D;
	texture->setImage(image);
	stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    
    geode->setStateSet( stateset );
    
    float radius = 0.8f;
    float height = 1.0f;
    
    geode->addDrawable(new osg::ShapeDrawable(osgNew osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),radius)));
    geode->addDrawable(new osg::ShapeDrawable(osgNew osg::Box(osg::Vec3(2.0f,0.0f,0.0f),2*radius)));
    geode->addDrawable(new osg::ShapeDrawable(osgNew osg::Cone(osg::Vec3(4.0f,0.0f,0.0f),radius,height)));
    geode->addDrawable(new osg::ShapeDrawable(osgNew osg::Cylinder(osg::Vec3(6.0f,0.0f,0.0f),radius,height)));

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

    glutInit( &argc, argv );

    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);

    // create the viewer and the model to it.
    osgGLUT::Viewer viewer;

    viewer.setWindowTitle(argv[0]);

 
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    osg::Node* node = createShapes();

    // add model to viewer.
    viewer.addViewport( node );

    // register trackball maniupulators.
    viewer.registerCameraManipulator(osgNew osgGA::TrackballManipulator);
    
    viewer.open();

    viewer.run();

    return 0;
}
