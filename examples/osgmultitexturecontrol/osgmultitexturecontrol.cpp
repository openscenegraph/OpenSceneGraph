/* OpenSceneGraph example, osgmultitexture.
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

#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgFX/MultiTextureControl>

#include <osgGA/TerrainManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgTerrain/Terrain>
#include <osgTerrain/TerrainTile>

#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>


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

/** Callback used to track the elevation of the camera and update the texture weights in an MultiTextureControl node.*/
class ElevationLayerBlendingCallback : public osg::NodeCallback
{
    public:
    
        typedef std::vector<double> Elevations;

        ElevationLayerBlendingCallback(osgFX::MultiTextureControl* mtc, const Elevations& elevations, float animationTime=4.0f):
            _previousFrame(0),
            _previousTime(0.0),
            _animationTime(animationTime),
            _currentElevation(0.0),
            _mtc(mtc),
            _elevations(elevations) {}
    
        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            if (nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
            {

                float deltaTime = 0.01f;
                if (_previousFrame!=0)
                {
                    deltaTime = float(nv->getFrameStamp()->getReferenceTime() - _previousTime);
                }

                _previousTime = nv->getFrameStamp()->getReferenceTime();
                _previousFrame = nv->getFrameStamp()->getFrameNumber();

                if (_mtc.valid() && !_elevations.empty())
                {
                    unsigned int index = _mtc->getNumTextureWeights()-1;
                    for(unsigned int i=0; i<_elevations.size(); ++i)
                    {
                        if (_currentElevation>_elevations[i]) 
                        {
                            index = i;
                            break;
                        }
                    }

                    float delta = std::min(deltaTime/_animationTime, 1.0f);

                    for(unsigned int i=0; i<_mtc->getNumTextureWeights(); ++i)
                    {
                        float currentValue = _mtc->getTextureWeight(i);
                        float desiredValue = (i==index) ? 1.0f : 0.0f;
                        if (desiredValue != currentValue)
                        {
                            if (currentValue<desiredValue)
                            {
                                desiredValue = std::min(currentValue + delta, desiredValue);
                            }
                            else
                            {
                                desiredValue = std::max(currentValue - delta, desiredValue);
                            }

                            _mtc->setTextureWeight(i, desiredValue);
                        }
                    }

                }
            }
            else if (nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
            {
                _currentElevation = nv->getViewPoint().z();

                osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>(node);
                if (csn) 
                {
                    osg::EllipsoidModel* em = csn->getEllipsoidModel();
                    if (em)
                    {
                        double X = nv->getViewPoint().x();
                        double Y = nv->getViewPoint().y();
                        double Z = nv->getViewPoint().z();
                        double latitude, longitude;
                        em->convertXYZToLatLongHeight(X,Y,Z,latitude, longitude, _currentElevation);
                    }
                }

            }

            traverse(node,nv);
        }

        unsigned int                                    _previousFrame;
        double                                          _previousTime;
        float                                           _animationTime;
        double                                          _currentElevation;
        
        osg::observer_ptr<osgFX::MultiTextureControl>   _mtc;
        Elevations                                      _elevations;
};


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

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->addCommandLineOption("-v","Set the terrain vertical scale.");
    arguments.getApplicationUsage()->addCommandLineOption("-r","Set the terrain sample ratio.");
    arguments.getApplicationUsage()->addCommandLineOption("--login <url> <username> <password>","Provide authentication information for http file access.");
   
    // construct the viewer.
    osgViewer::Viewer viewer(arguments);


    // set the tile loaded callback to load the optional imagery
    osg::ref_ptr<osgTerrain::WhiteListTileLoadedCallback> whiteList = new osgTerrain::WhiteListTileLoadedCallback;
    std::string setname;
    while(arguments.read("--allow",setname))
    {
        whiteList->allow(setname);
    }
    while(arguments.read("--allow-all"))
    {
        whiteList->setAllowAll(true);
    }
    osgTerrain::TerrainTile::setTileLoadedCallback(whiteList.get());


