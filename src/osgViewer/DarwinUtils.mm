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
#include "DarwinUtils.h"
#include <Cocoa/Cocoa.h>

namespace osgDarwin {


static inline CGRect toCGRect(NSRect nsRect)
{
	CGRect cgRect;

	cgRect.origin.x = nsRect.origin.x;
	cgRect.origin.y = nsRect.origin.y;
	cgRect.size.width = nsRect.size.width;
	cgRect.size.height = nsRect.size.height;

	return cgRect;
}


MenubarController::MenubarController()
:	osg::Referenced(), 
    _list(), 
    _menubarShown(false),
    _mutex() 
{
	// the following code will query the system for the available rect on the main-display (typically the displaying showing the menubar + the dock

	NSRect rect = [[[NSScreen screens] objectAtIndex: 0] visibleFrame];
	_availRect = toCGRect(rect);
	
	// now we need the rect of the main-display including the menubar and the dock
	_mainScreenBounds = CGDisplayBounds( CGMainDisplayID() );


	// NSRect 0/0 is bottom/left, _mainScreenBounds 0/0 is top/left
	_availRect.origin.y = _mainScreenBounds.size.height - _availRect.size.height - _availRect.origin.y;
	
		
	// hide the menubar initially
	SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
}




MenubarController* MenubarController::instance() 
{
    static osg::ref_ptr<MenubarController> s_menubar_controller = new MenubarController();
    return s_menubar_controller.get();
}


void MenubarController::attachWindow(WindowAdapter* win)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _list.push_back(win);
    update();
}


void MenubarController::detachWindow(osgViewer::GraphicsWindow* win) 
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    for(WindowList::iterator i = _list.begin(); i != _list.end(); ) {
        if ((*i)->getWindow() == win)
            i = _list.erase(i);
        else 
            ++i;
    }
    update();
}

// iterate through all open windows and check, if they intersect the area occupied by the menubar/dock, and if so, hide the menubar/dock

void MenubarController::update() 
{
    OSErr error(noErr);
    unsigned int windowsCoveringMenubarArea = 0;    
    unsigned int windowsIntersectingMainScreen = 0;
    for(WindowList::iterator i = _list.begin(); i != _list.end(); ) {
		WindowAdapter* wi = (*i).get();
        if (wi->valid()) {
            CGRect windowBounds;
			wi->getWindowBounds(windowBounds);
			
			if (CGRectIntersectsRect(_mainScreenBounds, windowBounds))
            {
                ++windowsIntersectingMainScreen;
                // osg::notify(osg::ALWAYS) << "testing rect " << windowBounds.origin.x << "/" << windowBounds.origin.y << " " << windowBounds.size.width << "x" << windowBounds.size.height << std::endl;
				// osg::notify(osg::ALWAYS) << "against      " << _availRect.origin.x << "/" << _availRect.origin.y << " " << _availRect.size.width << "x" << _availRect.size.height << std::endl;
                // the window intersects the main-screen, does it intersect with the menubar/dock?
                if (((_availRect.origin.y > _mainScreenBounds.origin.y) && (_availRect.origin.y > windowBounds.origin.y)) ||
                    ((_availRect.origin.x > _mainScreenBounds.origin.x) && (_availRect.origin.x > windowBounds.origin.x)) || 
                    ((_availRect.size.width < _mainScreenBounds.size.width) && (_availRect.origin.x + _availRect.size.width < windowBounds.origin.x + windowBounds.size.width)) || 
                    ((_availRect.size.height < _mainScreenBounds.size.height) && (_availRect.origin.y + _availRect.size.height < windowBounds.origin.y + windowBounds.size.height) ))
                {
                    ++windowsCoveringMenubarArea;
                }
            }
            
            ++i;
        }
        else
            i= _list.erase(i);
    }
    
    // see http://developer.apple.com/technotes/tn2002/tn2062.html for hiding the dock+menubar
        
    if (windowsCoveringMenubarArea && _menubarShown)
        error = SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
    
    if (!windowsCoveringMenubarArea && !_menubarShown)
        error = SetSystemUIMode(kUIModeNormal, 0);
        _menubarShown = !windowsCoveringMenubarArea;
    
    // osg::notify(osg::DEBUG_INFO) << "MenubarController:: " << windowsCoveringMenubarArea << " windows covering the menubar/dock area, " << windowsIntersectingMainScreen << " intersecting mainscreen" << std::endl;
}



/** Helper method to get a double value out of a CFDictionary */
static double getDictDouble (CFDictionaryRef refDict, CFStringRef key)
{
	double value;
	CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(refDict, key);
	if (!number_value) // if can't get a number for the dictionary
		return -1;  // fail
	if (!CFNumberGetValue(number_value, kCFNumberDoubleType, &value)) // or if cant convert it
		return -1; // fail
	return value; // otherwise return the long value
}

/** Helper method to get a long value out of a CFDictionary */
static long getDictLong(CFDictionaryRef refDict, CFStringRef key)        // const void* key?
{
	long value = 0;
	CFNumberRef number_value = (CFNumberRef)CFDictionaryGetValue(refDict, key); 
	if (!number_value) // if can't get a number for the dictionary
		return -1;  // fail
	if (!CFNumberGetValue(number_value, kCFNumberLongType, &value)) // or if cant convert it
		return -1; // fail
	return value;
}



