/*
 * OsgMainApp.hpp
 *
 *  Created on: 29/05/2011
 *      Author: Jorge Izquierdo Ciges
 */

#ifndef OSGMAINAPP_HPP_
#define OSGMAINAPP_HPP_

//Android log
#include <android/log.h>
#include <iostream>
#include <cstdlib>
#include <math.h>

//Standard libraries
#include <string>

//osg
#include <osg/GL>
#include <osg/GLExtensions>
#include <osg/Depth>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Node>
#include <osg/Notify>
//osgText
#include <osgText/Text>
//osgDB
#include <osgDB/DatabasePager>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
//osg_viewer
#include <osgViewer/Viewer>
#include <osgViewer/Renderer>
#include <osgViewer/ViewerEventHandlers>
//osgGA
#include <osgGA/GUIEventAdapter>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>
//Self headers
#include "OsgAndroidNotifyHandler.hpp"

//Static plugins Macro
USE_OSGPLUGIN(ive)
USE_OSGPLUGIN(osg)
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(terrain)
USE_OSGPLUGIN(rgb)
USE_OSGPLUGIN(OpenFlight)
USE_OSGPLUGIN(dds)
//Static DOTOSG
USE_DOTOSGWRAPPER_LIBRARY(osg)
USE_DOTOSGWRAPPER_LIBRARY(osgFX)
USE_DOTOSGWRAPPER_LIBRARY(osgParticle)
USE_DOTOSGWRAPPER_LIBRARY(osgTerrain)
USE_DOTOSGWRAPPER_LIBRARY(osgText)
USE_DOTOSGWRAPPER_LIBRARY(osgViewer)
USE_DOTOSGWRAPPER_LIBRARY(osgVolume)
//Static serializer
USE_SERIALIZER_WRAPPER_LIBRARY(osg)
USE_SERIALIZER_WRAPPER_LIBRARY(osgAnimation)
USE_SERIALIZER_WRAPPER_LIBRARY(osgFX)
USE_SERIALIZER_WRAPPER_LIBRARY(osgManipulator)
USE_SERIALIZER_WRAPPER_LIBRARY(osgParticle)
USE_SERIALIZER_WRAPPER_LIBRARY(osgTerrain)
USE_SERIALIZER_WRAPPER_LIBRARY(osgText)
USE_SERIALIZER_WRAPPER_LIBRARY(osgVolume)

#define  LOG_TAG    "osgNativeLib"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

struct Model{
    std::string filename;
    std::string name;
};

class OsgMainApp{
private:
    osg::ref_ptr<osgViewer::Viewer> _viewer;
    osg::ref_ptr<osg::Group> _root;
    osg::ref_ptr<osg::StateSet> _state;
    osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> _manipulator;

    float _lodScale;
    unsigned int _prevFrame;

    bool _initialized;
    bool _clean_scene;

    OsgAndroidNotifyHandler *_notifyHandler;

    std::vector<Model> _vModels;
    std::vector<Model> _vModelsToLoad;
    std::vector<Model> _vModelsToDelete;

    void loadModels();
    void deleteModels();

public:
    OsgMainApp();
    ~OsgMainApp();

    //Initialization function
    void initOsgWindow(int x,int y,int width,int height);
    //Draw
    void draw();
    //Events
    void mouseButtonPressEvent(float x,float y,int button);
    void mouseButtonReleaseEvent(float x,float y,int button);
    void mouseMoveEvent(float x,float y);
    void keyboardDown(int key);
    void keyboardUp(int key);
    //Loading and unloading
    void loadObject(std::string filePath);
    void loadObject(std::string name,std::string filePath);
    void unLoadObject(int number);
    void clearScene();
    //Other functions
    int  getNumberObjects();
    std::string getObjectName(int nunmber);

    void setClearColor(osg::Vec4f color);
    osg::Vec4f getClearColor();
};


#endif /* OSGMAINAPP_HPP_ */
