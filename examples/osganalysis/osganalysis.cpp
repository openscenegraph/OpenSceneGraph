/* OpenSceneGraph example, osgterrain.
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


#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgUtil/IncrementalCompileOperation>


struct CustomCompileCompletedCallback : public osgUtil::IncrementalCompileOperation::CompileCompletedCallback
{
    CustomCompileCompletedCallback():
        completed(false) {}

    virtual bool compileCompleted(osgUtil::IncrementalCompileOperation::CompileSet* compileSet)
    {
        OSG_NOTICE<<"compileCompleted"<<std::endl;
        completed = true;
        return true;
    }

    bool completed;
};

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    osg::ref_ptr<osgUtil::IncrementalCompileOperation> incrementalCompile = new osgUtil::IncrementalCompileOperation;
    viewer.setIncrementalCompileOperation(incrementalCompile.get());

    double timeBetweenMerges = 2.0;
    while(arguments.read("--interval",timeBetweenMerges)) {}

    typedef std::vector< osg::ref_ptr<osg::Node> > Models;
    Models models;

    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            // not an option so assume string is a filename.
            osg::Node *node = osgDB::readNodeFile( arguments[pos]);
            if(node)
            {
                if (node->getName().empty()) node->setName( arguments[pos] );
                models.push_back(node);
            }
        }
    }

    OSG_NOTICE<<"models.size()="<<models.size()<<std::endl;

    osg::ref_ptr<osg::Group> group = new osg::Group;

    unsigned int modelIndex = 0;

    group->addChild(models[modelIndex++].get());

    viewer.setSceneData(group);

    viewer.realize();

    double timeOfLastMerge = viewer.getFrameStamp()->getReferenceTime();

    osg::ref_ptr<CustomCompileCompletedCallback> compileCompletedCallback;

    while(!viewer.done())
    {
        viewer.frame();

        double currentTime = viewer.getFrameStamp()->getReferenceTime();

        if (!compileCompletedCallback &&
            modelIndex<models.size() &&
            (currentTime-timeOfLastMerge)>timeBetweenMerges)
        {
            OSG_NOTICE<<"Compiling model "<<modelIndex<<" at "<<currentTime<<std::endl;

            osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet> compileSet =
                new osgUtil::IncrementalCompileOperation::CompileSet(models[modelIndex].get());

            compileCompletedCallback = new CustomCompileCompletedCallback;

            compileSet->_compileCompletedCallback = compileCompletedCallback;

            incrementalCompile->add(compileSet.get());
        }

        if (compileCompletedCallback.valid() && compileCompletedCallback->completed)
        {
            OSG_NOTICE<<"Merging model "<<modelIndex<<" at "<<currentTime<<std::endl;

            timeOfLastMerge = currentTime;

            compileCompletedCallback = 0;

            group->removeChildren(0,group->getNumChildren());

            group->addChild(models[modelIndex++].get());

            viewer.home();
        }

    }

    return 0;
}
