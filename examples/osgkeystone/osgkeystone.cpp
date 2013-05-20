/* OpenSceneGraph example, osganimate.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TextureRectangle>
#include <osg/TexMat>
#include <osg/Stencil>
#include <osg/PolygonStipple>
#include <osg/ValueObject>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/config/SingleWindow>
#include <osgViewer/config/SingleScreen>
#include <osgViewer/config/WoWVxDisplay>


int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    // initialize the viewer.
    osgViewer::Viewer viewer(arguments);
    
    osg::DisplaySettings* ds = viewer.getDisplaySettings() ? viewer.getDisplaySettings() : osg::DisplaySettings::instance().get();
    ds->readCommandLine(arguments);

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    if (!model)
    {
        OSG_NOTICE<<"No models loaded, please specify a model file on the command line"<<std::endl;
        return 1;
    }


    OSG_NOTICE<<"Stereo "<<ds->getStereo()<<std::endl;
    OSG_NOTICE<<"StereoMode "<<ds->getStereoMode()<<std::endl;

    viewer.setSceneData(model.get());
    
    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    OSG_NOTICE<<"KeystoneFileNames.size()="<<ds->getKeystoneFileNames().size()<<std::endl;
    for(osg::DisplaySettings::FileNames::iterator itr = ds->getKeystoneFileNames().begin();
        itr != ds->getKeystoneFileNames().end();
        ++itr)
    {
        OSG_NOTICE<<"   keystone filename = "<<*itr<<std::endl;
    }

    ds->setKeystoneHint(true);
    
    if (!ds->getKeystoneFileNames().empty())
    {
        for(osg::DisplaySettings::Objects::iterator itr = ds->getKeystones().begin();
            itr != ds->getKeystones().end();
            ++itr)
        {
            osgViewer::Keystone* keystone = dynamic_cast<osgViewer::Keystone*>(itr->get());
            if (keystone) 
            {
                std::string filename;
                keystone->getUserValue("filename",filename);
                OSG_NOTICE<<"Loaded keystone "<<filename<<", "<<keystone<<std::endl;
                
                ds->getKeystones().push_back(keystone);
            }
        }
    }
    
    viewer.apply(new osgViewer::SingleScreen(0));
    
    viewer.realize();

    while(!viewer.done())
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        viewer.renderingTraversals();
    }
    return 0;
}
