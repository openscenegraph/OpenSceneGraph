/*
 *  DarwinUtils.cpp
 *  OpenSceneGraph
 *
 *  Created by Stephan Huber on 27.06.08.
 *  Copyright 2008 Stephan Maximilian Huber, digital mind. All rights reserved.
 *
 */

#include <osg/Referenced>
#include <osg/DeleteHandler>
#include <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "IPhoneUtils.h"


namespace osgIPhone {


/** ctor, get a list of all attached displays */
IPhoneWindowingSystemInterface::IPhoneWindowingSystemInterface() :
    _displayCount(0),
    _displayIds(0)
{
	_displayCount = 1;      
    _displayIds = 1;
}

/** dtor */
IPhoneWindowingSystemInterface::~IPhoneWindowingSystemInterface()
{
    if (osg::Referenced::getDeleteHandler())
    {
        osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
        osg::Referenced::getDeleteHandler()->flushAll();
    }

    _displayIds = 0;
}

/** @return a CGDirectDisplayID for a ScreenIdentifier */
int IPhoneWindowingSystemInterface::getDisplayID(const osg::GraphicsContext::ScreenIdentifier& si) {
    if (si.screenNum < _displayCount)
        return _displayIds;
    else {
        osg::notify(osg::WARN) << "GraphicsWindowIPhone :: invalid screen # " << si.screenNum << ", returning main-screen instead" << std::endl;
        return _displayIds;
    }
}

/** @return count of attached screens */
unsigned int IPhoneWindowingSystemInterface::getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) 
{
    return _displayCount;
}

void IPhoneWindowingSystemInterface::getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution)
{
    int id = getDisplayID(si);
	CGRect lFrame = [[UIScreen mainScreen] bounds];
	
    resolution.width = lFrame.size.width;
    resolution.height = lFrame.size.height;
    resolution.colorDepth = 24; 
    resolution.refreshRate = 60; //i've read 60 is max, not sure if thats true
    if (resolution.refreshRate<0) resolution.refreshRate = 0;
}


void IPhoneWindowingSystemInterface::enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettingsList & resolutionList) {
        // Warning! This method has not been tested.
        resolutionList.clear();

        int displayid = getDisplayID(screenIdentifier);
       // CFArrayRef availableModes = CGDisplayAvailableModes(displayid);
        //unsigned int numberOfAvailableModes = CFArrayGetCount(availableModes);

            osg::GraphicsContext::ScreenSettings tmpSR;

			CGRect lFrame = [[UIScreen mainScreen] bounds];

            long width = lFrame.size.width;
            tmpSR.width = width<=0 ? 0 : width;
            long height = lFrame.size.height;
            tmpSR.height = height<=0 ? 0 : height;
            long rate = 60;
            tmpSR.refreshRate = rate<=0 ? 0 : rate;
            long depth = 24;
            tmpSR.colorDepth = depth<=0 ? 0 : depth;

            resolutionList.push_back(tmpSR);

}

/** return the top left coord of a specific screen in global screen space */
void IPhoneWindowingSystemInterface::getScreenTopLeft(const osg::GraphicsContext::ScreenIdentifier& si, int& x, int& y) {
	
	//bounds will return full screen, application frame includes the IPhone status bar at the top
	CGRect lFrame = [[UIScreen mainScreen] bounds]; //applicationFrame];
    x = static_cast<int>(lFrame.origin.x);
    y = static_cast<int>(lFrame.origin.y);
    
    // osg::notify(osg::DEBUG_INFO) << "topleft of screen " << si.screenNum <<" " << bounds.origin.x << "/" << bounds.origin.y << std::endl;
}


bool IPhoneWindowingSystemInterface::setScreenSettings(const osg::GraphicsContext::ScreenIdentifier &si, const osg::GraphicsContext::ScreenSettings & settings)
{
    bool result = setScreenResolutionImpl(si, settings.width, settings.height);
    if (result)
        setScreenRefreshRateImpl(si, settings.refreshRate);
    
    return result;
}



/** implementation of setScreenResolution, can't change screen res on IPhone */
bool IPhoneWindowingSystemInterface::setScreenResolutionImpl(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, unsigned int width, unsigned int height) 
{ 
    int displayid = getDisplayID(screenIdentifier);
    return true; 
}

/** implementation of setScreenRefreshRate, don't think you can do this on IPhone */
bool IPhoneWindowingSystemInterface::setScreenRefreshRateImpl(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, double refreshRate) { 
    
    boolean_t  success(false);
    unsigned width, height;
    getScreenResolution(screenIdentifier, width, height);
    return true;
}


unsigned int IPhoneWindowingSystemInterface::getScreenContaining(int x, int y, int w, int h)
{
    return 1;
}


}
