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
#include "IOSUtils.h"


namespace osgIOS {


class AutoReleasePoolHelper {
public:
    AutoReleasePoolHelper() {
        pool = [[NSAutoreleasePool alloc] init];
    }
    
    ~AutoReleasePoolHelper() { [pool release]; }
private:
    NSAutoreleasePool* pool;
};


/** ctor, get a list of all attached displays */
IOSWindowingSystemInterface::IOSWindowingSystemInterface()
:   osg::GraphicsContext::WindowingSystemInterface()
{
}

/** dtor */
IOSWindowingSystemInterface::~IOSWindowingSystemInterface()
{
    if (osg::Referenced::getDeleteHandler())
    {
        osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
        osg::Referenced::getDeleteHandler()->flushAll();
    }

}


/** @return count of attached screens */
unsigned int IOSWindowingSystemInterface::getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) 
{
    AutoReleasePoolHelper auto_release_pool_helper;
    return [[UIScreen screens] count];
}

void IOSWindowingSystemInterface::getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution)
{
    AutoReleasePoolHelper auto_release_pool_helper;
    
    if(si.screenNum >= [[UIScreen screens] count]){return;}
    
    

    //get the screens array from the UIScreen class
    NSArray* screens = [UIScreen screens];
    //iterate to the desired screen num
    UIScreen* screen = [screens objectAtIndex:si.screenNum];
    
    if (si.screenNum == 0) 
    {
        //internal display supports only one mode, UiScreenMode reports wrong sizes for internal display at least for iOS 3.2
        float scale = 1.0f;
        
        #if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
            scale = [screen scale];
        #endif
        
        
        resolution.width = [screen bounds].size.width * scale;
        resolution.height = [screen bounds].size.height * scale;
        resolution.colorDepth = 24; 
        resolution.refreshRate = 60; //i've read 60 is max, not sure if that's true        
    } 
    else 
    {
        //get the screen mode
        NSArray* modesArray = [screen availableModes];
        
        if(modesArray)
        {
            //for this method we copy the first mode (default) then return
            UIScreenMode* mode = [modesArray objectAtIndex:0];

            CGSize size = [mode size];
            resolution.width = size.width;
            resolution.height = size.height;
            resolution.colorDepth = 24; 
            resolution.refreshRate = 60; //i've read 60 is max, not sure if that's true
            
            OSG_INFO << "new resolution for screen " << si.screenNum << ": " << size.width << "x" << size.height << std::endl;
        }
    }
}

//
//Due to the weird
void IOSWindowingSystemInterface::enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, 
                                                             osg::GraphicsContext::ScreenSettingsList & resolutionList) 
{
    AutoReleasePoolHelper auto_release_pool_helper;
    
    if(si.screenNum >= [[UIScreen screens] count]){return;}
    
    //get the screens array from the UIScreen class
    NSArray* screens = [UIScreen screens];
    //get the desired screen num
    UIScreen* screen = [screens objectAtIndex:si.screenNum];

    if (si.screenNum == 0) 
    {
        //internal display supports only one mode, UiScreenMode reports wrong sizes for internal screen at least for iOS 3.2
        osg::GraphicsContext::ScreenSettings resolution;
        
        float scale = 1.0f;
        #if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
            scale = [screen scale];
        #endif
        
        resolution.width = [screen bounds].size.width * scale;
        resolution.height = [screen bounds].size.height * scale;
        resolution.colorDepth = 24; 
        resolution.refreshRate = 60; //i've read 60 is max, not sure if that's true
        resolutionList.push_back(resolution);
        
        
    } 
    else 
    {
        // external display may support more resolutions:

        //get the screen mode
        NSArray* modesArray = [screen availableModes];
        NSEnumerator* modesEnum = [modesArray objectEnumerator];
        UIScreenMode* mode;
        //iterate over modes and get their size property
        while ( mode = [modesEnum nextObject] ) {
            
            osg::GraphicsContext::ScreenSettings resolution;
            CGSize size = [mode size];
            resolution.width = size.width;
            resolution.height = size.height;
            resolution.colorDepth = 24; 
            resolution.refreshRate = 60; //i've read 60 is max, not sure if that's true
            resolutionList.push_back(resolution);
            
            OSG_INFO << "new resolution: " << size.width << "x" << size.height << std::endl;
        }
    
    }
}



