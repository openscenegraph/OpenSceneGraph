// -*-c++-*-

/*
 * Draw an outline around a model.
 */

#include <osg/Group>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osgFX/Outline>


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] <file>");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // create outline effect
    osg::ref_ptr<osgFX::Outline> outline = new osgFX::Outline;
    outline->setWidth(8);
    outline->setColor(osg::Vec4(1,1,0,1));

    // create scene
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild(outline.get());

    osg::ref_ptr<osg::Node> model0 = osgDB::readNodeFile(arguments.argc() > 1 ? arguments[1] : "al.obj");
    outline->addChild(model0.get());

    // must have stencil buffer...
    osg::DisplaySettings::instance()->setMinimumNumStencilBits(1);

    // construct the viewer
    osgViewer::Viewer viewer;
    viewer.setSceneData(root.get());

    // must clear stencil buffer...
    unsigned int clearMask = viewer.getCamera()->getClearMask();
    viewer.getCamera()->setClearMask(clearMask | GL_STENCIL_BUFFER_BIT);
    viewer.getCamera()->setClearStencil(0);

    return viewer.run();
}
