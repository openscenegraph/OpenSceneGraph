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
#include <limits>

@interface MenubarToggler : NSObject {

    osg::ref_ptr<osg::DisplaySettings> _displaySettings;
    osg::DisplaySettings::OSXMenubarBehavior _menubarBehavior;
}

-(void) show;
-(void) hide;
-(void) setDisplaySettings: (osg::DisplaySettings*) display_settings;

@end

@implementation MenubarToggler

-(id) init
{
    self = [super init];
    _menubarBehavior = osg::DisplaySettings::MENUBAR_AUTO_HIDE;
    _displaySettings = NULL;
    return self;
}

-(void) setDisplaySettings: (osg::DisplaySettings*) display_settings
{
    _displaySettings = display_settings;
}

-(void) hide
{
    if(_displaySettings.valid()) _menubarBehavior = _displaySettings->getOSXMenubarBehavior();
    
    if (_menubarBehavior == osg::DisplaySettings::MENUBAR_FORCE_SHOW)
        return;
    
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
        NSApplicationPresentationOptions options;
        switch(_menubarBehavior) {
            case osg::DisplaySettings::MENUBAR_AUTO_HIDE:
                options = NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideDock;
                break;
            
            case osg::DisplaySettings::MENUBAR_FORCE_HIDE:
                options = NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock;
                break;
            
            default:
                options = NSApplicationPresentationDefault;
                
        }
    
        [[NSApplication sharedApplication] setPresentationOptions: options];
    #else
        SystemUIMode mode = kUIModeAllHidden;
        SystemUIOptions options = 0;
        switch(_menubarBehavior) {
            case osg::DisplaySettings::MENUBAR_AUTO_HIDE:
                options = kUIOptionAutoShowMenuBar;
                break;
            
            case osg::DisplaySettings::MENUBAR_FORCE_HIDE:
                break;
            
            default:
                mode = kUIModeNormal;
                
        }
    
        OSErr error = SetSystemUIMode(mode, options);
        if (error) {
            OSG_DEBUG << "MenubarToggler::hide failed with " << error << std::endl;
        }
    #endif
}


-(void) show
{
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
        [[NSApplication sharedApplication] setPresentationOptions: NSApplicationPresentationDefault];
    #else
        OSErr error = SetSystemUIMode(kUIModeNormal, 0);
        if (error) {
            OSG_DEBUG << "MenubarToggler::show failed with " << error << std::endl;
        }
    #endif
}


@end

