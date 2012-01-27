/*
 *  Sketch.cpp
 *  cefixSketch
 *
 *  Created by Stephan Huber on 11.05.09.
 *  Copyright 2009 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */


#include "Sketch.h"
#include <cefix/DataFactory.h>
#include <cefix/WidgetFactory.h>
#include <cefix/HSliderWithCaptionAndValueWidget.h>

#ifdef CEFIX_FOR_IPHONE
#include <osgGA/MultiTouchTrackballManipulator>
#endif

Sketch::Sketch()
:   cefix::Sketch(),
    _uiWindow(NULL)
{
}

void Sketch::init() {
    cefix::Sketch::init();
	
	_device = cefix::InputDeviceManager::instance()->open("IOS_Accelerometer");
	if (_device.valid()) 
	{
		std::cout << (*_device.get()) << std::endl;
	}
    
    #ifdef CEFIX_FOR_IPHONE
        cefix::DisplayCollection* dc = new cefix::DisplayCollection();
        
        // fullscreen-konfiguration erzeugen
        cefix::WindowConfiguration* win_conf = cefix::WindowConfiguration::createFullScreen(0);
        
        // wir deaktivieren das automatische Mitdrehen
        _windowData = new osgViewer::GraphicsWindowIOS::WindowData(_uiWindow, false);
        win_conf->getTraits()->inheritedWindowData = _windowData;
        win_conf->getTraits()->y = 30;
        win_conf->getTraits()->height = 290;
                
        dc->addWindowConfiguration(win_conf);
        setDisplayCollection(dc);    
    #endif
}



void Sketch::setup() 
{
    #ifdef CEFIX_FOR_IPHONE
        getMainWindow()->setTrackballManipulator(new osgGA::MultiTouchTrackballManipulator());
        getMainWindow()->setCameraManipulator(getMainWindow()->getTrackballManipulator());
    #endif
    
    float ss1_m = 7;        // some convenient defaults for demo
	float ss1_a = 1;
	float ss1_b = 1;
	float ss1_n1 = 0.2;
	float ss1_n2 = 1.7;
	float ss1_n3 = 1.7;
	float ss2_m = 7;
	float ss2_a = 1;
	float ss2_b = 1;
	float ss2_n1 = 0.2;
	float ss2_n2 = 1.7;
	float ss2_n3 = 1.7;
	
	_drawable = new SuperShape3D(ss1_m, ss1_a, ss1_b, ss1_n1, ss1_n2, ss1_n3, ss2_m, ss2_a, ss2_b, ss2_n1, ss2_n2, ss2_n3, 60);
	osg::Geode* geode = new osg::Geode();
	geode->addDrawable(_drawable);
	_mat = new osg::MatrixTransform();
	_mat->addChild(geode);
	getWorld()->addChild(_mat);
	
	// allow osg-event handler:
	allowOsgHandler(true);
    
	
}


void Sketch::draw()
{
	if (_device) {
		osg::Vec3 v = _device->getVec3Value(0);
		_mat->setMatrix(osg::Matrix::rotate(v[0], osg::X_AXIS, v[1], osg::Y_AXIS, v[2], osg::Z_AXIS)); 
	}
}




