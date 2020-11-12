// This is public domain software and comes with
// absolutely no warranty. Use of public domain software
// may vary between counties, but in general you are free
// to use and distribute this software for any purpose.


// Example: OSG using an OpenGL 3.1 context.
// The comment block at the end of the source describes building OSG
// for use with OpenGL 3.x.

#include <osgViewer/Viewer>
#include <osgViewer/config/SphericalDisplay>
#include <osgViewer/config/PanoramicSphericalDisplay>
#include <osgViewer/config/WoWVxDisplay>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/Viewport>
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Texture2D>
#include <osg/Texture1D>

#include "StandardStereo.h"
#include "MultiviewOVR.h"

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    if (arguments.read("--standard"))
    {
        viewer.apply(new StandardStereo());
    }
    else
    {
        viewer.apply(new MultiviewOVR());
    }

    osg::ref_ptr<osg::Node> root = osgDB::readRefNodeFiles( arguments );
    if (!root)
    {
        osg::notify( osg::FATAL ) << "Unable to load model from command line." << std::endl;
        return 1;
    }

    viewer.setSceneData( root );

    viewer.addEventHandler(new osgViewer::StatsHandler());


    return viewer.run();
}