namespace osgDarwin {

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
size_t displayBitsPerPixelForMode(CGDisplayModeRef mode)
{
    CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
    if (!pixEnc)
    {
        OSG_WARN << "CGDisplayModeCopyPixelEncoding returned NULL" << std::endl;
        CGDisplayModeRelease(mode);
        return 0;
    }

    size_t depth = 0;
    if (CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        depth = 32;
    }
    else if (CFStringCompare(pixEnc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        depth = 16;
    }
    else if (CFStringCompare(pixEnc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
    {
        depth = 8;
    }
    else
    {
        OSG_WARN << "Unable to match pixel encoding '" << CFStringGetCStringPtr(pixEnc, kCFStringEncodingUTF8) << "'" << std::endl;
    }
    CFRelease(pixEnc);
    
    return depth;
}

#endif

size_t displayBitsPerPixel( CGDirectDisplayID displayId )
{
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    return CGDisplayBitsPerPixel(displayId);
#else
    CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
    if (!mode)
    {
        OSG_WARN << "CGDisplayCopyDisplayMode returned NULL" << std::endl;
        return 0;
    }

    unsigned int depth = displayBitsPerPixelForMode(mode);
    CGDisplayModeRelease(mode);

    return depth;
#endif
}


static bool findBestDisplayModeFor(const CGDirectDisplayID& displayid,  int desired_width,  int desired_height, unsigned int desired_color_depth, double desired_refresh_rate) {
    
    CFArrayRef availableModes = CGDisplayCopyAllDisplayModes(displayid, NULL);
    unsigned int numberOfAvailableModes = CFArrayGetCount(availableModes);
    
    CGDisplayModeRef best_match(NULL);
    
    int best_dx = std::numeric_limits<int>::max();
    int best_dy = std::numeric_limits<int>::max();
    
    
    for (unsigned int i=0; i<numberOfAvailableModes; ++i)
    {
        CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(availableModes, i);
        osg::GraphicsContext::ScreenSettings tmpSR;

        int w = CGDisplayModeGetWidth(mode);
        int h = CGDisplayModeGetHeight(mode);
        unsigned int color_depth = displayBitsPerPixelForMode(mode);

        double rate = CGDisplayModeGetRefreshRate(mode);
        
        int dx(w - desired_width);
        int dy(h - desired_height);
        
        if ((dx > 0) && (dx <= best_dx) && (dy > 0) && (dy <= best_dy) && (color_depth >= desired_color_depth) && (rate >= desired_refresh_rate)) {
            best_match = mode;
            best_dx = dx;
            best_dy = dy;
        }
    }
    bool result = false;
    if(best_match)
    {
        result = CGDisplaySetDisplayMode(displayid, best_match, NULL) != kCGErrorSuccess;
    }
    else if (desired_refresh_rate > 0)
    {
        // try again with a lower refresh-rate
        result = findBestDisplayModeFor(displayid, desired_width, desired_height, desired_color_depth, 0);
    }
    else if (desired_color_depth > 0)
    {
        // try again with a lower color_depth
        result = findBestDisplayModeFor(displayid, desired_width, desired_height, 0, 0);
    }
    
    CFRelease(availableModes);

    return result;
}




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
:    osg::Referenced(), 
    _list(), 
    _menubarShown(true),
    _mutex() 
{
    // the following code will query the system for the available rect on the main-display (typically the displaying showing the menubar + the dock

    NSRect rect = [[[NSScreen screens] objectAtIndex: 0] visibleFrame];
    _availRect = toCGRect(rect);
    
    // now we need the rect of the main-display including the menubar and the dock
    _mainScreenBounds = CGDisplayBounds( CGMainDisplayID() );


    // NSRect 0/0 is bottom/left, _mainScreenBounds 0/0 is top/left
    _availRect.origin.y = _mainScreenBounds.size.height - _availRect.size.height - _availRect.origin.y;
    
    _toggler = [[MenubarToggler alloc] init];
    update();
}

MenubarController::~MenubarController()
{
    [_toggler release];
}


void MenubarController::setDisplaySettings(osg::DisplaySettings* display_settings)
{
    [_toggler setDisplaySettings:display_settings];
    update();
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
                // OSG_ALWAYS << "testing rect " << windowBounds.origin.x << "/" << windowBounds.origin.y << " " << windowBounds.size.width << "x" << windowBounds.size.height << std::endl;
                // OSG_ALWAYS << "against      " << _availRect.origin.x << "/" << _availRect.origin.y << " " << _availRect.size.width << "x" << _availRect.size.height << std::endl;
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
            i = _list.erase(i);
    }
    
    // if we use the cocoa implementation then we have a NSRunLoop in place, and so we can use the deferred menubar-toggling which is thread safe
            
    #ifdef USE_DARWIN_COCOA_IMPLEMENTATION
    
        // SetSystemUIMode is not threadsafe, you'll get crashes if you call this method from other threads
        // so use a small NSObject to switch the menubar on the main thread via performSelectorOnMainThread
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        if (windowsCoveringMenubarArea && _menubarShown) 
        {
            
            //error = SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
            [_toggler performSelectorOnMainThread: @selector(hide) withObject:NULL waitUntilDone: YES];
        }
        if (!windowsCoveringMenubarArea && !_menubarShown) 
        {
            //error = SetSystemUIMode(kUIModeNormal, 0);
            [_toggler performSelectorOnMainThread: @selector(show) withObject:NULL waitUntilDone: YES];
        }
        [pool release];
    
    #else
    
        OSErr error;
        
        // see http://developer.apple.com/technotes/tn2002/tn2062.html for hiding the dock+menubar
        if (windowsCoveringMenubarArea && _menubarShown) 
        {
            error = SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
        } 
        if (!windowsCoveringMenubarArea && !_menubarShown) 
        {
            error = SetSystemUIMode(kUIModeNormal, 0);
        }
    #endif
    
    _menubarShown = !windowsCoveringMenubarArea;
}



/** Helper method to get a double value out of a CFDictionary */
#if (MAC_OS_X_VERSION_MAX_ALLOWED < 1060)
	static double getDictDouble (CFDictionaryRef refDict, CFStringRef key)
{
    double value;
    CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(refDict, key);
    if (!number_value) // if can't get a number for the dictionary
        return -1;  // fail
    if (!CFNumberGetValue(number_value, kCFNumberDoubleType, &value)) // or if can't convert it
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
    if (!CFNumberGetValue(number_value, kCFNumberLongType, &value)) // or if can't convert it
        return -1; // fail
    return value;
}
#endif


/** ctor, get a list of all attached displays */
DarwinWindowingSystemInterface::DarwinWindowingSystemInterface() :
    _initialized(false),
    _displayCount(0),
    _displayIds(NULL)
{
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

void DarwinWindowingSystemInterface::_init()
{
    if (_initialized) return;

    ProcessSerialNumber sn = { 0, kCurrentProcess };
    TransformProcessType(&sn,kProcessTransformToForegroundApplication);
    SetFrontProcess(&sn);

    if( CGGetActiveDisplayList( 0, NULL, &_displayCount ) != CGDisplayNoErr )
    {
        OSG_WARN << "DarwinWindowingSystemInterface: could not get # of screens" << std::endl;
        _displayCount = 0;

        _initialized = true;
        return;
    }

    _displayIds = new CGDirectDisplayID[_displayCount];

    if( CGGetActiveDisplayList( _displayCount, _displayIds, &_displayCount ) != CGDisplayNoErr )
    {
        OSG_WARN << "DarwinWindowingSystemInterface: CGGetActiveDisplayList failed" << std::endl;
    }

    _initialized = true;
}

/** @return a CGDirectDisplayID for a ScreenIdentifier */
CGDirectDisplayID DarwinWindowingSystemInterface::getDisplayID(const osg::GraphicsContext::ScreenIdentifier& si)
{
    _init();

    if (_displayCount==0)
    {
        OSG_WARN << "DarwinWindowingSystemInterface::getDisplayID(..) no valid screens available returning 0 instead." << std::endl;
        return 0;
    }

    if (si.screenNum < static_cast<int>(_displayCount))
    {
        return _displayIds[si.screenNum];
    }
    else
    {
        OSG_WARN << "DarwinWindowingSystemInterface::getDisplayID(..) invalid screen # " << si.screenNum << ", returning main-screen instead." << std::endl;
        return _displayIds[0];
    }
}

/** @return count of attached screens */
unsigned int DarwinWindowingSystemInterface::getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) 
{
    _init();

    return _displayCount;
}

void DarwinWindowingSystemInterface::getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution)
{
    _init();

    if (_displayCount==0)
    {
        resolution.width = 0;
        resolution.height = 0;
        resolution.colorDepth = 0;
        resolution.refreshRate = 0;
        return;
    }
    
    CGDirectDisplayID id = getDisplayID(si);
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    
        CGDisplayModeRef display_mode_ref = CGDisplayCopyDisplayMode(id);
        resolution.width = CGDisplayModeGetWidth(display_mode_ref);
        resolution.height = CGDisplayModeGetHeight(display_mode_ref);
        resolution.colorDepth = displayBitsPerPixelForMode(display_mode_ref);
        resolution.refreshRate = CGDisplayModeGetRefreshRate(display_mode_ref);
    
        CGDisplayModeRelease(display_mode_ref);
    
    #else
        resolution.width = CGDisplayPixelsWide(id);
        resolution.height = CGDisplayPixelsHigh(id);
        resolution.colorDepth = displayBitsPerPixel(id);
        
        resolution.refreshRate = getDictDouble (CGDisplayCurrentMode(id), kCGDisplayRefreshRate);        // Not tested
    #endif
    if (resolution.refreshRate<0) resolution.refreshRate = 0;
}


void DarwinWindowingSystemInterface::enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettingsList & resolutionList)
{
    _init();

    // Warning! This method has not been tested.
    resolutionList.clear();

    if (_displayCount==0)
    {
        return;
    }

    CGDirectDisplayID displayid = getDisplayID(screenIdentifier);
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    
    CFArrayRef availableModes = CGDisplayCopyAllDisplayModes(displayid, NULL);
    unsigned int numberOfAvailableModes = CFArrayGetCount(availableModes);
    for (unsigned int i=0; i<numberOfAvailableModes; ++i)
    {
        CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(availableModes, i);
        osg::GraphicsContext::ScreenSettings tmpSR;

        tmpSR.width = CGDisplayModeGetWidth(mode);
        tmpSR.height = CGDisplayModeGetHeight(mode);
        tmpSR.colorDepth = displayBitsPerPixelForMode(mode);
        tmpSR.refreshRate = CGDisplayModeGetRefreshRate(mode);

        resolutionList.push_back(tmpSR);
    }

    CFRelease(availableModes);

#else
    
    CFArrayRef availableModes = CGDisplayAvailableModes(displayid);
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
#endif
}

/** return the top left coord of a specific screen in global screen space */
void DarwinWindowingSystemInterface::getScreenTopLeft(const osg::GraphicsContext::ScreenIdentifier& si, int& x, int& y)
{
    _init();

    if (_displayCount==0)
    {
        x = 0;
        y = 0;
        return;
    }

    CGRect bounds = CGDisplayBounds( getDisplayID(si) );
    x = static_cast<int>(bounds.origin.x);
    y = static_cast<int>(bounds.origin.y);
    
    // OSG_DEBUG << "topleft of screen " << si.screenNum <<" " << bounds.origin.x << "/" << bounds.origin.y << std::endl;
}


bool DarwinWindowingSystemInterface::setScreenSettings(const osg::GraphicsContext::ScreenIdentifier &si, const osg::GraphicsContext::ScreenSettings & settings)
{
    CGDirectDisplayID displayid = getDisplayID(si);

    #if (MAC_OS_X_VERSION_MAX_ALLOWED >= 1060)
    
        return findBestDisplayModeFor(displayid, settings.width, settings.height, settings.colorDepth, settings.refreshRate);
    
    #else
        // add next line and on following line replace hard coded depth and refresh rate
        CGRefreshRate refresh =  getDictDouble (CGDisplayCurrentMode(displayid), kCGDisplayRefreshRate);  
        CFDictionaryRef display_mode_values =
            CGDisplayBestModeForParametersAndRefreshRate(
                            displayid, 
                            settings.colorDepth,
                            settings.width, settings.height,
                            settings.refreshRate,
                            NULL);

                                          
        return CGDisplaySwitchToMode(displayid, display_mode_values) != kCGErrorSuccess;
    #endif
    
    return false;
}




unsigned int DarwinWindowingSystemInterface::getScreenContaining(int x, int y, int w, int h)
{
    _init();

    if (_displayCount==0)
    {
        return 0;
    }

    CGRect rect = CGRectMake(x,y,w,h);
    for(unsigned int i = 0; i < _displayCount; ++i) {
        CGRect bounds = CGDisplayBounds( getDisplayID(i) );
        if (CGRectIntersectsRect(bounds, rect)) {
            return i;
        }
    }
    
    return 0;
}

}
