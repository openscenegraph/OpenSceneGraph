/*
 *  IOSUtils.h
 *  OpenSceneGraph
 *
 *  Created by Thomas Hogarth on 25.11.09.
 *
 * By default we create a full res buffer across all devices and if now viewContentScaleFator is given We use the screens ScaleFactor.
 * This means that for backward compatibility you need to set the windowData _viewContentScaleFactor to 1.0f and set the screen res to the
 * res that of the older gen device.
 * http://developer.apple.com/library/ios/documentation/IOS/Conceptual/IOSOSProgrammingGuide/SupportingResolutionIndependence/SupportingResolutionIndependence.html#//apple_ref/doc/uid/TP40007072-CH10-SW11
 *
 */

#ifdef __APPLE__

#ifndef IOS_UTILS_HEADER_
#define IOS_UTILS_HEADER_

#ifdef __OBJC__
@class UIScreen;
#else
class UIScreen;
#endif

#include <osg/DeleteHandler>
#include <osg/GraphicsContext>
#include <osgViewer/GraphicsWindow>

namespace osgIOS {



struct IOSWindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface
{
public:
    IOSWindowingSystemInterface();

    /** dtor */
    ~IOSWindowingSystemInterface();


    /** @return count of attached screens */
    virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) ;

    virtual void getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution);

    virtual void enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettingsList & resolutionList);

    virtual bool setScreenSettings (const osg::GraphicsContext::ScreenIdentifier & si, const osg::GraphicsContext::ScreenSettings & settings);

    /** returns screen-ndx containing rect x,y,w,h, NOT_TESTED@tom */
    unsigned int getScreenContaining(int x, int y, int w, int h);

    //IOS specific

    //
    //return the UIScreen object asscoiated with the passed ScreenIdentifier
    //returns nil if si isn't found
    UIScreen* getUIScreen(const osg::GraphicsContext::ScreenIdentifier& si);

    //
    //Get the contents scale factor of the screen, this is the scale factor required
    //to convert points to pixels on this screen
    bool getScreenContentScaleFactor(const osg::GraphicsContext::ScreenIdentifier& si, float& scaleFactor);

    //
    //Get the screens size in points, docs state a point is roughly 1/160th of an inch
    bool getScreenSizeInPoints(const osg::GraphicsContext::ScreenIdentifier& si, osg::Vec2& pointSize);

protected:

    /** implementation of setScreenResolution */
    //IPad can have extenal screens which we can request a res for
    //the main screen screenNum 0 can not currently have its res changed
    //as it only has one mode (might change though and this should still handle it)
    bool setScreenResolutionImpl(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, unsigned int width, unsigned int height);

    /** implementation of setScreenRefreshRate, currently can't set refresh rate of IOS*/
    bool setScreenRefreshRateImpl(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, double refreshRate);


private:



};

#if 0
template <class WSI>
struct RegisterWindowingSystemInterfaceProxy
{
    RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(new WSI);
    }

    ~RegisterWindowingSystemInterfaceProxy()
    {
        if (osg::Referenced::getDeleteHandler())
        {
            osg::Referenced::getDeleteHandler()->setNumFramesToRetainObjects(0);
            osg::Referenced::getDeleteHandler()->flushAll();
        }

        osg::GraphicsContext::setWindowingSystemInterface(0);
    }
};
#endif


}

#endif

#endif // __APPLE__
