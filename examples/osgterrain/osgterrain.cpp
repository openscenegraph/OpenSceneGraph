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

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/CoordinateSystemNode>
#include <osg/ClusterCullingCallback>
#include <osg/ArgumentParser>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgText/FadeText>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgTerrain/TerrainNode>
#include <osgTerrain/GeometryTechnique>
#include <osgTerrain/Layer>

#include <iostream>


class FilterHandler : public osgGA::GUIEventHandler 
{
    public: 

        FilterHandler(osgTerrain::GeometryTechnique* gt):
            _gt(gt) {}

        bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
        {
            if (!_gt) return false;
            
            switch(ea.getEventType())
            {
            case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey() == 'g')
                    {
                        osg::notify(osg::NOTICE)<<"Gaussian"<<std::endl;
                        _gt->setFilterMatrixAs(osgTerrain::GeometryTechnique::GAUSSIAN);
                        return true;
                    }
                    else if (ea.getKey() == 's')
                    {
                        osg::notify(osg::NOTICE)<<"Smooth"<<std::endl;
                        _gt->setFilterMatrixAs(osgTerrain::GeometryTechnique::SMOOTH);
                        return true;
                    }
                    else if (ea.getKey() == 'S')
                    {
                        osg::notify(osg::NOTICE)<<"Sharpen"<<std::endl;
                        _gt->setFilterMatrixAs(osgTerrain::GeometryTechnique::SHARPEN);
                        return true;
                    }
                    else if (ea.getKey() == '+')
                    {
                        _gt->setFilterWidth(_gt->getFilterWidth()*1.1);
                        osg::notify(osg::NOTICE)<<"Filter width = "<<_gt->getFilterWidth()<<std::endl;
                        return true;
                    }
                    else if (ea.getKey() == '-')
                    {
                        _gt->setFilterWidth(_gt->getFilterWidth()/1.1);
                        osg::notify(osg::NOTICE)<<"Filter width = "<<_gt->getFilterWidth()<<std::endl;
                        return true;
                    }
                    else if (ea.getKey() == '>')
                    {
                        _gt->setFilterBias(_gt->getFilterBias()+0.1);
                        osg::notify(osg::NOTICE)<<"Filter bias = "<<_gt->getFilterBias()<<std::endl;
                        return true;
                    }
                    else if (ea.getKey() == '<')
                    {
                        _gt->setFilterBias(_gt->getFilterBias()-0.1);
                        osg::notify(osg::NOTICE)<<"Filter bias = "<<_gt->getFilterBias()<<std::endl;
                        return true;
                    }
                    break;
                }
            default:
                break;
            }
            return false;
            
        }

    protected:
    
        osg::observer_ptr<osgTerrain::GeometryTechnique> _gt;

};



class LayerHandler : public osgGA::GUIEventHandler 
{
    public: 

        LayerHandler(osgTerrain::Layer* layer):
            _layer(layer) {}

        bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
        {
            if (!_layer) return false;
            
            float scale = 1.2;
            
            switch(ea.getEventType())
            {
            case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey() == 'q')
                    {
                        _layer->transform(0.0, scale);
                        return true;
                    }
                    else if (ea.getKey() == 'a')
                    {
                        _layer->transform(0.0, 1.0f/scale);
                        return true;
                    }
                    break;
                }
            default:
                break;
            }
            return false;
            
        }

    protected:

        osg::observer_ptr<osgTerrain::Layer> _layer;

};

