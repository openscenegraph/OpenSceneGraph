// -*-c++-*-

#include <osgProducer/Viewer>

#include <osgDB/ReadFile>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TexMat>
#include <osg/CullFace>

#include <osgGA/TrackballManipulator>

#include <math.h>

#include "MpegImageStream.h"


/*
 * Create morphed textured geometry
 */
osg::Geode* morphGeom(osg::Vec3Array* coords,
                      osg::Vec3Array* normals,
                      osg::Vec2Array* texCoords,
                      osg::Image* image,
                      osg::TexMat* texMat)
{
    /*
     * GeoSet
     */
    osg::Geometry* gset = new osg::Geometry();

    gset->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,4));

    gset->setVertexArray(coords);

    gset->setNormalArray(normals);
    gset->setNormalBinding(osg::Geometry::BIND_OVERALL);

    gset->setTexCoordArray(0,texCoords);

    /*
     * StateSet
     */
    osg::StateSet* state = new osg::StateSet;

    osg::Material* mtl = new osg::Material();
    osg::Vec4 white( 1.0f, 1.0f, 1.0f, 1.0f );
    mtl->setEmission( osg::Material::FRONT_AND_BACK, white );
    mtl->setAmbient( osg::Material::FRONT_AND_BACK, white );
    mtl->setDiffuse( osg::Material::FRONT_AND_BACK, white );
    mtl->setSpecular( osg::Material::FRONT_AND_BACK, white );
    state->setAttribute(mtl);

    //osg::Texture2D* tex = new osg::Texture2D;
    osg::TextureRectangle* tex = new osg::TextureRectangle;
    if (!image) {
        image = osgDB::readImageFile("lz.rgb");
    }
    tex->setImage(image);
    tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
    tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
    state->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);

    if (texMat)
        state->setTextureAttributeAndModes(0, texMat, osg::StateAttribute::ON);

    // don't cull faces
    osg::CullFace* cull = new osg::CullFace;
    state->setAttributeAndModes(cull, osg::StateAttribute::OFF);

    /*
     * Geode
     */
    osg::Geode* geode = new osg::Geode;
    geode->setStateSet( state );
    geode->addDrawable( gset );

    return geode;
}


/*
 * Main
 */
int main(int argc, char** argv)
{
    // coordinates
    osg::Vec3Array* coords = new osg::Vec3Array(4);
    (*coords)[0].set( -1.0f, 0.0f, -1.0f );
    (*coords)[1].set(  1.0f, 0.0f, -1.0f );
    (*coords)[2].set(  1.0f, 0.0f,  1.0f );
    (*coords)[3].set( -1.0f, 0.0f,  1.0f );


    // normals
    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set( 0.0f, 1.0f, 0.0f );

    // texture coordinates
    osg::Vec2Array* texCoords = new osg::Vec2Array(4);
    (*texCoords)[0].set(0.0f, 0.0f);
    (*texCoords)[1].set(1.0f, 0.0f);
    (*texCoords)[2].set(1.0f, 1.0f);
    (*texCoords)[3].set(0.0f, 1.0f);


    // open MpegImageStream
    osg::MpegImageStream* mpeg = NULL;
    if (argc > 1) {
        mpeg = new osg::MpegImageStream(argv[1]);
        mpeg->start();
    }

    osg::TexMat* texMat = new osg::TexMat;
    texMat->setMatrix(osg::Matrix::scale(mpeg->s(),-mpeg->t(),1.0f)*osg::Matrix::translate(0.0f,mpeg->t(),0.0f));


    // Create morphed geometry
    osg::Geode* geode = morphGeom(coords,
                                  normals, texCoords, mpeg, texMat);
    //coordMorph.addGeode(geode);




    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
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

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }


    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

    // set the scene to render
    viewer.setSceneData(geode);

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();
        
        // update the geoemtry
        //coordMorph.update(viewer.getFrameStamp()->getReferenceTime());

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
