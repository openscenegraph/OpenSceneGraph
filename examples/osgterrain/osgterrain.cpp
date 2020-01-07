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

#include <osg/ArgumentParser>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/GeometryTechnique>
#include <osgTerrain/DisplacementMappingTechnique>
#include <osgTerrain/Layer>

#include <osgFX/MultiTextureControl>


#include <iostream>

template<class T>
class FindTopMostNodeOfTypeVisitor : public osg::NodeVisitor
{
public:
    FindTopMostNodeOfTypeVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _foundNode(0)
    {}

    void apply(osg::Node& node)
    {
        T* result = dynamic_cast<T*>(&node);
        if (result)
        {
            _foundNode = result;
        }
        else
        {
            traverse(node);
        }
    }

    T* _foundNode;
};

template<class T>
T* findTopMostNodeOfType(osg::Node* node)
{
    if (!node) return 0;

    FindTopMostNodeOfTypeVisitor<T> fnotv;
    node->accept(fnotv);

    return fnotv._foundNode;
}

// class to handle events with a pick
class TerrainHandler : public osgGA::GUIEventHandler {
public:

    TerrainHandler(osgTerrain::Terrain* terrain, osgFX::MultiTextureControl* mtc):
        _terrain(terrain),
        _mtc(mtc) {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& /*aa*/)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                if (ea.getKey()=='r')
                {
                    _terrain->setSampleRatio(_terrain->getSampleRatio()*0.5);
                    osg::notify(osg::NOTICE)<<"Sample ratio "<<_terrain->getSampleRatio()<<std::endl;
                    return true;
                }
                else if (ea.getKey()=='R')
                {
                    _terrain->setSampleRatio(_terrain->getSampleRatio()/0.5);
                    osg::notify(osg::NOTICE)<<"Sample ratio "<<_terrain->getSampleRatio()<<std::endl;
                    return true;
                }
                else if (ea.getKey()=='v')
                {
                    _terrain->setVerticalScale(_terrain->getVerticalScale()*1.25);
                    osg::notify(osg::NOTICE)<<"Vertical scale "<<_terrain->getVerticalScale()<<std::endl;
                    return true;
                }
                else if (ea.getKey()=='V')
                {
                    _terrain->setVerticalScale(_terrain->getVerticalScale()/1.25);
                    osg::notify(osg::NOTICE)<<"Vertical scale "<<_terrain->getVerticalScale()<<std::endl;
                    return true;
                }
                else if (ea.getKey()=='!') // shift 1
                {
                    assignTextureWeightToSingleTextureUnit(1);
                    return true;
                }
                else if (ea.getKey()=='"') // shift 1
                {
                    assignTextureWeightToSingleTextureUnit(2);
                    return true;
                }
                else if (ea.getKey()==')') // shift 1
                {
                    assignTextureWeightToSingleTextureUnit(0);
                    return true;
                }
                else if (ea.getKey()=='A')
                {
                    assignedToAll();
                    return true;
                }
                else if (ea.getKey()=='l')
                {
                    toggleDefine("LIGHTING");
                    return true;
                }
                else if (ea.getKey()=='h')
                {
                    toggleDefine("HEIGHTFIELD_LAYER");
                    return true;
                }
                else if (ea.getKey()=='t')
                {
                    toggleDefine("TEXTURE_2D");
                    return true;
                }
                else if (ea.getKey()=='y')
                {
                    toggleDefine("COLOR_LAYER0");
                    return true;
                }
                else if (ea.getKey()=='u')
                {
                    toggleDefine("COLOR_LAYER1");
                    return true;
                }
                else if (ea.getKey()=='i')
                {
                    toggleDefine("COLOR_LAYER2");
                    return true;
                }
                else if (ea.getKey()=='d')
                {
                    toggleDefine("COMPUTE_DIAGONALS", osg::StateAttribute::OFF);
                    return true;
                }

                return false;
            }
            default:
                return false;
        }
    }

    void toggleDefine(const std::string& defineName, int expectedDefault=osg::StateAttribute::ON)
    {
        osg::StateSet::DefineList& defineList = _terrain->getOrCreateStateSet()->getDefineList();
        osg::StateSet::DefineList::iterator itr = defineList.find(defineName);
        if (itr==defineList.end())
        {
            defineList[defineName].second = (expectedDefault | osg::StateAttribute::OVERRIDE); // assume the defines start off.
            itr = defineList.find(defineName);
        }

        osg::StateSet::DefinePair& dp = itr->second;
        if ( (dp.second & osg::StateAttribute::ON)==0) dp.second = (osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        else dp.second = (osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);


    }

protected:

    ~TerrainHandler() {}

    void assignTextureWeightToSingleTextureUnit(unsigned int unit)
    {
        if (!_mtc) return;
        for(unsigned int i=0; i<_mtc->getNumTextureWeights(); ++i)
        {
            _mtc->setTextureWeight(i, (i==unit) ? 1.0f : 0.0f);
        }
    }

    void assignedToAll()
    {
        if (!_mtc && _mtc->getNumTextureWeights()>0) return;
        float div = 1.0f/static_cast<float>(_mtc->getNumTextureWeights());
        for(unsigned int i=0; i<_mtc->getNumTextureWeights(); ++i)
        {
            _mtc->setTextureWeight(i, div);
        }
    }

    osg::ref_ptr<osgTerrain::Terrain>           _terrain;
    osg::ref_ptr<osgFX::MultiTextureControl>    _mtc;
};

class CleanTechniqueReadFileCallback : public osgDB::ReadFileCallback
{

