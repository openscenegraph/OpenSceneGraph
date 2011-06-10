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
#include <osgTerrain/Layer>

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

    TerrainHandler(osgTerrain::Terrain* terrain):
        _terrain(terrain) {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
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

                return false;
            }
            default:
                return false;
        }
    }

protected:

    ~TerrainHandler() {}

    osg::ref_ptr<osgTerrain::Terrain>  _terrain;
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

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFiles(arguments);

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

    // register our custom handler for adjust Terrain settings
    viewer.addEventHandler(new TerrainHandler(terrain.get()));

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode.get() );


    // run the viewers main loop
    return viewer.run();

}
