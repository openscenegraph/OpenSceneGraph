/*
 *  Sketch.h
 *  cefixSketch
 *
 *  Created by Stephan Huber on 11.05.09.
 *  Copyright 2009 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#ifndef SKETCH_HEADER
#define SKETCH_HEADER

#ifdef __OBJC__
@class UIWindow;
@class UIView;
#else
class UIWindow;
class UIView;
#endif


#include <cefix/Sketch.h>
#include <cefix/GroupWidget.h>
#include <cefix/InputDeviceManager.h>
#include <osgViewer/api/IOS/GraphicsWindowIOS>

#include "SuperShape3D.h"

class Sketch : public cefix::Sketch {
public:
    Sketch();
    void init();
	void setup();
	void draw();
    
    void setUiWindow(UIWindow* win) { _uiWindow = win; }
    osgViewer::GraphicsWindowIOS::WindowData* getWindowData() { return _windowData; }
private:
    
    osg::ref_ptr<cefix::GroupWidget> _widgets;
    osg::ref_ptr<SuperShape3D> _drawable;
	osg::ref_ptr<cefix::InputDevice> _device;
	osg::ref_ptr<osg::MatrixTransform> _mat;
    UIWindow* _uiWindow;
    osg::ref_ptr<osgViewer::GraphicsWindowIOS::WindowData> _windowData;

};


#endif