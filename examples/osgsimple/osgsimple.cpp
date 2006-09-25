// C++ source file - (C) 2003 Robert Osfield, released under the OSGPL.
//
// Simple example of use of Producer::RenderSurface to create an OpenGL
// graphics window, and OSG for rendering.

#include <Producer/RenderSurface>

#include <osgUtil/SceneView>
#include <osgDB/ReadFile>
#include <osgGA/SimpleViewer>


int main( int argc, char **argv )
{
    if (argc<2) 
    {
        std::cout << argv[0] <<": requires filename argument." << std::endl;
        return 1;
    }

    // load the scene.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    }
    
    // create the window to draw to.
    osg::ref_ptr<Producer::RenderSurface> renderSurface = new Producer::RenderSurface;
    renderSurface->setWindowName("osgsimple");
    renderSurface->setWindowRectangle(100,100,800,600);
    renderSurface->useBorder(true);
    renderSurface->realize();
    
    
    osgGA::SimpleViewer viewer;
    viewer.setSceneData(loadedModel.get());

    // initialize the view to look at the center of the scene graph
    const osg::BoundingSphere& bs = loadedModel->getBound();
    osg::Matrix viewMatrix;
    viewMatrix.makeLookAt(bs.center()-osg::Vec3(0.0,2.0f*bs.radius(),0.0),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while( renderSurface->isRealized() )
    {        
        // update the window dimensions, in case the window has been resized.
        viewer.getEventQueue()->windowResize(0,0,renderSurface->getWindowWidth(),renderSurface->getWindowHeight());

        // set the view
        viewer.getCamera()->setViewMatrix(viewMatrix);
        
        // advance the frame, handle events, update the scene and render it. 
        viewer.frame();

        // Swap Buffers
        renderSurface->swapBuffers();
    }

    return 0;
}