    // obtain the vertical scale
    float verticalScale = 1.0f;
    while(arguments.read("-v",verticalScale)) {}
    
    // obtain the sample ratio
    float sampleRatio = 1.0f;
    while(arguments.read("-r",sampleRatio)) {}


    // set up any authentication.
    std::string url, username, password;
    while(arguments.read("--login",url, username, password))
    {
        if (!osgDB::Registry::instance()->getAuthenticationMap())
        {
            osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
            osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(
                url,
                new osgDB::AuthenticationDetails(username, password)
            );
        }
    }

    // add all the event handlers to the viewer
    {
        // add the state manipulator
        viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

        // add the thread model handler
        viewer.addEventHandler(new osgViewer::ThreadingHandler);

        // add the window size toggle handler
        viewer.addEventHandler(new osgViewer::WindowSizeHandler);

        // add the stats handler
        viewer.addEventHandler(new osgViewer::StatsHandler);

        // add the help handler
        viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

        // add the record camera path handler
        viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

        // add the LOD Scale handler
        viewer.addEventHandler(new osgViewer::LODScaleHandler);
    }

    // add all the camera manipulators
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );

        unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        std::string pathfile;
        char keyForAnimationPath = '5';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid()) 
            {
                num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                ++keyForAnimationPath;
            }
        }

        keyswitchManipulator->selectMatrixManipulator(num);

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // set up the scene graph
    {
        // load the nodes from the commandline arguments.
        osg::Node* rootnode = osgDB::readNodeFiles(arguments);

        if (!rootnode)
        {
            osg::notify(osg::NOTICE)<<"Warning: no valid data loaded, please specify a database on the command line."<<std::endl;
            return 1;
        }

        osgTerrain::Terrain* terrain = findTopMostNodeOfType<osgTerrain::Terrain>(rootnode);
        if (!terrain)
        {
            terrain = new osgTerrain::Terrain;

            osg::CoordinateSystemNode* csn = findTopMostNodeOfType<osg::CoordinateSystemNode>(rootnode);
            if (csn)
            {
                terrain->set(*csn);
            }

            terrain->addChild(rootnode);

            rootnode = terrain;
            csn = terrain;
        }

        terrain->setSampleRatio(sampleRatio);
        terrain->setVerticalScale(verticalScale);

        // register our custom handler for adjust Terrain settings
        viewer.addEventHandler(new TerrainHandler(terrain));


        unsigned int numLayers = 1;
        osgFX::MultiTextureControl* mtc = findTopMostNodeOfType<osgFX::MultiTextureControl>(rootnode);
        if (mtc)
        {

            numLayers = mtc->getNumTextureWeights();

            // switch on just the first texture layer.
            mtc->setTextureWeight(0,1.0f);
            for(unsigned int i=1; i<numLayers; ++i)
            {
                mtc->setTextureWeight(i,0.0f);
            }
        }

        if (numLayers<2)
        {
            osg::notify(osg::NOTICE)<<"Warning: scene must have MultiTextureControl node with at least 2 texture units defined."<<std::endl;
            return 1;
        }

        double maxElevationTransition = 1e6;
        ElevationLayerBlendingCallback::Elevations elevations;
        for(unsigned int i=0; i<numLayers; ++i)
        {
            elevations.push_back(maxElevationTransition);
            maxElevationTransition /= 2.0;
        }


        // we must assign callback as both an update and cull callback, as update callback to do the update of
        // the osgFX::MultiTextureControl node a thread safe way, and as a cull callback to gather the camera
        // position information.
        ElevationLayerBlendingCallback* elbc = new ElevationLayerBlendingCallback(mtc, elevations);
        terrain->setUpdateCallback(elbc);
        terrain->setCullCallback(elbc);

        // add a viewport to the viewer and attach the scene graph.
        viewer.setSceneData( rootnode );
    }
    


    // create the windows and run the threads.
    viewer.realize();

    return viewer.run();
}
