#include <osgViewer/Viewer>

#include <osg/Notify>

#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>

#include <iostream>

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
   
    // construct the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
    if (!rootnode)
    {
        osg::notify(osg::NOTICE)<<"Please specify and model filename on the command line."<<std::endl;
        return 1;
    }
    
    osg::Image* image = osgDB::readImageFile("Images/reflect.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);

        osg::TexGen* texgen = new osg::TexGen;
        texgen->setMode(osg::TexGen::SPHERE_MAP);

        osg::TexEnv* texenv = new osg::TexEnv;
        texenv->setMode(osg::TexEnv::BLEND);
        texenv->setColor(osg::Vec4(0.3f,0.3f,0.3f,0.3f));

        osg::StateSet* stateset = new osg::StateSet;
        stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);
        stateset->setTextureAttributeAndModes(1,texgen,osg::StateAttribute::ON);
        stateset->setTextureAttribute(1,texenv);
        
        rootnode->setStateSet(stateset);
    }
    else
    {
        osg::notify(osg::NOTICE)<<"unable to load reflect map, model will not be mutlitextured"<<std::endl;
    }

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    
    // create the windows and run the threads.
    viewer.realize();

    for(unsigned int contextID = 0; 
        contextID<osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts();
        ++contextID)
    {
        osg::Texture::Extensions* textExt = osg::Texture::getExtensions(contextID,false);
        if (textExt)
        {
            if (!textExt->isMultiTexturingSupported())
            {
                std::cout<<"Warning: multi-texturing not supported by OpenGL drivers, unable to run application."<<std::endl;
                return 1;
            }
        }
    }

    return viewer.run();
}