#if 1

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


    double x = 0.0;
    double y = 0.0;
    double w = 1.0;
    double h = 1.0;
    
    osg::ref_ptr<osg::Node> test = osgDB::readNodeFiles(arguments);
    
    if (!test.valid()) return 0;
    
    viewer.setSceneData(test.get());
    viewer.realize();

    while (!viewer.done())
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        viewer.frame();
    }
    
    return 0;

    osg::ref_ptr<osgTerrain::TerrainNode> terrain = new osgTerrain::TerrainNode;
    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::EllipsoidLocator(-osg::PI, -osg::PI*0.5, 2.0*osg::PI, osg::PI, 0.0);
    osg::ref_ptr<osgTerrain::ValidDataOperator> validDataOperator = new osgTerrain::NoDataValue(0.0);
    osg::ref_ptr<osgTerrain::Layer> lastAppliedLayer;

    unsigned int layerNum = 0;

    std::string filterName;

    bool readParameter = false;
    float minValue, maxValue;
    float scale = 1.0f;
    float offset = 0.0f;


    int pos = 1;
    while(pos<arguments.argc())
    {
        readParameter = false;
        std::string filename;
        
        if (arguments.read(pos, "--layer",layerNum)) 
        {
            osg::notify(osg::NOTICE)<<"Set layer number to "<<layerNum<<std::endl;
            readParameter = true;
        }

        else if (arguments.read(pos, "-b"))
        {
            terrain->setTreatBoundariesToValidDataAsDefaultValue(true);
        }
        
        else if (arguments.read(pos, "-e",x,y,w,h))
        {
            // define the extents.
            locator = new osgTerrain::EllipsoidLocator(x,y,w,h,0);
            readParameter = true;
        }

        else if (arguments.read(pos, "--transform",offset, scale) || arguments.read(pos, "-t",offset, scale))
        {
            // define the extents.
            readParameter = true;
        }

        else if (arguments.read(pos, "--cartesian",x,y,w,h))
        {
            // define the extents.
            locator = new osgTerrain::CartesianLocator(x,y,w,h,0);
            readParameter = true;
        }

        else if (arguments.read(pos, "--hf",filename))
        {
            readParameter = true;
            
            osg::notify(osg::NOTICE)<<"--hf "<<filename<<std::endl;

            osg::ref_ptr<osg::HeightField> hf = osgDB::readHeightFieldFile(filename);
            if (hf.valid())
            {
                osg::ref_ptr<osgTerrain::HeightFieldLayer> hfl = new osgTerrain::HeightFieldLayer;
                hfl->setHeightField(hf.get());
                
                hfl->setLocator(locator.get());
                hfl->setValidDataOperator(validDataOperator.get());
                
                if (offset!=0.0f || scale!=1.0f)
                {
                    hfl->transform(offset,scale);
                }
                
                terrain->setElevationLayer(hfl.get());
                
                lastAppliedLayer = hfl.get();
                
                osg::notify(osg::NOTICE)<<"created osgTerrain::HeightFieldLayer"<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"failed to create osgTerrain::HeightFieldLayer"<<std::endl;
            }
            
            scale = 1.0f;
            offset = 0.0f;
            
        }

        else if (arguments.read(pos, "-d",filename) || arguments.read(pos, "--elevation-image",filename))
        {
            readParameter = true;
            osg::notify(osg::NOTICE)<<"--elevation-image "<<filename<<std::endl;

            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
            if (image.valid())
            {
                osg::ref_ptr<osgTerrain::ImageLayer> imageLayer = new osgTerrain::ImageLayer;
                imageLayer->setImage(image.get());
                imageLayer->setLocator(locator.get());
                imageLayer->setValidDataOperator(validDataOperator.get());
                
                if (offset!=0.0f || scale!=1.0f)
                {
                    imageLayer->transform(offset,scale);
                }
                
                terrain->setElevationLayer(imageLayer.get());
                
                lastAppliedLayer = imageLayer.get();

                osg::notify(osg::NOTICE)<<"created Elevation osgTerrain::ImageLayer"<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"failed to create osgTerrain::ImageLayer"<<std::endl;
            }

            scale = 1.0f;
            offset = 0.0f;
            
        }
        
        else if (arguments.read(pos, "-c",filename) || arguments.read(pos, "--image",filename))
        {
            readParameter = true;
            osg::notify(osg::NOTICE)<<"--image "<<filename<<" x="<<x<<" y="<<y<<" w="<<w<<" h="<<h<<std::endl;

            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
            if (image.valid())
            {
                osg::ref_ptr<osgTerrain::ImageLayer> imageLayer = new osgTerrain::ImageLayer;
                imageLayer->setImage(image.get());
                imageLayer->setLocator(locator.get());
                imageLayer->setValidDataOperator(validDataOperator.get());
                
                if (offset!=0.0f || scale!=1.0f)
                {
                    imageLayer->transform(offset,scale);
                }

                terrain->setColorLayer(layerNum, imageLayer.get());

                lastAppliedLayer = imageLayer.get();

                osg::notify(osg::NOTICE)<<"created Color osgTerrain::ImageLayer"<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"failed to create osgTerrain::ImageLayer"<<std::endl;
            }

            scale = 1.0f;
            offset = 0.0f;
            
        }

        else if (arguments.read(pos, "--filter",filterName))
        {
            readParameter = true;
            
            if (filterName=="NEAREST")
            {
                osg::notify(osg::NOTICE)<<"--filter "<<filterName<<std::endl;
                terrain->setColorFilter(layerNum, osgTerrain::TerrainNode::NEAREST);
            }
            else if (filterName=="LINEAR") 
            {
                osg::notify(osg::NOTICE)<<"--filter "<<filterName<<std::endl;
                terrain->setColorFilter(layerNum, osgTerrain::TerrainNode::LINEAR);
            }
            else
            {
                osg::notify(osg::NOTICE)<<"--filter "<<filterName<<" unrecognized filter name, please use LINEAER or NEAREST."<<std::endl;
            }            
            
        }

        else if (arguments.read(pos, "--tf",minValue, maxValue))
        {
            readParameter = true;

            osg::ref_ptr<osg::TransferFunction1D> tf = new osg::TransferFunction1D;
            
            tf->setInputRange(minValue, maxValue);
            
            tf->allocate(6);
            tf->setValue(0, osg::Vec4(1.0,1.0,1.0,1.0));
            tf->setValue(1, osg::Vec4(1.0,0.0,1.0,1.0));
            tf->setValue(2, osg::Vec4(1.0,0.0,0.0,1.0));
            tf->setValue(3, osg::Vec4(1.0,1.0,0.0,1.0));
            tf->setValue(4, osg::Vec4(0.0,1.0,1.0,1.0));
            tf->setValue(5, osg::Vec4(0.0,1.0,0.0,1.0));
            
            osg::notify(osg::NOTICE)<<"--tf "<<minValue<<" "<<maxValue<<std::endl;

            terrain->setColorTransferFunction(layerNum, tf.get());
        }
        else
        {
            ++pos;
        }

    }
    

    osg::ref_ptr<osgTerrain::GeometryTechnique> geometryTechnique = new osgTerrain::GeometryTechnique;
    
    terrain->setTerrainTechnique(geometryTechnique.get());
    
    viewer.addEventHandler(new FilterHandler(geometryTechnique.get()));
    
    viewer.addEventHandler(new LayerHandler(lastAppliedLayer.get()));

    if (!terrain) return 0;
    
    // return 0;

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(terrain.get());

    return viewer.run();
}

