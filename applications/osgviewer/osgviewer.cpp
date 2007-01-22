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
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osgText/Text>

#include <osgViewer/Viewer>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <iostream>

class ThreadingHandler : public osgGA::GUIEventHandler 
{
public: 

    ThreadingHandler() {}
        
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;
    
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()=='m')
                {
                    switch(viewer->getThreadingModel())
                    {
                        case(osgViewer::Viewer::SingleThreaded):
                            viewer->setThreadingModel(osgViewer::Viewer::ThreadPerContext);
                            osg::notify(osg::NOTICE)<<"Threading model 'ThreadPerContext' selected."<<std::endl;
                            break;
                        case(osgViewer::Viewer::ThreadPerContext):
                            viewer->setThreadingModel(osgViewer::Viewer::ThreadPerCamera);
                            osg::notify(osg::NOTICE)<<"Threading model 'ThreadPerCamera' selected."<<std::endl;
                            break;
                        case(osgViewer::Viewer::ThreadPerCamera):
                            viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
                            osg::notify(osg::NOTICE)<<"Threading model 'SingleTheaded' selected."<<std::endl;
                            break;
                    }
                    return true;
                }
                if (ea.getKey()=='e')
                {
                    switch(viewer->getEndBarrierPosition())
                    {
                        case(osgViewer::Viewer::BeforeSwapBuffers):
                            viewer->setEndBarrierPosition(osgViewer::Viewer::AfterSwapBuffers);
                            osg::notify(osg::NOTICE)<<"Threading model 'AfterSwapBuffers' selected."<<std::endl;
                            break;
                        case(osgViewer::Viewer::AfterSwapBuffers):
                            viewer->setEndBarrierPosition(osgViewer::Viewer::BeforeSwapBuffers);
                            osg::notify(osg::NOTICE)<<"Threading model 'BeforeSwapBuffers' selected."<<std::endl;
                            break;
                    }
                    return true;
                }
            }
            default: break;
        }
        
        return false;
    }
    
    bool _done;
};

class StatsHandler : public osgGA::GUIEventHandler 
{
public: 

    StatsHandler():
        _statsType(NO_STATS),
        _frameRateChildNum(0),
        _viewerChildNum(0),
        _sceneChildNum(0) {}

