/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <iostream>
#include <sstream>

class WindowCaptureCallback : public osg::Camera::DrawCallback
{
    public:
    
        struct ContextData : public osg::Referenced
        {
        
            ContextData(osg::GraphicsContext* gc, const std::string& name):
                _gc(gc),
                _fileName(name),
                _pixelFormat(GL_RGB),
                _type(GL_UNSIGNED_BYTE),
                _width(0),
                _height(0),
                _currentImageIndex(0),
                _currentPboIndex(0)
            {
                getSize(gc, _width, _height);
                
                std::cout<<"Window size "<<_width<<", "<<_height<<std::endl;
            
                // single buffered image
                _imageBuffer.push_back(new osg::Image);
                
                // double buffer PBO.
                _pboBuffer.push_back(new osg::PixelBufferObject);
                _pboBuffer.push_back(new osg::PixelBufferObject);
            }
            
            void getSize(osg::GraphicsContext* gc, int& width, int& height)
            {
                if (gc->getTraits())
                {
                    width = gc->getTraits()->width;
                    height = gc->getTraits()->height;
                }
            }
            
            void read()
            {
                std::cout<<"Read to "<<_fileName<<" image "<<_currentImageIndex<<" "<<_currentPboIndex<<std::endl;
                
                unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();
                unsigned int nextPboIndex = (_currentPboIndex+1)%_pboBuffer.size();

                int width=0, height=0;
                getSize(_gc, width, height);
                if (width!=_width || _height!=height)
                {
                    std::cout<<"   Window resized "<<width<<", "<<height<<std::endl;
                    _width = width;
                    _height = height;
                }
                
                osg::Image* image = _imageBuffer[_currentImageIndex].get();
                
                image->readPixels(0,0,_width,_height,
                                  _pixelFormat,_type);
                                   
                if (!_fileName.empty())
                {
                    osgDB::writeImageFile(*image, _fileName);
                }
                
                _currentImageIndex = nextImageIndex;
                _currentPboIndex = nextPboIndex;

            }
        
            typedef std::vector< osg::ref_ptr<osg::Image> >             ImageBuffer;
            typedef std::vector< osg::ref_ptr<osg::PixelBufferObject> > PBOBuffer;
        
            osg::GraphicsContext*   _gc;
            std::string             _fileName;
            
            GLenum                  _pixelFormat;
            GLenum                  _type;
            int                     _width;
            int                     _height;
            
            unsigned int            _currentImageIndex;
            ImageBuffer             _imageBuffer;
            
            unsigned int            _currentPboIndex;
            PBOBuffer               _pboBuffer;
        };
    
        WindowCaptureCallback()
        {
        }

        ContextData* createContextData(osg::GraphicsContext* gc) const
        {
            std::stringstream filename;
            filename << "test_"<<_contextDataMap.size()<<".jpg";
            return new ContextData(gc,filename.str());
        }
        
        ContextData* getContextData(osg::GraphicsContext* gc) const
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
            osg::ref_ptr<ContextData>& data = _contextDataMap[gc];
            if (!data) data = createContextData(gc);
            
            return data.get();
        }

        virtual void operator () (osg::RenderInfo& renderInfo) const
        {
            osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
            osg::notify(osg::NOTICE)<<"Capture screen image "<<gc<<std::endl;
            
            osg::ref_ptr<ContextData> cd = getContextData(gc);
            cd->read();
        }
        
        typedef std::map<osg::GraphicsContext*, osg::ref_ptr<ContextData> > ContextDataMap;
        
        mutable OpenThreads::Mutex  _mutex;
        mutable ContextDataMap      _contextDataMap;
        
};

void addCallbackToViewer(osgViewer::ViewerBase& viewer, WindowCaptureCallback* callback)
{
    osgViewer::ViewerBase::Windows windows;
    viewer.getWindows(windows);
    for(osgViewer::ViewerBase::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)
    {
        osgViewer::GraphicsWindow* window = *itr;
        osg::GraphicsContext::Cameras& cameras = window->getCameras();
        osg::Camera* lastCamera = 0;
        for(osg::GraphicsContext::Cameras::iterator cam_itr = cameras.begin();
            cam_itr != cameras.end();
            ++cam_itr)
        {
            if (lastCamera)
            {
                if ((*cam_itr)->getRenderOrder() > (*cam_itr)->getRenderOrder())
                {
                    lastCamera = (*cam_itr);
                }
                if ((*cam_itr)->getRenderOrder() == lastCamera->getRenderOrder() &&
                    (*cam_itr)->getRenderOrderNum() >= lastCamera->getRenderOrderNum())
                {
                    lastCamera = (*cam_itr);
                }
            }
            else
            {
                lastCamera = *cam_itr;
            }
        }
        
        if (lastCamera)
        {
            osg::notify(osg::NOTICE)<<"Last camera "<<lastCamera<<std::endl;
            
            lastCamera->setFinalDrawCallback(callback);
        }
        else
        {
            osg::notify(osg::NOTICE)<<"No camera found"<<std::endl;
        }
    }
}

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");

    osgViewer::Viewer viewer(arguments);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }
    
    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

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

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    viewer.setSceneData( loadedModel.get() );

    viewer.realize();
    
    addCallbackToViewer(viewer, new WindowCaptureCallback);

    return viewer.run();

}