bool IOSWindowingSystemInterface::setScreenSettings(const osg::GraphicsContext::ScreenIdentifier &si, const osg::GraphicsContext::ScreenSettings & settings)
{
    bool result = setScreenResolutionImpl(si, settings.width, settings.height);
    if (result)
        setScreenRefreshRateImpl(si, settings.refreshRate);
    
    return result;
}



/** implementation of setScreenResolution */
//IPad can have extenal screens which we can request a res for
//the main screen screenNum 0 can not currently have its res changed
//as it only has one mode (might change though and this should still handle it)
//
bool IOSWindowingSystemInterface::setScreenResolutionImpl(const osg::GraphicsContext::ScreenIdentifier& si, unsigned int width, unsigned int height) 
{
    AutoReleasePoolHelper auto_release_pool_helper;
    
    if(si.screenNum >= [[UIScreen screens] count]){return false;}

    
    //get the screens array from the UIScreen class
    NSArray* screens = [UIScreen screens];
    
    //iterate to the desired screen num
    UIScreen* screen = [screens objectAtIndex:si.screenNum];

    //get the screen mode
    NSArray* modesArray = [screen availableModes];
    NSEnumerator* modesEnum = [modesArray objectEnumerator];
    UIScreenMode* mode;
    //iterate over modes and get their size property
    while ( mode = [modesEnum nextObject] ) {
        
        osg::GraphicsContext::ScreenSettings resolution;
        CGSize size = [mode size];
        
        //if the modes size/resolution matches the passed width/height then assign this
        //mode as the screens current mode
        if(size.width == width && size.height == height)
        {
            screen.currentMode = mode;
            OSG_INFO << "IOSWindowingSystemInterface::setScreenResolutionImpl: Set resolution of screen '" << si.screenNum << "', to '" << width << ", " << height << "'." << std::endl;
            return true;
        }
        
    }

    OSG_WARN << "IOSWindowingSystemInterface::setScreenResolutionImpl: Failed to set resolution of screen '" << si.screenNum << "', to '" << width << ", " << height << "'." << std::endl;
    return false; 
}

/** implementation of setScreenRefreshRate, don't think you can do this on IOS */
bool IOSWindowingSystemInterface::setScreenRefreshRateImpl(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, double refreshRate) { 
    
    return true;
}


unsigned int IOSWindowingSystemInterface::getScreenContaining(int x, int y, int w, int h)
{
    return 1;
}

//
//return the UIScreen object asscoiated with the passed ScreenIdentifier
//returns nil if si isn't found
//
UIScreen* IOSWindowingSystemInterface::getUIScreen(const osg::GraphicsContext::ScreenIdentifier& si)
{
    AutoReleasePoolHelper auto_release_pool_helper;
    
    if(si.screenNum >= [[UIScreen screens] count]){return nil;}
    
    UIScreen* screen = [[UIScreen screens] objectAtIndex:si.screenNum];
    return screen;
}

//
//Returns the contents scale factor of the screen, this is the scale factor required
//to convert points to pixels on this screen
//
bool IOSWindowingSystemInterface::getScreenContentScaleFactor(const osg::GraphicsContext::ScreenIdentifier& si, float& scaleFactor)
{
    AutoReleasePoolHelper auto_release_pool_helper;
    
    if(si.screenNum >= [[UIScreen screens] count]){return false;}
    
    UIScreen* screen = this->getUIScreen(si);
    if(screen != nil)
    {
        scaleFactor = 1.0f;
#if defined(__IPHONE_4_0) && (__IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_4_0)
        CGFloat scale = [screen scale];
        scaleFactor = scale;
#endif
        return true;
    }
    return false;
}
    
//
//Returns the screens size in points, docs state a point is roughly 1/160th of an inch
//
bool IOSWindowingSystemInterface::getScreenSizeInPoints(const osg::GraphicsContext::ScreenIdentifier& si, osg::Vec2& pointSize)
{
    AutoReleasePoolHelper auto_release_pool_helper;
    
    if(si.screenNum >= [[UIScreen screens] count]){return false;}
    
    UIScreen* screen = this->getUIScreen(si);
    if(screen != nil)
    {
        CGRect bounds = [screen bounds];
        pointSize.x() = bounds.size.width;
        pointSize.y() = bounds.size.height;
        return true;
    }
    return false;    
}

}