    public:

        class CleanTechniqueVisitor : public osg::NodeVisitor
        {
        public:
            CleanTechniqueVisitor():
                osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

            void apply(osg::Node& node)
            {
                osgTerrain::TerrainTile* tile = dynamic_cast<osgTerrain::TerrainTile*>(&node);
                if (tile)
                {
                    if (tile->getTerrainTechnique())
                    {
                        // OSG_NOTICE<<"Resetting TerrainTechnhique "<<tile->getTerrainTechnique()->className()<<" to 0"<<std::endl;
                        tile->setTerrainTechnique(0);
                    }
                }
                else
                {
                    traverse(node);
                }
            }
        };


        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
        {
            osgDB::ReaderWriter::ReadResult rr = ReadFileCallback::readNode(filename, options);
            if (rr.validNode())
            {
                CleanTechniqueVisitor ctv;
                rr.getNode()->accept(ctv);
            }
            return rr;
        }

    protected:
        virtual ~CleanTechniqueReadFileCallback() {}
};


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        std::string pathfile;
        char keyForAnimationPath = '5';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid())
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }


    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the record camera path handler
    viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // obtain the vertical scale
    float verticalScale = 1.0f;
    while(arguments.read("-v",verticalScale)) {}

    // obtain the sample ratio
    float sampleRatio = 1.0f;
    while(arguments.read("-r",sampleRatio)) {}

    osgTerrain::TerrainTile::BlendingPolicy blendingPolicy = osgTerrain::TerrainTile::INHERIT;
    std::string strBlendingPolicy;
    while(arguments.read("--blending-policy", strBlendingPolicy))
    {
        if (strBlendingPolicy == "INHERIT") blendingPolicy = osgTerrain::TerrainTile::INHERIT;
        else if (strBlendingPolicy == "DO_NOT_SET_BLENDING") blendingPolicy = osgTerrain::TerrainTile::DO_NOT_SET_BLENDING;
        else if (strBlendingPolicy == "ENABLE_BLENDING") blendingPolicy = osgTerrain::TerrainTile::ENABLE_BLENDING;
        else if (strBlendingPolicy == "ENABLE_BLENDING_WHEN_ALPHA_PRESENT") blendingPolicy = osgTerrain::TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT;
    }

    bool useDisplacementMappingTechnique = arguments.read("--dm");
    if (useDisplacementMappingTechnique)
    {
        osgDB::Registry::instance()->setReadFileCallback(new CleanTechniqueReadFileCallback());
    }

    bool setDatabaseThreadAffinity = false;
    unsigned int cpuNum = 0;
    while(arguments.read("--db-affinity", cpuNum)) { setDatabaseThreadAffinity = true; }

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> rootnode = osgDB::readRefNodeFiles(arguments);

    if (!rootnode)
    {
        osg::notify(osg::NOTICE)<<"Warning: no valid data loaded, please specify a database on the command line."<<std::endl;
        return 1;
    }

    osg::ref_ptr<osgTerrain::Terrain> terrain = findTopMostNodeOfType<osgTerrain::Terrain>(rootnode.get());
    if (!terrain)
    {
        // no Terrain node present insert one above the loaded model.
        terrain = new osgTerrain::Terrain;

        // if CoordinateSystemNode is present copy it's contents into the Terrain, and discard it.
        osg::CoordinateSystemNode* csn = findTopMostNodeOfType<osg::CoordinateSystemNode>(rootnode.get());
        if (csn)
        {
            terrain->set(*csn);
            for(unsigned int i=0; i<csn->getNumChildren();++i)
            {
                terrain->addChild(csn->getChild(i));
            }
        }
        else
        {
            terrain->addChild(rootnode.get());
        }

        rootnode = terrain.get();
    }

    terrain->setSampleRatio(sampleRatio);
    terrain->setVerticalScale(verticalScale);
    terrain->setBlendingPolicy(blendingPolicy);

    if (arguments.read("--equalize-boundaries") || arguments.read("-e"))
    {
        terrain->setEqualizeBoundaries(true);
    }


    if (useDisplacementMappingTechnique)
    {
        terrain->setTerrainTechniquePrototype(new osgTerrain::DisplacementMappingTechnique());
    }


    // register our custom handler for adjust Terrain settings
    viewer.addEventHandler(new TerrainHandler(terrain.get(), findTopMostNodeOfType<osgFX::MultiTextureControl>(rootnode.get())));

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode.get() );

    // if required set the DatabaseThread affinity, note must call after viewer.setSceneData() so that the osgViewer::Scene object is constructed with it's DatabasePager.
    if (setDatabaseThreadAffinity)
    {
        for (unsigned int i=0; i<viewer.getDatabasePager()->getNumDatabaseThreads(); ++i)
        {
            osgDB::DatabasePager::DatabaseThread* thread = viewer.getDatabasePager()->getDatabaseThread(i);
            thread->setProcessorAffinity(cpuNum);
            OSG_NOTICE<<"Settings affinity of DatabaseThread="<<thread<<" isRunning()="<<thread->isRunning()<<" cpuNum="<<cpuNum<<std::endl;
        }
    }

    // following are tests of the #pragma(tic) shader composition
    //terrain->getOrCreateStateSet()->setDefine("NUM_LIGHTS", "1");
    //terrain->getOrCreateStateSet()->setDefine("LIGHTING"); // , osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE);
    //terrain->getOrCreateStateSet()->setDefine("COMPUTE_DIAGONALS"); // , osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE);

    // run the viewers main loop
    return viewer.run();

}
