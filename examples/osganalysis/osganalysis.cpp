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
#include <osgDB/WriteFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osgUtil/IncrementalCompileOperation>
#include <osgUtil/Simplifier>

class StripStateVisitor : public osg::NodeVisitor
{
public:
    StripStateVisitor(bool useStateSets, bool useDisplayLists, bool useVBO):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _useStateSets(useStateSets),
        _useDisplayLists(useDisplayLists),
        _useVBO(useVBO) {}

    bool _useStateSets;
    bool _useDisplayLists;
    bool _useVBO;

    void apply(osg::Node& node)
    {
        if (!_useStateSets && node.getStateSet()) node.setStateSet(0);
        traverse(node);
    }

    void apply(osg::Geode& node)
    {
        if (!_useStateSets && node.getStateSet()) node.setStateSet(0);
        for(unsigned int i = 0; i<node.getNumDrawables(); ++i)
        {
            process(*node.getDrawable(i));
        }

        traverse(node);
    }

    void process(osg::Drawable& drawable)
    {
        if (!_useStateSets && drawable.getStateSet())
        {
            drawable.setStateSet(0);
        }

        drawable.setUseDisplayList(_useDisplayLists);
        drawable.setUseVertexBufferObjects(_useVBO);
    }
};

class SceneGraphProcessor : public osg::Referenced
{
public:
    SceneGraphProcessor()
    {
        _init();
    }

    SceneGraphProcessor(osg::ArgumentParser& arguments)
    {
        _init();

        while (arguments.read("--vbo")) { modifyDrawableSettings = true; useVBO = true;  }
        while (arguments.read("--dl")) { modifyDrawableSettings = true; useDisplayLists = true;  }

        while (arguments.read("-s", simplificatioRatio)) {}

        while (arguments.read("--build-mipmaps")) { modifyTextureSettings = true; buildImageMipmaps = true; }
        while (arguments.read("--compress")) { modifyTextureSettings = true; compressImages = true; }
        while (arguments.read("--disable-mipmaps")) { modifyTextureSettings = true; disableMipmaps = true; }

        OSG_NOTICE<<"simplificatioRatio="<<simplificatioRatio<<std::endl;
    }

    virtual osg::Node* process(osg::Node* node)
    {
        if (!node)
        {
            OSG_NOTICE<<"SceneGraphProcessor::process(Node*) : error cannot process NULL Node."<<std::endl;
            return 0;
        }

        OSG_NOTICE<<"SceneGraphProcessor::process("<<node<<") : "<<node->getName()<<std::endl;

        if (simplificatioRatio < 1.0)
        {
            OSG_NOTICE<<"Running simplifier with simplification ratio="<<simplificatioRatio<<std::endl;
            float maxError = 4.0f;
            osgUtil::Simplifier simplifier(simplificatioRatio, maxError);
            //simplifier.setDoTriStrip(false);
            //simplifier.setSmoothing(false);
            node->accept(simplifier);
        }

        if (modifyDrawableSettings || modifyTextureSettings)
        {
            OSG_NOTICE<<"Running StripStateVisitor"<<std::endl;
            StripStateVisitor ssv(true, useDisplayLists, useVBO);
            node->accept(ssv);
        }


        return node;
    }

protected:

    void _init()
    {
        modifyDrawableSettings = false;
        useVBO = false;
        useDisplayLists = false;
        simplificatioRatio = 1.0;

        modifyTextureSettings = false;
        buildImageMipmaps = false;
        compressImages = false;
        disableMipmaps = false;
    }

    bool modifyDrawableSettings;
    bool useVBO;
    bool useDisplayLists;
    float simplificatioRatio;

    bool modifyTextureSettings;
    bool buildImageMipmaps;
    bool compressImages;
    bool disableMipmaps;

};

class DatabasePagingOperation : public osg::Operation, public osgUtil::IncrementalCompileOperation::CompileCompletedCallback
{
public:

    DatabasePagingOperation(const std::string& filename,
                            const std::string& outputFilename,
                             SceneGraphProcessor* sceneGraphProcessor, 
                             osgUtil::IncrementalCompileOperation* ico):
        Operation("DatabasePaging Operation", false),
        _filename(filename),
        _outputFilename(outputFilename),
        _modelReadyToMerge(false),
        _sceneGraphProcessor(sceneGraphProcessor),
        _incrementalCompileOperation(ico) {}

    virtual void operator () (osg::Object* object)
    {
        osg::notify(osg::NOTICE)<<"LoadAndCompileOperation "<<_filename<<std::endl;

        _modelReadyToMerge = false;
        _loadedModel = osgDB::readNodeFile(_filename);

        if (_loadedModel.valid())
        {
            if (_sceneGraphProcessor.valid())
            {
                _loadedModel = _sceneGraphProcessor->process(_loadedModel.get());
            }
        }

        if (_loadedModel.valid())
        {
            if (!_outputFilename.empty())
            {
                osgDB::writeNodeFile(*_loadedModel, _outputFilename);
            }

            if (_incrementalCompileOperation.valid())
            {
                osg::ref_ptr<osgUtil::IncrementalCompileOperation::CompileSet> compileSet =
                    new osgUtil::IncrementalCompileOperation::CompileSet(_loadedModel.get());

                compileSet->_compileCompletedCallback = this;

                _incrementalCompileOperation->add(compileSet.get());
            }
            else
            {
                _modelReadyToMerge = true;
            }
        }

        osg::notify(osg::NOTICE)<<"done LoadAndCompileOperation "<<_filename<<std::endl;
    }

