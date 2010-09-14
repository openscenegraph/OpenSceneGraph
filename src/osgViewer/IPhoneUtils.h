/*
 *  IphoneUtils.h
 *  OpenSceneGraph
 *
 *  Created by Thomas Hogarth on 25.11.09.
 *
 */

#ifdef __APPLE__ 
 
#ifndef IPHONE_UTILS_HEADER_
#define IPHONE_UTILS_HEADER_

#include <osg/DeleteHandler>
#include <osg/GraphicsContext>
#include <osgViewer/GraphicsWindow>
#include <Foundation/Foundation.h>

namespace osgIPhone {



struct IPhoneWindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface
{
    public:
        IPhoneWindowingSystemInterface();

        /** dtor */
        ~IPhoneWindowingSystemInterface();

        /** @return a CGDirectDisplayID for a ScreenIdentifier */
        int getDisplayID(const osg::GraphicsContext::ScreenIdentifier& si);

        /** @return count of attached screens */
        virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) ;

        virtual void getScreenSettings(const osg::GraphicsContext::ScreenIdentifier& si, osg::GraphicsContext::ScreenSettings & resolution);

        virtual void enumerateScreenSettings(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, osg::GraphicsContext::ScreenSettingsList & resolutionList);
        
        virtual bool setScreenSettings (const osg::GraphicsContext::ScreenIdentifier & si, const osg::GraphicsContext::ScreenSettings & settings);

        /** return the top left coord of a specific screen in global screen space */
        void getScreenTopLeft(const osg::GraphicsContext::ScreenIdentifier& si, int& x, int& y);

        

        /** returns screen-ndx containing rect x,y,w,h */
        unsigned int getScreenContaining(int x, int y, int w, int h);
    
    protected:
    
        /** implementation of setScreenResolution */
        bool setScreenResolutionImpl(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, unsigned int width, unsigned int height) ;

        /** implementation of setScreenRefreshRate */
        bool setScreenRefreshRateImpl(const osg::GraphicsContext::ScreenIdentifier& screenIdentifier, double refreshRate);

    
        template<class PixelBufferImplementation, class GraphicsWindowImplementation>
        osg::GraphicsContext* createGraphicsContextImplementation(osg::GraphicsContext::Traits* traits)
        {
            if (traits->pbuffer)
            {
                osg::ref_ptr<PixelBufferImplementation> pbuffer = new PixelBufferImplementation(traits);
                if (pbuffer->valid()) return pbuffer.release();
                else return 0;
            }
            else
            {
                osg::ref_ptr<GraphicsWindowImplementation> window = new GraphicsWindowImplementation(traits);
                if (window->valid()) return window.release();
                else return 0;
            }
        }
    
    
    private:
        int        _displayCount;
        int        _displayIds;


};

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



}

#endif

#endif // __APPLE__
