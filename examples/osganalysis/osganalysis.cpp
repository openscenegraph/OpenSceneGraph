/* OpenSceneGraph example, osganalysis.
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
#include <osgDB/FileNameUtils>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgUtil/IncrementalCompileOperation>
#include <osgUtil/Simplifier>
#include <osgUtil/MeshOptimizers>

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

    void apply(osg::Drawable& drawable)
    {
        if (!_useStateSets && drawable.getStateSet())
        {
            drawable.setStateSet(0);
        }

        drawable.setUseDisplayList(_useDisplayLists);
        drawable.setUseVertexBufferObjects(_useVBO);
    }
};

class OptimizeImageVisitor : public osg::NodeVisitor
{
public:
    OptimizeImageVisitor(osgDB::ImageProcessor* imageProcessor, bool compressImages, bool generateMipmaps):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _imageProcessor(imageProcessor),
        _compressImages(compressImages),
        _generateMipmaps(generateMipmaps) {}

    osg::ref_ptr<osgDB::ImageProcessor> _imageProcessor;
    bool _compressImages;
    bool _generateMipmaps;

    void apply(osg::Node& node)
    {
        processStateSet(node.getStateSet());
        traverse(node);
    }

    void apply(osg::Geode& node)
    {
        processStateSet(node.getStateSet());
        for(unsigned int i = 0; i<node.getNumDrawables(); ++i)
        {
            processStateSet(node.getDrawable(i)->getStateSet());
        }

        traverse(node);
    }

    void processStateSet(osg::StateSet* stateset)
    {
        if (!stateset) return;

        for(unsigned int i=0; i<stateset->getNumTextureAttributeLists(); ++i)
        {
            osg::StateAttribute* sa = stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE);
            osg::Texture* texture = dynamic_cast<osg::Texture*>(sa);
            if (texture)
            {
                for(unsigned int i=0; i<texture->getNumImages(); ++i)
                {
                    proccessImage(texture->getImage(i));
                }
            }
        };
    }


    void proccessImage(osg::Image* image)
    {
        if (!image) return;

        if (_imageProcessor.valid())
        {
            OSG_NOTICE<<"Will be using ImageProcessor to process image "<<image->getFileName()<<std::endl;
        }
        else
        {
            OSG_NOTICE<<"No ImageProcessor to process image "<<image->getFileName()<<std::endl;
        }
            OSG_NOTICE<<"   compressImage "<<_compressImages<<std::endl;
            OSG_NOTICE<<"   generateMipmaps "<<_generateMipmaps<<std::endl;
    }

};


class SwapArrayVisitor : public osg::ArrayVisitor
{
public:
    SwapArrayVisitor(osg::Array* array):
        _array(array) {}

    template <class ARRAY>
    void apply_imp(ARRAY& array)
    {
        if (array.getType()!=_array->getType())
        {
            OSG_NOTICE<<"Arrays incompatible"<<std::endl;
            return;
        }
        OSG_NOTICE<<"Swapping Array"<<std::endl;
        array.swap(*static_cast<ARRAY*>(_array));
    }

    virtual void apply(osg::ByteArray& ba) { apply_imp(ba); }
    virtual void apply(osg::ShortArray& ba) { apply_imp(ba); }
    virtual void apply(osg::IntArray& ba) { apply_imp(ba); }
    virtual void apply(osg::UByteArray& ba) { apply_imp(ba); }
    virtual void apply(osg::UShortArray& ba) { apply_imp(ba); }
    virtual void apply(osg::UIntArray& ba) { apply_imp(ba); }
    virtual void apply(osg::Vec4ubArray& ba) { apply_imp(ba); }
    virtual void apply(osg::FloatArray& ba) { apply_imp(ba); }
    virtual void apply(osg::Vec2Array& ba) { apply_imp(ba); }
    virtual void apply(osg::Vec3Array& ba) { apply_imp(ba); }
    virtual void apply(osg::Vec4Array& ba) { apply_imp(ba); }

    osg::Array* _array;
};

class MemoryVisitor : public osg::NodeVisitor
{
public:
     MemoryVisitor():
         osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}


    void reset()
    {
         _nodes.clear();
         _geometryMap.clear();
         _arrayMap.clear();
         _primitiveSetMap.clear();
    }

    void apply(osg::Node& node)
    {
        _nodes.insert(&node);
        traverse(node);
    }

    void apply(osg::Geode& geode)
    {
        _nodes.insert(&geode);
        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            apply(&geode, geode.getDrawable(i));
        }
    }

    void apply(osg::Geode* geode, osg::Drawable* drawable)
    {
        if (!drawable) return;

        osg::Geometry* geometry = drawable->asGeometry();
        if (geometry)
        {
            _geometryMap[geometry].insert(geode);

            apply(geometry, geometry->getVertexArray());
            apply(geometry, geometry->getNormalArray());
            apply(geometry, geometry->getColorArray());
            apply(geometry, geometry->getSecondaryColorArray());
            apply(geometry, geometry->getFogCoordArray());

            for(unsigned int i=0; i<geometry->getNumTexCoordArrays(); ++i)
            {
                apply(geometry, geometry->getTexCoordArray(i));
            }
            for(unsigned int i=0; i<geometry->getNumVertexAttribArrays(); ++i)
            {
                apply(geometry, geometry->getVertexAttribArray(i));
            }

            for(unsigned int i=0; i<geometry->getNumPrimitiveSets(); ++i)
            {
                apply(geometry, geometry->getPrimitiveSet(i));
            }
        }
    }

    void apply(osg::Geometry* geometry, osg::Array* array)
    {
        if (!array) return;
        _arrayMap[array].insert(geometry);
    }

    void apply(osg::Geometry* geometry, osg::PrimitiveSet* primitiveSet)
    {
        if (!primitiveSet) return;
        _primitiveSetMap[primitiveSet].insert(geometry);
    }

    void report(std::ostream& out)
    {
        out<<"Nodes "<<_nodes.size()<<std::endl;
        out<<"Geometries "<<_geometryMap.size()<<std::endl;
        out<<"Arrays "<<_arrayMap.size()<<std::endl;
        out<<"PrimitiveSets "<<_primitiveSetMap.size()<<std::endl;
    }

    void reallocate()
    {
        OSG_NOTICE<<"Reallocating Arrays"<<std::endl;

        typedef std::vector< osg::ref_ptr<osg::Array> > ArrayVector;
        typedef std::vector< osg::ref_ptr<osg::Geometry> > GeometryVector;
        ArrayVector newArrays;
        GeometryVector newGeometries;
        for(GeometryMap::iterator itr = _geometryMap.begin();
            itr != _geometryMap.end();
            ++itr)
        {
            osg::Geometry* geometry = itr->first;
            bool useVBO = geometry->getUseVertexBufferObjects();
            osg::Geometry* newGeometry = osg::clone(geometry, osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL));
            newGeometry->setUseVertexBufferObjects(false);
            newGeometry->setUseVertexBufferObjects(useVBO);
            newGeometries.push_back(newGeometry);
        }

        GeometryVector::iterator geom_itr = newGeometries.begin();
        for(GeometryMap::iterator itr = _geometryMap.begin();
            itr != _geometryMap.end();
            ++itr, ++geom_itr)
        {
            osg::Geometry* geometry = itr->first;
            Geodes& geodes = itr->second;
            for(Geodes::iterator gitr = geodes.begin();
                gitr != geodes.end();
                ++gitr)
            {
                osg::Geode* geode = const_cast<osg::Geode*>(*gitr);
                geode->replaceDrawable(geometry, geom_itr->get());
            }
        }
    }

    typedef std::vector< osg::ref_ptr<osg::Geometry> > GeometryVector;
    typedef std::pair<osg::Array*, osg::Array*> ArrayPair;
    typedef std::vector< ArrayPair > ArrayVector;
    typedef std::pair<osg::PrimitiveSet*, osg::PrimitiveSet*> PrimitiveSetPair;
    typedef std::vector< PrimitiveSetPair > PrimitiveSetVector;

    osg::Array* cloneArray(ArrayVector& arrayVector, osg::Array* array)
    {
        if (!array) return 0;
        osg::Array* newArray = static_cast<osg::Array*>(array->cloneType());
        arrayVector.push_back(ArrayPair(array,newArray));
        return newArray;
    }

    osg::PrimitiveSet* clonePrimitiveSet(PrimitiveSetVector& psVector, osg::PrimitiveSet* ps)
    {
        if (!ps) return 0;
        osg::PrimitiveSet* newPS = static_cast<osg::PrimitiveSet*>(ps->cloneType());
        psVector.push_back(PrimitiveSetPair(ps,newPS));
        return newPS;
    }

    void reallocate2()
    {
        OSG_NOTICE<<"Reallocating Arrays"<<std::endl;

        ArrayVector arrayVector;
        PrimitiveSetVector primitiveSetVector;
        GeometryVector newGeometries;

        for(GeometryMap::iterator itr = _geometryMap.begin();
            itr != _geometryMap.end();
            ++itr)
        {
            osg::Geometry* geometry = itr->first;
            osg::ref_ptr<osg::Geometry> newGeometry = osg::clone(geometry, osg::CopyOp::SHALLOW_COPY);
            newGeometries.push_back(newGeometry.get());

            newGeometry->setVertexArray(cloneArray(arrayVector, geometry->getVertexArray()));
            newGeometry->setNormalArray(cloneArray(arrayVector, geometry->getNormalArray()));
            newGeometry->setColorArray(cloneArray(arrayVector, geometry->getColorArray()));
            newGeometry->setSecondaryColorArray(cloneArray(arrayVector, geometry->getSecondaryColorArray()));
            newGeometry->setFogCoordArray(cloneArray(arrayVector, geometry->getFogCoordArray()));
            for(unsigned int i=0; i<geometry->getNumTexCoordArrays(); ++i)
            {
                newGeometry->setTexCoordArray(i, cloneArray(arrayVector, geometry->getTexCoordArray(i)));
            }
            for(unsigned int i=0; i<geometry->getNumVertexAttribArrays(); ++i)
            {
                newGeometry->setVertexAttribArray(i, cloneArray(arrayVector, geometry->getVertexAttribArray(i)));
            }

            for(unsigned int i=0; i<geometry->getNumPrimitiveSets(); ++i)
            {
                newGeometry->setPrimitiveSet(i,clonePrimitiveSet(primitiveSetVector, geometry->getPrimitiveSet(i)));
            }
        }

        GeometryVector::iterator geom_itr = newGeometries.begin();
        for(GeometryMap::iterator itr = _geometryMap.begin();
            itr != _geometryMap.end();
            ++itr, ++geom_itr)
        {
            osg::Geometry* geometry = itr->first;
            Geodes& geodes = itr->second;
            for(Geodes::iterator gitr = geodes.begin();
                gitr != geodes.end();
                ++gitr)
            {
                osg::Geode* geode = const_cast<osg::Geode*>(*gitr);
                geode->replaceDrawable(geometry, geom_itr->get());
            }
        }
    }

protected:

     typedef std::set<osg::Node*>  Nodes;
     typedef std::set<osg::Geode*>  Geodes;
     typedef std::set<osg::Geometry*>  Geometries;
     typedef std::map<osg::Geometry*, Geodes> GeometryMap;
     typedef std::map<osg::Array*, Geometries> ArrayMap;
     typedef std::map<osg::PrimitiveSet*, Geometries> PrimitiveSetMap;

     Nodes              _nodes;
     GeometryMap        _geometryMap;
     ArrayMap           _arrayMap;
     PrimitiveSetMap    _primitiveSetMap;
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
        while (arguments.read("--tristripper")) { useTriStripVisitor=true; }
        while (arguments.read("--no-tristripper")) { useTriStripVisitor=false; }
        while (arguments.read("--smoother")) {  useSmoothingVisitor=true; }
        while (arguments.read("--no-smoother")) {  useSmoothingVisitor=false; }

        while (arguments.read("--remove-duplicate-vertices") || arguments.read("--rdv")) removeDuplicateVertices = true;
        while (arguments.read("--optimize-vertex-cache") || arguments.read("--ovc")) optimizeVertexCache = true;
        while (arguments.read("--optimize-vertex-order") || arguments.read("--ovo")) optimizeVertexOrder = true;

        while (arguments.read("--build-mipmaps")) { modifyTextureSettings = true; buildImageMipmaps = true; }
        while (arguments.read("--compress")) { modifyTextureSettings = true; compressImages = true; }
        while (arguments.read("--disable-mipmaps")) { modifyTextureSettings = true; disableMipmaps = true; }

        while (arguments.read("--reallocate") || arguments.read("--ra") ) { reallocateMemory = true; }

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
            simplifier.setDoTriStrip(useTriStripVisitor);
            simplifier.setSmoothing(useSmoothingVisitor);
            node->accept(simplifier);
        }


        if (modifyTextureSettings)
        {
            OptimizeImageVisitor oiv(osgDB::Registry::instance()->getImageProcessor(), compressImages, buildImageMipmaps);
            node->accept(oiv);
        }


        if (removeDuplicateVertices)
        {
            OSG_NOTICE<<"Running osgUtil::IndexMeshVisitor"<<std::endl;
            osgUtil::IndexMeshVisitor imv;
            node->accept(imv);
            imv.makeMesh();
        }

        if (optimizeVertexCache)
        {
            OSG_NOTICE<<"Running osgUtil::VertexCacheVisitor"<<std::endl;
            osgUtil::VertexCacheVisitor vcv;
            node->accept(vcv);
            vcv.optimizeVertices();
        }

        if (optimizeVertexOrder)
        {
            OSG_NOTICE<<"Running osgUtil::VertexAccessOrderVisitor"<<std::endl;
            osgUtil::VertexAccessOrderVisitor vaov;
            node->accept(vaov);
            vaov.optimizeOrder();
        }

        if (modifyDrawableSettings)
        {
            OSG_NOTICE<<"Running StripStateVisitor"<<std::endl;
            StripStateVisitor ssv(true, useDisplayLists, useVBO);
            node->accept(ssv);
        }

        MemoryVisitor mv;
        node->accept(mv);
        mv.report(osg::notify(osg::NOTICE));

        if (reallocateMemory)
        {
            OSG_NOTICE<<"Running Reallocation of scene graph memory"<<std::endl;
            mv.reallocate();
        }

        mv.reset();
        node->accept(mv);
        mv.report(osg::notify(osg::NOTICE));

        return node;
    }

protected:

    void _init()
    {
        modifyDrawableSettings = false;
        useVBO = false;
        useDisplayLists = false;

        simplificatioRatio = 1.0;
        useTriStripVisitor = false;
        useSmoothingVisitor = false;

        removeDuplicateVertices = false;
        optimizeVertexCache = false;
        optimizeVertexOrder = false;

        reallocateMemory = false;
        
        modifyTextureSettings = false;
        buildImageMipmaps = false;
        compressImages = false;
        disableMipmaps = false;
    }

    bool modifyDrawableSettings;
    bool useVBO;
    bool useDisplayLists;

    float simplificatioRatio;
    bool useTriStripVisitor;
    bool useSmoothingVisitor;

    bool removeDuplicateVertices;
    bool optimizeVertexCache;
    bool optimizeVertexOrder;

    bool reallocateMemory;
    
    bool modifyTextureSettings;
    bool buildImageMipmaps;
    bool compressImages;
    bool disableMipmaps;

};
// 
class DatabasePagingOperation : public osg::Operation, public osgUtil::IncrementalCompileOperation::CompileCompletedCallback
{
public:

    DatabasePagingOperation(const std::string& filename,
                            const std::string& outputFilename,
                             SceneGraphProcessor* sceneGraphProcessor, 
                             osgUtil::IncrementalCompileOperation* ico):
        osg::Referenced(true),
        Operation("DatabasePaging Operation", false),
        _filename(filename),
        _outputFilename(outputFilename),
        _modelReadyToMerge(false),
        _sceneGraphProcessor(sceneGraphProcessor),
        _incrementalCompileOperation(ico)
        {
        }

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
                OSG_NOTICE<<"Writing out file "<<_outputFilename<<std::endl;
                
                osgDB::writeNodeFile(*_loadedModel, _outputFilename);
            }

            if (_incrementalCompileOperation.valid())
            {
                OSG_NOTICE<<"Registering with ICO "<<_outputFilename<<std::endl;

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

class TexturePoolHandler : public osgGA::GUIEventHandler
{
public:
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYUP)
        {
            if (ea.getKey()=='r')
            {
                osg::Texture::getTextureObjectManager(0)->reportStats(osg::notify(osg::NOTICE));
                osg::GLBufferObjectManager::getGLBufferObjectManager(0)->reportStats(osg::notify(osg::NOTICE));
            }
        }
        return false;
    }
};

struct ReportStatsAnimationCompletedCallback : public osgGA::AnimationPathManipulator::AnimationCompletedCallback
{
    virtual void completed(const osgGA::AnimationPathManipulator*)
    {
        OSG_NOTICE<<"Animation completed"<<std::endl;
        osg::Texture::getTextureObjectManager(0)->reportStats(osg::notify(osg::NOTICE));
        osg::GLBufferObjectManager::getGLBufferObjectManager(0)->reportStats(osg::notify(osg::NOTICE));
    }
};

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // set up camera manipulators
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        char keyForAnimationPath = '8';
        double animationSpeed = 1.0;
        while(arguments.read("--speed",animationSpeed) ) {}

        std::string pathfile;
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid())
            {
                apm->setTimeScale(animationSpeed);
                apm->setAnimationCompletedCallback(new ReportStatsAnimationCompletedCallback());
                
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // set up event handlers 
    {
        viewer.addEventHandler( new osgViewer::StatsHandler());
        viewer.addEventHandler( new osgViewer::WindowSizeHandler() );
        viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
        viewer.addEventHandler( new TexturePoolHandler() );
    }

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
    while (arguments.read("-o",outputPostfix)) { OSG_NOTICE<<"Set ouputPostfix to "<<outputPostfix<<std::endl; }


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
        OSG_NOTICE<<"No files loaded, please specify files on commandline."<<std::endl;
        return 1;
    }

    // load the models using a paging thread and use the incremental compile operation to
    // manage the compilation of GL objects without breaking frame.

    unsigned int modelIndex = 0;

    osg::ref_ptr<osg::OperationThread> databasePagingThread;
    osg::ref_ptr<DatabasePagingOperation> databasePagingOperation;

    databasePagingThread = new osg::OperationThread;
    databasePagingThread->startThread();


    osg::ref_ptr<osg::Group> group = new osg::Group;
    viewer.setSceneData(group.get());

    viewer.realize();

    std::string filename = fileNames[modelIndex++];
    std::string outputFilename = outputPostfix.empty() ? std::string() : osgDB::getStrippedName(filename)+outputPostfix;

    databasePagingOperation = new DatabasePagingOperation(
        filename,
        outputFilename,
        sceneGraphProcessor.get(),
        incrementalCompile.get());

    databasePagingThread->add(databasePagingOperation.get());


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
            std::string outputFilename = outputPostfix.empty() ? std::string() : osgDB::getStrippedName(filename)+outputPostfix;

            databasePagingOperation = new DatabasePagingOperation(
                filename,
                outputFilename,
                sceneGraphProcessor.get(),
                incrementalCompile.get());

            databasePagingThread->add(databasePagingOperation.get());
        }

        if (databasePagingOperation.get() && databasePagingOperation->_modelReadyToMerge)
        {
            OSG_NOTICE<<"Merging subgraph"<<std::endl;
            
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
