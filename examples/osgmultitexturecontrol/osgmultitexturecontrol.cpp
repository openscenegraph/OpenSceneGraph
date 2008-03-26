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

#include <osgTerrain/TerrainSystem>

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
            _previousFrame(-1),
            _previousTime(0.0),
            _mtc(mtc),
            _elevations(elevations),
            _animationTime(animationTime) {}
    
        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        { 
            if (!nv->getFrameStamp() || _previousFrame==nv->getFrameStamp()->getFrameNumber())
            {
                // we've already updated for this frame so no need to do it again, just traverse children.
                traverse(node,nv);
                return;
            }
            
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            
            float deltaTime = 0.01f;
            if (_previousFrame!=-1)
            {
                deltaTime = float(nv->getFrameStamp()->getReferenceTime() - _previousTime);
            }
            
            _previousTime = nv->getFrameStamp()->getReferenceTime();
            _previousFrame = nv->getFrameStamp()->getFrameNumber();

            double elevation = nv->getViewPoint().z();
        
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
                    em->convertXYZToLatLongHeight(X,Y,Z,latitude, longitude, elevation);
                }
            }
        
            if (_mtc.valid() && !_elevations.empty())
            {
                unsigned int index = _mtc->getNumTextureWeights()-1;
                for(unsigned int i=0; i<_elevations.size(); ++i)
                {
                    if (elevation>_elevations[i]) 
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
        
            traverse(node,nv);
        }

        int                                             _previousFrame;
        double                                          _previousTime;
        float                                           _animationTime;
        osg::observer_ptr<osgFX::MultiTextureControl>   _mtc;
        Elevations                                      _elevations;
        
        OpenThreads::Mutex                              _mutex;
};


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
   
    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);

    if (!rootnode)
    {
        osg::notify(osg::NOTICE)<<"Warning: no valid data loaded, please specify a database on the command line."<<std::endl;
        return 1;
    }
    
    osgTerrain::TerrainSystem* terrain = findTopMostNodeOfType<osgTerrain::TerrainSystem>(rootnode);
    if (!terrain)
    {
        terrain = new osgTerrain::TerrainSystem;
        terrain->addChild(rootnode);
        
        rootnode = terrain;
    }    
    
    osg::CoordinateSystemNode* csn = findTopMostNodeOfType<osg::CoordinateSystemNode>(rootnode);

    unsigned int numLayers = 1;
    osgFX::MultiTextureControl* mtc = findTopMostNodeOfType<osgFX::MultiTextureControl>(rootnode);
    if (mtc)
    {
        numLayers = mtc->getNumTextureWeights();
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
    
    ElevationLayerBlendingCallback* elbc = new ElevationLayerBlendingCallback(mtc, elevations);

    // assign to the most appropriate node (the CoordinateSystemNode is best as it provides the elevation on the globe.)
    if (csn) csn->setCullCallback(elbc);    
    else if (mtc) mtc->setCullCallback(elbc);
    else rootnode->setCullCallback(elbc);

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

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData( rootnode );
    
    // create the windows and run the threads.
    viewer.realize();

    return viewer.run();
}
