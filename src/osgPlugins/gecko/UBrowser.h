/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#ifndef UBROWSER_H
#define UBROWSER_H

#include <osgWidget/Browser>

#include <osg/OperationThread>
#include <osg/Timer>

#include <list>
#include <algorithm>

#include <nsGUIEvent.h>
#include "llmozlib2.h"

class UBrowserImage;

//////////////////////////////////////////////////////////////////////////
//
// UBrowserManager interface
//
class UBrowserManager : public osgWidget::BrowserManager
{
    public:

        UBrowserManager();
        
        bool isInitialized() const { return _initialized; }

        virtual void init(const std::string& application);

        virtual osgWidget::BrowserImage* createBrowserImage(const std::string& url, int width, int height);

    public:
 
        void* getNativeWindowHandle();
        
        void registerUBrowserImage(UBrowserImage* image)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_ubrowserImageListMutex);
            _ubrowserImageList.push_back(image);
        }

        void unregisterUBrowserImage(UBrowserImage* image)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_ubrowserImageListMutex);
            UBrowserImageList::iterator itr = std::find(_ubrowserImageList.begin(), _ubrowserImageList.end(), image);
            if (itr != _ubrowserImageList.end()) _ubrowserImageList.erase(itr);
        }

        bool sendKeyEvent(UBrowserImage* image, int key, bool keyDown);
        
        bool sendPointerEvent(UBrowserImage* image, int x, int y, int buttonMask);

        void navigateTo(UBrowserImage* image, const std::string& page);

        typedef std::list< UBrowserImage* > UBrowserImageList;

        OpenThreads::Mutex  _ubrowserImageListMutex;
        UBrowserImageList   _ubrowserImageList;
        
        void active(UBrowserImage* image);

    protected:
    
        UBrowserManager(const UBrowserManager& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) {}

        virtual ~UBrowserManager();
        
        META_Object(osgWidget,UBrowserManager)

        void setUpKeyMap();
        int convertToXULKey(int key) const;

        bool        _initialized;

        void*       _nativeWindowHandle;
        
        typedef     std::map<int, int> KeyMap;
        KeyMap      _keyMap;

        int         _previousButtonMask;
        
        osg::ref_ptr<osg::OperationThread> _thread;
};

////////////////////////////////////////////////////////////////////////////////////
//
// UBrowser  interface
class UBrowserImage : public osgWidget::BrowserImage, public LLEmbeddedBrowserWindowObserver
{
    public:
    
        UBrowserImage(UBrowserManager* manager, const std::string& homeURL, int width, int height);


        const std::string& getHomeURL() const { return _homeURL; }

        virtual bool sendPointerEvent(int x, int y, int buttonMask);

        virtual bool sendKeyEvent(int key, bool keyDown);

        virtual void setFrameLastRendered(const osg::FrameStamp* frameStamp);

        virtual void navigateTo(const std::string& url);
        
        

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onPageChanged( const EventType& eventIn )
        {
            // flag that an update is required - page grab happens in idle() so we don't stall
            osg::notify(osg::NOTICE) << "Event: onPageChanged " << eventIn.getEventUri() << std::endl;
            _needsUpdate = true;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onNavigateBegin( const EventType& eventIn )
        {
            osg::notify(osg::NOTICE) << "Event: begin navigation to " << eventIn.getEventUri() << std::endl;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onNavigateComplete( const EventType& eventIn )
        {
            osg::notify(osg::NOTICE) << "Event: end navigation to " << eventIn.getEventUri() << " with response status of " << eventIn.getIntValue() << std::endl;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onUpdateProgress( const EventType& eventIn )
        {
            osg::notify(osg::NOTICE) << "Event: progress value updated to " << eventIn.getIntValue() << std::endl;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onStatusTextChange( const EventType& eventIn )
        {
            osg::notify(osg::INFO) << "Event: status updated to " << eventIn.getStringValue() << std::endl;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onLocationChange( const EventType& eventIn )
        {
            osg::notify(osg::NOTICE) << "Event: location changed to " << eventIn.getStringValue() << std::endl;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onClickLinkHref( const EventType& eventIn )
        {
            osg::notify(osg::NOTICE) << "Event: clicked on link to " << eventIn.getStringValue() << std::endl;
        };

        void setBrowserWindowId(int id) { _browserWindowId = id; }
        int getBrowserWindowId() const { return _browserWindowId; }

        osg::ref_ptr<UBrowserManager> _manager;
        
        void updated();
        
        double getTimeOfLastUpdate() const { return _timeOfLastUpdate; }
        double getTimeOfLastRender() const { return _timeOfLastRender; }

        double time() const { return osg::Timer::instance()->time_s(); }

    protected:

        virtual ~UBrowserImage();
        
        int         _browserWindowId;        
        bool        _needsUpdate;
        std::string _homeURL;
        
        double      _timeOfLastUpdate;
        double      _timeOfLastRender;
};

#endif
