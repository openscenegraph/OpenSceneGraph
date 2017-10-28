/* OpenSceneGraph example, osgsimplifier.
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

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>
#include <osgUtil/Simplifier>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <iostream>

class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:

    KeyboardEventHandler(unsigned int& flag, const std::string& filename) :
        _flag(flag),
        _outputFilename(filename)
    {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                if (ea.getKey()=='n')
                {
                    _flag = 1;
                    return true;
                }
                if (ea.getKey()=='p')
                {
                    _flag = 2;
                    return true;
                }
                if (ea.getKey()=='o')
                {
                    osgViewer::View* view = dynamic_cast<osgViewer::Viewer*>(aa.asView());
                    osg::Node* sceneData = view ? view->getSceneData() : 0;
                    if (sceneData)
                    {
                        OSG_NOTICE<<"Witten model to file: "<<_outputFilename<<std::endl;
                        osgDB::writeNodeFile(*sceneData, _outputFilename);
                    }
                    return true;
                }
                break;
            }
            default:
                break;
        }
        return false;
    }

private:

    unsigned int& _flag;
    std::string _outputFilename;
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" examples illustrates simplification of triangle meshes.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--ratio <ratio>","Specify the sample ratio","0.5]");
    arguments.getApplicationUsage()->addCommandLineOption("--max-error <error>","Specify the maximum error","4.0");


    std::string outputFilename="model.osgt";

    float sampleRatio = 0.5f;
    float maxError = 4.0f;

    // construct the viewer.
    osgViewer::Viewer viewer;

    // read the sample ratio if one is supplied
    while (arguments.read("--ratio",sampleRatio)) {}
    while (arguments.read("--max-error",maxError)) {}
    while (arguments.read("-o",outputFilename)) {}

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readRefNodeFile("dumptruck.osgt");

    // if no model has been successfully loaded report failure.
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    //loadedModel->accept(simplifier);

    unsigned int keyFlag = 0;
    viewer.addEventHandler(new KeyboardEventHandler(keyFlag, outputFilename));

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // create the windows and run the threads.
    viewer.realize();

    float multiplier = 0.8f;
    float minRatio = 0.001f;
    float ratio = sampleRatio;


    while( !viewer.done() )
    {
        // fire off the cull and draw traversals of the scene.
        viewer.frame();

        if (keyFlag == 1 || keyFlag == 2)
        {
            if (keyFlag == 1) ratio *= multiplier;
            if (keyFlag == 2) ratio /= multiplier;
            if (ratio<minRatio) ratio=minRatio;

            osgUtil::Simplifier simplifier(ratio, maxError);

            std::cout<<"Running osgUtil::Simplifier with SampleRatio="<<ratio<<" maxError="<<maxError<<" ...";
            std::cout.flush();

            osg::ref_ptr<osg::Node> root = (osg::Node*)loadedModel->clone(osg::CopyOp::DEEP_COPY_ALL);

            root->accept(simplifier);

            std::cout<<"done"<<std::endl;

            viewer.setSceneData(root.get());
            keyFlag = 0;
        }
    }

    return 0;
}