    enum StatsType
    {
        NO_STATS = 0,
        FRAME_RATE = 1,
        VIEWER_STATS = 2,
        SCENE_STATS = 3,
        LAST = 4
    };
        
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;
    
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                if (ea.getKey()=='s')
                {
                    if (viewer->getStats())
                    {
                        if (!_camera.valid())
                        {
                            setUpCamera(viewer);
                            setUpScene(viewer);
                        }

                        ++_statsType;
                        
                        if (_statsType==LAST) _statsType = NO_STATS;
                        
                        switch(_statsType)
                        {
                            case(NO_STATS):
                            {
                                _camera->setNodeMask(0x0); 
                                _switch->setAllChildrenOff();
                                break;
                            }
                            case(FRAME_RATE):
                            {
                                _camera->setNodeMask(0xffffffff);
                                _switch->setValue(_frameRateChildNum, true);
                                break;
                            }
                            case(VIEWER_STATS):
                            {
                                _camera->setNodeMask(0xffffffff);
                                _switch->setValue(_viewerChildNum, true);
                                break;
                            }
                            case(SCENE_STATS):
                            {
                                _switch->setValue(_sceneChildNum, true);
                                _camera->setNodeMask(0xffffffff);
                                break;
                            }
                            default:
                                break;
                        }

                        
                    }
                    return true;
                }
                if (ea.getKey()=='S')
                {
                    if (viewer->getStats())
                    {
                        osg::notify(osg::NOTICE)<<std::endl<<"Stats report:"<<std::endl;
                        typedef std::vector<osg::Stats*> StatsList;
                        StatsList statsList;
                        statsList.push_back(viewer->getStats());
                        
                        osgViewer::Viewer::Contexts contexts;
                        viewer->getContexts(contexts);
                        for(osgViewer::Viewer::Contexts::iterator gcitr = contexts.begin();
                            gcitr != contexts.end();
                            ++gcitr)
                        {
                            osg::GraphicsContext::Cameras& cameras = (*gcitr)->getCameras();
                            for(osg::GraphicsContext::Cameras::iterator itr = cameras.begin();
                                itr != cameras.end();
                                ++itr)
                            {
                                if ((*itr)->getStats())
                                {
                                    statsList.push_back((*itr)->getStats());
                                }
                            }
                        }
                        
                        for(int i = viewer->getStats()->getEarliestFrameNumber(); i<= viewer->getStats()->getLatestFrameNumber()-1; ++i)
                        {
                            for(StatsList::iterator itr = statsList.begin();
                                itr != statsList.end();
                                ++itr)
                            {
                                if (itr==statsList.begin()) (*itr)->report(osg::notify(osg::NOTICE), i);
                                else (*itr)->report(osg::notify(osg::NOTICE), i, "    ");
                            }
                            osg::notify(osg::NOTICE)<<std::endl;
                        }
                        
                    }
                    return true;
                }
            }
            default: break;
        }
        
        return false;
        
    }

    void setUpCamera(osgViewer::Viewer* viewer)
    {
        osgViewer::Viewer::Windows windows;
        viewer->getWindows(windows);
        
        if (windows.empty()) return;
        
        osgViewer::GraphicsWindow* window = windows.front();
        
        _camera = new osg::Camera;
        _camera->setGraphicsContext(window);
        _camera->setViewport(0, 0, window->getTraits()->width, window->getTraits()->height);
        
        _camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
        _camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        _camera->setViewMatrix(osg::Matrix::identity());

        // only clear the depth buffer
        _camera->setClearMask(0);
        
        viewer->setUpRenderingSupport();
    }
    
    struct TextDrawCallback : public virtual osg::Drawable::DrawCallback
    {
        TextDrawCallback(osg::Stats* stats, const std::string name, int frameDelta, double multiplier = 1.0):
            _stats(stats),
            _attributeName(name),
            _frameDelta(frameDelta),
            _multiplier(multiplier) {}
    
        /** do customized draw code.*/
        virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
        {
            osgText::Text* text = (osgText::Text*)drawable;
            
            int frameNumber = renderInfo.getState()->getFrameStamp()->getFrameNumber();
            
            double value;
            if (_stats->getAttribute( frameNumber+_frameDelta, _attributeName, value))
            {
                sprintf(_tmpText,"%4.2f",value * _multiplier);
                text->setText(_tmpText);
            }
                        
            text->drawImplementation(renderInfo);
        }

        osg::Stats*     _stats;
        std::string     _attributeName;
        int             _frameDelta;
        double          _multiplier;
        mutable char    _tmpText[128];
    };

    void setUpScene(osgViewer::Viewer* viewer)
    {
        _switch = new osg::Switch;
        _camera->addChild(_switch.get());

        osg::StateSet* stateset = _switch->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

        std::string font("fonts/arial.ttf");

        float leftPos = 10.0f;
        float characterSize = 20.0f;

        osg::Vec3 pos(leftPos,1000.0f,0.0f);

        osg::Vec4 colorFR(1.0f,1.0f,1.0f,1.0f);
        osg::Vec4 colorUpdate( 0.0f,1.0f,0.0f,1.0f);
        osg::Vec4 colorCull( 0.0f,1.0f,1.0f,1.0f);
        osg::Vec4 colorDraw( 1.0f,1.0f,0.0f,1.0f);
        osg::Vec4 colorGPU( 1.0f,0.5f,0.0f,1.0f);

        // frame rate stats
        {
            osg::Geode* geode = new osg::Geode();

            osg::ref_ptr<osgText::Text> frameRateLabel = new osgText::Text;
            geode->addDrawable( frameRateLabel.get() );

            frameRateLabel->setColor(colorFR);
            frameRateLabel->setFont(font);
            frameRateLabel->setCharacterSize(characterSize);
            frameRateLabel->setPosition(pos);
            frameRateLabel->setText("Frame Rate: ");

            pos.x() = frameRateLabel->getBound().xMax();

            _frameRateValue = new osgText::Text;
            geode->addDrawable( _frameRateValue.get() );

            _frameRateValue->setColor(colorFR);
            _frameRateValue->setFont(font);
            _frameRateValue->setCharacterSize(characterSize);
            _frameRateValue->setPosition(pos);
            _frameRateValue->setText("0.0");

            _frameRateValue->setDrawCallback(new TextDrawCallback(viewer->getStats(),"Frame rate",-1));

            pos.y() -= characterSize*1.5f;
            
            _frameRateChildNum = _switch->getNumChildren();
            _switch->addChild(geode, false);
        }
        

        // viewer stats
        {
            osg::Geode* geode = new osg::Geode();


            {
                pos.x() = leftPos;

                osg::ref_ptr<osgText::Text> eventLabel = new osgText::Text;
                geode->addDrawable( eventLabel.get() );

                eventLabel->setColor(colorUpdate);
                eventLabel->setFont(font);
                eventLabel->setCharacterSize(characterSize);
                eventLabel->setPosition(pos);
                eventLabel->setText("Event: ");

                pos.x() = eventLabel->getBound().xMax();

                _eventValue = new osgText::Text;
                geode->addDrawable( _eventValue.get() );

                _eventValue->setColor(colorUpdate);
                _eventValue->setFont(font);
                _eventValue->setCharacterSize(characterSize);
                _eventValue->setPosition(pos);
                _eventValue->setText("0.0");

                _eventValue->setDrawCallback(new TextDrawCallback(viewer->getStats(),"Event traversal time taken",-1, 1000.0));

                pos.y() -= characterSize*1.5f;
            }
            
            {
                pos.x() = leftPos;

                osg::ref_ptr<osgText::Text> updateLabel = new osgText::Text;
                geode->addDrawable( updateLabel.get() );

                updateLabel->setColor(colorUpdate);
                updateLabel->setFont(font);
                updateLabel->setCharacterSize(characterSize);
                updateLabel->setPosition(pos);
                updateLabel->setText("Update: ");

                pos.x() = updateLabel->getBound().xMax();

                _updateValue = new osgText::Text;
                geode->addDrawable( _updateValue.get() );

                _updateValue->setColor(colorUpdate);
                _updateValue->setFont(font);
                _updateValue->setCharacterSize(characterSize);
                _updateValue->setPosition(pos);
                _updateValue->setText("0.0");

                _updateValue->setDrawCallback(new TextDrawCallback(viewer->getStats(),"Update traversal time taken",-1, 1000.0));

                pos.y() -= characterSize*1.5f;
            }
            
            
            {
                pos.x() = leftPos;

                osg::ref_ptr<osgText::Text> updateLabel = new osgText::Text;
                geode->addDrawable( updateLabel.get() );

                updateLabel->setColor(colorDraw);
                updateLabel->setFont(font);
                updateLabel->setCharacterSize(characterSize);
                updateLabel->setPosition(pos);
                updateLabel->setText("Rendering: ");

                pos.x() = updateLabel->getBound().xMax();

                _renderingValue = new osgText::Text;
                geode->addDrawable( _renderingValue.get() );

                _renderingValue->setColor(colorDraw);
                _renderingValue->setFont(font);
                _renderingValue->setCharacterSize(characterSize);
                _renderingValue->setPosition(pos);
                _renderingValue->setText("0.0");

                _renderingValue->setDrawCallback(new TextDrawCallback(viewer->getStats(),"Rendering traversals time taken",-1, 1000.0));

                pos.y() -= characterSize*1.5f;
            }

            _viewerChildNum = _switch->getNumChildren();
            _switch->addChild(geode, false);
        }

        // scene stats
        {
            pos.x() = leftPos;

            osg::Geode* geode = new osg::Geode();

            {
                osgText::Text* text = new  osgText::Text;
                geode->addDrawable( text );

                text->setFont(font);
                text->setFont(font);
                text->setCharacterSize(characterSize);
                text->setPosition(pos);
                text->setText("Scene Stats to do...");

                pos.y() -= characterSize*1.5f;
            }    

            _sceneChildNum = _switch->getNumChildren();
            _switch->addChild(geode, false);
        }
    }
    

    int                             _statsType;
    osg::ref_ptr<osg::Camera>       _camera;
    osg::ref_ptr<osg::Switch>       _switch;
    
    unsigned int                    _frameRateChildNum;
    osg::ref_ptr<osgText::Text>     _frameRateValue;
    osg::ref_ptr<osgText::Text>     _eventValue;
    osg::ref_ptr<osgText::Text>     _updateValue;
    osg::ref_ptr<osgText::Text>     _renderingValue;

    unsigned int                    _viewerChildNum;
    
    unsigned int                    _sceneChildNum;
    
};

int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line parameters");
    arguments.getApplicationUsage()->addCommandLineOption("--help-env","Display environmental variables available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-keys","Display keyboard & mouse bindings available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-all","Display all command line, env vars and keyboard & mouse bindings.");

    // if user request help write it out to cout.
    bool helpAll = arguments.read("--help-all");
    unsigned int helpType = ((helpAll || arguments.read("-h") || arguments.read("--help"))? osg::ApplicationUsage::COMMAND_LINE_OPTION : 0 ) |
                            ((helpAll ||  arguments.read("--help-env"))? osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE : 0 ) |
                            ((helpAll ||  arguments.read("--help-keys"))? osg::ApplicationUsage::KEYBOARD_MOUSE_BINDING : 0 );
    if (helpType)
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

    osgViewer::Viewer viewer;
    
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
    {
        osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
        statesetManipulator->setStateSet(viewer.getCamera()->getOrCreateStateSet());

        viewer.addEventHandler( statesetManipulator.get() );
    }
    
    // add the thread model handler
    viewer.addEventHandler(new ThreadingHandler);

    // add the stats handler
    viewer.addEventHandler(new StatsHandler);

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

    return viewer.run();
}