/** ctor, get a list of all attached displays */
DarwinWindowingSystemInterface::DarwinWindowingSystemInterface() :
	_displayCount(0),
	_displayIds(NULL)
{
	ProcessSerialNumber sn = { 0, kCurrentProcess };
	TransformProcessType(&sn,kProcessTransformToForegroundApplication);
	SetFrontProcess(&sn);
	
	if( CGGetActiveDisplayList( 0, NULL, &_displayCount ) != CGDisplayNoErr )
		osg::notify(osg::WARN) << "DarwinWindowingSystemInterface: could not get # of screens" << std::endl;
		
	_displayIds = new CGDirectDisplayID[_displayCount];
	if( CGGetActiveDisplayList( _displayCount, _displayIds, &_displayCount ) != CGDisplayNoErr )
		osg::notify(osg::WARN) << "DarwinWindowingSystemInterface: CGGetActiveDisplayList failed" << std::endl;
	
	}

/** dtor */
DarwinWindowingSystemInterface::~DarwinWindowingSystemInterface()
{
	if (osg::Referenced::getDeleteHandler())
	{
		osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
		osg::Referenced::getDeleteHandler()->flushAll();
	}

	if (_displayIds) delete[] _displayIds;
	_displayIds = NULL;
}

/** @return a CGDirectDisplayID for a ScreenIdentifier */
CGDirectDisplayID DarwinWindowingSystemInterface::getDisplayID(const osg::GraphicsContext::ScreenIdentifier& si) {
	if (si.screenNum < static_cast<int>(_displayCount))
		return _displayIds[si.screenNum];
	else {
		osg::notify(osg::WARN) << "GraphicsWindowCarbon :: invalid screen # " << si.screenNum << ", returning main-screen instead" << std::endl;
		return _displayIds[0];
	}
}

/** @return count of attached screens */
unsigned int DarwinWindowingSystemInterface::getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) 
{
	return _displayCount;
}

void DarwinWindowingSystemInterface::getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution)
{
	CGDirectDisplayID id = getDisplayID(si);
	resolution.width = CGDisplayPixelsWide(id);
	resolution.height = CGDisplayPixelsHigh(id);
	resolution.colorDepth = CGDisplayBitsPerPixel(id);
	resolution.refreshRate = getDictDouble (CGDisplayCurrentMode(id), kCGDisplayRefreshRate);        // Not tested
	if (resolution.refreshRate<0) resolution.refreshRate = 0;
}


void DarwinWindowingSystemInterface::enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettingsList & resolutionList) {
        // Warning! This method has not been tested.
        resolutionList.clear();

        CGDirectDisplayID displayID = getDisplayID(screenIdentifier);
        CFArrayRef availableModes = CGDisplayAvailableModes(displayID);
        unsigned int numberOfAvailableModes = CFArrayGetCount(availableModes);
        for (unsigned int i=0; i<numberOfAvailableModes; ++i) {
            // look at each mode in the available list
            CFDictionaryRef mode = (CFDictionaryRef)CFArrayGetValueAtIndex(availableModes, i);
            osg::GraphicsContext::ScreenSettings tmpSR;

            long width = getDictLong(mode, kCGDisplayWidth);
            tmpSR.width = width<=0 ? 0 : width;
            long height = getDictLong(mode, kCGDisplayHeight);
            tmpSR.height = height<=0 ? 0 : height;
            long rate = getDictLong(mode, kCGDisplayRefreshRate);
            tmpSR.refreshRate = rate<=0 ? 0 : rate;
            long depth = getDictLong(mode, kCGDisplayBitsPerPixel);
            tmpSR.colorDepth = depth<=0 ? 0 : depth;

            resolutionList.push_back(tmpSR);
        }
    }

/** return the top left coord of a specific screen in global screen space */
void DarwinWindowingSystemInterface::getScreenTopLeft(const osg::GraphicsContext::ScreenIdentifier& si, int& x, int& y) {
	CGRect bounds = CGDisplayBounds( getDisplayID(si) );
	x = static_cast<int>(bounds.origin.x);
	y = static_cast<int>(bounds.origin.y);
	
	// osg::notify(osg::DEBUG_INFO) << "topleft of screen " << si.screenNum <<" " << bounds.origin.x << "/" << bounds.origin.y << std::endl;
}



/** implementation of setScreenResolution */
bool DarwinWindowingSystemInterface::setScreenResolution(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, unsigned int width, unsigned int height) 
{ 
	CGDirectDisplayID displayID = getDisplayID(screenIdentifier);
	
	// add next line and on following line replace hard coded depth and refresh rate
	CGRefreshRate refresh =  getDictDouble (CGDisplayCurrentMode(displayID), kCGDisplayRefreshRate);  
	CFDictionaryRef display_mode_values =
		CGDisplayBestModeForParametersAndRefreshRate(
						displayID, 
						CGDisplayBitsPerPixel(displayID), 
						width, height,  
						refresh,  
						NULL);

									  
	CGDisplaySwitchToMode(displayID, display_mode_values);    
	return true; 
}

/** implementation of setScreenRefreshRate */
bool DarwinWindowingSystemInterface::setScreenRefreshRate(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, double refreshRate) { 
	
	boolean_t  success(false);
	unsigned width, height;
	getScreenResolution(screenIdentifier, width, height);
	
	CGDirectDisplayID displayID = getDisplayID(screenIdentifier);
	
	// add next line and on following line replace hard coded depth and refresh rate
	CFDictionaryRef display_mode_values =
		CGDisplayBestModeForParametersAndRefreshRate(
						displayID, 
						CGDisplayBitsPerPixel(displayID), 
						width, height,  
						refreshRate,  
						&success);

									  
	if (success)
		CGDisplaySwitchToMode(displayID, display_mode_values);    
		
	return (success != 0);
}

	






}