    virtual bool compileCompleted(osgUtil::IncrementalCompileOperation::CompileSet* compileSet)
    {
        OSG_NOTICE<<"compileCompleted"<<std::endl;
        _modelReadyToMerge = true;
        return true;
    }

    std::string                                         _filename;
    std::string                                         _outputFilename;
    osg::ref_ptr<osg::Node>                             _loadedModel;
    bool                                                _modelReadyToMerge;
    osg::ref_ptr<SceneGraphProcessor>                   _sceneGraphProcessor;
    osg::ref_ptr<osgUtil::IncrementalCompileOperation>  _incrementalCompileOperation;
};


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.addEventHandler( new osgViewer::StatsHandler());
    viewer.addEventHandler( new osgViewer::WindowSizeHandler() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    /////////////////////////////////////////////////////////////////////////////////
    //
    // IncrementalCompileOperation settings
    //
    osg::ref_ptr<osgUtil::IncrementalCompileOperation> incrementalCompile = new osgUtil::IncrementalCompileOperation;
    viewer.setIncrementalCompileOperation(incrementalCompile.get());

    if (arguments.read("--force") || arguments.read("-f"))
    {
        incrementalCompile->assignForceTextureDownloadGeometry();
    }

    if (arguments.read("-a"))
    {
        incrementalCompile->setMinimumTimeAvailableForGLCompileAndDeletePerFrame(1);
        incrementalCompile->setConservativeTimeRatio(1);
        incrementalCompile->setMaximumNumOfObjectsToCompilePerFrame(100);
    }
    else if (arguments.read("-c"))
    {
        incrementalCompile->setMinimumTimeAvailableForGLCompileAndDeletePerFrame(0.0001);
        incrementalCompile->setConservativeTimeRatio(0.01);
        incrementalCompile->setMaximumNumOfObjectsToCompilePerFrame(1);
    }

    /////////////////////////////////////////////////////////////////////////////////
    //
    // SceneGraph processing setup
    //
    osg::ref_ptr<SceneGraphProcessor> sceneGraphProcessor = new SceneGraphProcessor(arguments);

    /////////////////////////////////////////////////////////////////////////////////
    //
    // Database settings
    //
    double timeBetweenMerges = 2.0;
    while(arguments.read("--interval",timeBetweenMerges)) {}

    std::string outputPostfix;
    while (arguments.read("-o",outputPostfix)) {}


    typedef std::vector< std::string > FileNames;
    FileNames fileNames;
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            fileNames.push_back(arguments[pos]);
        }
    }

    if (fileNames.empty())
    {
        OSG_NOTICE<<"No files loaded, please specifies files on commandline."<<std::endl;
        return 1;
    }

    // load the models using a paging thread and use the incremental compile operation to
    // manage the compilation of GL objects without breaking frame.

    unsigned int modelIndex = 0;

    osg::ref_ptr<osg::OperationThread> databasePagingThread;
    osg::ref_ptr<DatabasePagingOperation> databasePagingOperation;

    databasePagingThread = new osg::OperationThread;
    databasePagingThread->startThread();

    std::string filename = fileNames[modelIndex++];
    std::string outputFilename = outputPostfix.empty() ? std::string() : filename+outputPostfix;

    databasePagingOperation = new DatabasePagingOperation(
        filename,
        outputFilename,
        sceneGraphProcessor.get(),
        incrementalCompile.get());

    databasePagingThread->add(databasePagingOperation.get());

    osg::ref_ptr<osg::Group> group = new osg::Group;
    viewer.setSceneData(group);

    viewer.realize();

    double timeOfLastMerge = viewer.getFrameStamp()->getReferenceTime();

    while(!viewer.done())
    {
        viewer.frame();

        double currentTime = viewer.getFrameStamp()->getReferenceTime();

        if (!databasePagingOperation &&
            modelIndex<fileNames.size() &&
            (currentTime-timeOfLastMerge)>timeBetweenMerges)
        {
            std::string filename = fileNames[modelIndex++];
            std::string outputFilename = outputPostfix.empty() ? std::string() : filename+outputPostfix;

            databasePagingOperation = new DatabasePagingOperation(
                filename,
                outputFilename,
                sceneGraphProcessor.get(),
                incrementalCompile.get());

            databasePagingThread->add(databasePagingOperation.get());
        }

        if (databasePagingOperation.get() && databasePagingOperation->_modelReadyToMerge)
        {
            timeOfLastMerge = currentTime;

            group->removeChildren(0,group->getNumChildren());

            group->addChild(databasePagingOperation->_loadedModel.get());

            viewer.home();

            // we no longer need the paging operation as it's done it's job.
            databasePagingOperation = 0;

            viewer.home();
        }
    }

    return 0;
}