#else

#include <osg/OperationsThread>
#include <osg/Notify>

struct MyOperation : public osg::Operation
{

    MyOperation(const std::string& str, bool keep, double number):
        osg::Operation(str,keep),
        _number(number) {}

    virtual void operator () (osg::Object* object)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        
        osg::notify(osg::NOTICE)<<getName()<<" "<<_number<<" object="<<object<<std::endl;
        ++_number;
        if (_number>1000) setKeep(false);
    }
    
    OpenThreads::Mutex _mutex;
    double _number;
}; 



int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osg::ref_ptr<osg::OperationQueue> operations = new osg::OperationQueue;
    operations->add(new MyOperation("one ",true, 0.0));
    operations->add(new MyOperation("two ",true, 0.0));
    
    {
        typedef std::list< osg::ref_ptr<osg::OperationsThread> > Threads;
        Threads threads;

        unsigned int numberThreads = 2;
        while (arguments.read("--threads",numberThreads)) {}

        for(unsigned int i=0; i<numberThreads; ++i)    
        {
            osg::ref_ptr<osg::OperationsThread> thread = new osg::OperationsThread;
            thread->setOperationQueue(operations.get());
            threads.push_back(thread.get());
            thread->startThread();
        }

        while(!operations->empty())
        {
            osg::notify(osg::NOTICE)<<"Main loop"<<std::endl;
            osg::ref_ptr<osg::Operation> operation = operations->getNextOperation();
            if (operation.valid()) (*operation)(0);
        }

        osg::notify(osg::NOTICE)<<"Completed main loop ******************************* "<<std::endl;
    }
    
    osg::notify(osg::NOTICE)<<"Exiting main -------------------------------- "<<std::endl;

    return 0;

}

#endif
