/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* file:        examples/osgglsl/osgshaders.cpp
 * author:        Mike Weiblen 2005-04-05
 *
 * A demo of the OpenGL Shading Language shaders using core OSG.
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

#include <osg/Notify>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include "GL2Scene.h"

using namespace osg;

///////////////////////////////////////////////////////////////////////////

class KeyHandler: public osgGA::GUIEventHandler
{
    public:
        KeyHandler( GL2ScenePtr gl2Scene ) :
                _gl2Scene(gl2Scene)
        {}

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
        {
            if( ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN )
                return false;

            switch( ea.getKey() )
            {
                case 'x':
                    _gl2Scene->reloadShaderSource();
                    return true;
                case 'y':
                    _gl2Scene->toggleShaderEnable();
                    return true;
            }
            return false;
        }

    private:
        GL2ScenePtr _gl2Scene;
};

///////////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser args(&argc,argv);
    
    // set up the usage document
    args.getApplicationUsage()->setApplicationName(args.getApplicationName());
    args.getApplicationUsage()->setDescription(args.getApplicationName() +
            " demonstrates the OpenGL Shading Language using core OSG");
    args.getApplicationUsage()->setCommandLineUsage(args.getApplicationName());
    args.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

    args.getApplicationUsage()->addKeyboardMouseBinding( "x", "Reload and recompile shader source files." );
    args.getApplicationUsage()->addKeyboardMouseBinding( "y", "Toggle shader enable" );

    // construct the viewer.
    osgProducer::Viewer viewer(args);
    viewer.setUpViewer( osgProducer::Viewer::STANDARD_SETTINGS );
    viewer.getUsage( *args.getApplicationUsage() );

    if( args.read("-h") || args.read("--help") )
    {
        args.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    args.reportRemainingOptionsAsUnrecognized();
    if( args.errors() )
    {
        args.writeErrorMessages(std::cout);
        return 1;
    }

    // create the scene
    GL2ScenePtr gl2Scene = new GL2Scene;

    viewer.setSceneData( gl2Scene->getRootNode().get() );
    viewer.getEventHandlerList().push_front( new KeyHandler(gl2Scene) );
    viewer.realize();
    while( !viewer.done() )
    {
        viewer.sync();
        viewer.update();
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

/*EOF*/

