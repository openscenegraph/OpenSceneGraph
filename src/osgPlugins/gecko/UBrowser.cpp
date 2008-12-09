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

#include "UBrowser.h"

#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/io_utils>
#include <osg/GraphicsThread>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#ifdef _WINDOWS
    #include <windows.h>
#elif defined(__APPLE__)
    #include <Carbon/Carbon.h>
#else
    extern "C" {
    #include <gtk/gtk.h>
    }
#endif

//////////////////////////////////////////////////////////////////////////
//
// UBrowserManager implementation
//
UBrowserManager::UBrowserManager():
    _initialized(false),
    _nativeWindowHandle(0),
    _previousButtonMask(0)
{
    osg::notify(osg::NOTICE)<<"UBrowserManager::UBrowserManager()"<<std::endl;
}

UBrowserManager::~UBrowserManager()
{
    _thread->setDone(true);
    
    while(_thread->isRunning()) 
    {
        OpenThreads::Thread::YieldCurrentThread();
    }

    _thread = 0;
}

osgWidget::BrowserImage* UBrowserManager::createBrowserImage(const std::string& url, int width, int height)
{
    return new UBrowserImage(this, url, width, height);    
}

#if 1

#if defined(_WINDOWS)
void* UBrowserManager::getNativeWindowHandle()
{
    if (_nativeWindowHandle) return _nativeWindowHandle;
    
    // My implementation of the embedded browser needs a native window handle
    // Can't get this via GLUT so had to use this hack
    _nativeWindowHandle = FindWindow( NULL, _appWindowName.c_str() );

    return _nativeWindowHandle;
}

#elif defined(__APPLE__)
void* UBrowserManager::getNativeWindowHandle()
{
    if (_nativeWindowHandle) return _nativeWindowHandle;

    // Create a window just for this purpose.
    Rect window_rect = {100, 100, 200, 200};

    _nativeWindowHandle = (void*) NewCWindow(
        NULL,
        &window_rect,
        "\p",
        false,                // Create the window invisible.
        zoomDocProc,        // Window with a grow box and a zoom box
        kLastWindowOfClass,        // create it behind other windows
        false,                    // no close box
        0);
    }
    
    return _nativeWindowHandle;
}
#else
void* UBrowserManager::getNativeWindowHandle()
{
    if (_nativeWindowHandle) return _nativeWindowHandle;

    gtk_disable_setlocale();

    gtk_init(NULL, NULL);

    GtkWidget *win = gtk_window_new(GTK_WINDOW_POPUP);
    // Why a layout widget?  A MozContainer would be ideal, but
    // it involves exposing Mozilla headers to mozlib-using apps.
    // A layout widget with a GtkWindow parent has the desired
    // properties of being plain GTK, having a window, and being
    // derived from a GtkContainer.
    GtkWidget *rtnw = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(win), rtnw);

    gtk_widget_realize(rtnw);
    GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(rtnw), GTK_NO_WINDOW);

    _nativeWindowHandle = rtnw;
    
    return _nativeWindowHandle;
}
#endif

#endif

struct InitOperation : public osg::Operation
{
    InitOperation():
        Operation("init",false) {}

    /** Override the standard Operation operator and dynamic cast object to a GraphicsContext,
      * on success call operation()(GraphicsContext*).*/
    virtual void operator () (osg::Object* object)
    {
        osg::notify(osg::NOTICE)<<"InitOperation begin"<<std::endl;

        UBrowserManager* ubrowserManager = dynamic_cast<UBrowserManager*>(object);
    
        osg::notify(osg::NOTICE)<<"InitOperation ubrowserManager="<<ubrowserManager<<std::endl;

        // create a single browser window and set things up.
        std::string applicationDir = osgDB::getFilePath(ubrowserManager->getApplication());
        if (applicationDir.empty()) applicationDir = osgDB::getRealPath(".");                        
        else applicationDir = osgDB::getRealPath(applicationDir);
        
        std::string componentDir = ADDQUOTES(XUL_DIR);
        std::string profileDir = applicationDir + "/" + ".profile";

        osg::notify(osg::NOTICE)<<"applicationDir="<<applicationDir<<std::endl;
        osg::notify(osg::NOTICE)<<"componentDir="<<componentDir<<std::endl;
        osg::notify(osg::NOTICE)<<"profileDir="<<profileDir<<std::endl;
        osg::notify(osg::NOTICE)<<"ubrowserManager->getNativeWindowHandle()="<<ubrowserManager->getNativeWindowHandle()<<std::endl;

        osg::notify(osg::NOTICE)<<"before LLMozLib init() "<<std::endl;

        LLMozLib::getInstance()->init( applicationDir, componentDir, profileDir, ubrowserManager->getNativeWindowHandle() );

        osg::notify(osg::NOTICE)<<"done LLMozLib init() "<<std::endl;

        // append details to agent string
        LLMozLib::getInstance()->setBrowserAgentId( ubrowserManager->getApplication() );

        osg::notify(osg::NOTICE)<<"InitOperation end"<<std::endl;
    }

};

struct UpdateOperation : public osg::Operation
{
    UpdateOperation():
        osg::Operation("update",true) {}

    virtual void operator () (osg::Object* object)
    {
        UBrowserManager* ubrowserManager = dynamic_cast<UBrowserManager*>(object);

        // osg::notify(osg::NOTICE)<<"Update"<<std::endl;

        if (ubrowserManager->_ubrowserImageList.empty())
        {
            // osg::notify(osg::NOTICE)<<"Nothing to do"<<std::endl;

            OpenThreads::Thread::YieldCurrentThread();
            return;
        }
            
    #if !defined(_WINDOWS) && !defined(__APPLE__)
        // pump the GTK+Gecko event queue for a (limited) while.  this should
        // be done so that the Gecko event queue doesn't starve, and done
        // *here* so that mNeedsUpdate[] can be populated by callbacks
        // from Gecko.
        gtk_main_iteration_do(0);
        for (int iter=0; iter<10; ++iter)
        {
            if (gtk_events_pending())
                gtk_main_iteration();
        }
    #endif

        typedef std::list< osg::ref_ptr<UBrowserImage> > RefImageList;
        RefImageList images;
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(ubrowserManager->_ubrowserImageListMutex);
            std::copy(ubrowserManager->_ubrowserImageList.begin(), 
                      ubrowserManager->_ubrowserImageList.end(),
                      std::back_inserter(images));
        }

        int numUpdated = 0;

        for(RefImageList::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {
            if (update(itr->get())) ++numUpdated;
        }
        
        if (numUpdated==0)
        {
            //osg::notify(osg::NOTICE)<<"completed Update but no images updated"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
        else
        {
            //osg::notify(osg::NOTICE)<<"completed Updated "<<numUpdated<<std::endl;
        }

    }
    
    bool update(UBrowserImage* image)
    {
        if (!image) return false;
    
        double deltaTime = image->getTimeOfLastRender() - image->getTimeOfLastUpdate();

//        osg::notify(osg::NOTICE)<<"deltaTime = "<<deltaTime<<std::endl;
//        osg::notify(osg::NOTICE)<<"    image->getTimeOfLastRender() = "<<image->getTimeOfLastRender()<<std::endl;
//        osg::notify(osg::NOTICE)<<"    image->getTimeOfLastUpdate() = "<<image->getTimeOfLastUpdate()<<std::endl;

        if (deltaTime<0.0)
        {
            return false;
        }

        int id = image->getBrowserWindowId();

        if (id==0)
        {
            int width = image->s();
            int height = image->t();

            osg::notify(osg::INFO)<<"Constructing browser window for first time, width = "<<width<<" height = "<<height<<std::endl;

            id = LLMozLib::getInstance()->createBrowserWindow( width, height );
            
            image->setBrowserWindowId(id);

            // tell LLMozLib about the size of the browser window
            LLMozLib::getInstance()->setSize( id, width, height );

            // observer events that LLMozLib emits
            LLMozLib::getInstance()->addObserver( id, image );

            // don't flip bitmap
            LLMozLib::getInstance()->flipWindow( id, false );

            LLMozLib::getInstance()->navigateTo( id, image->getHomeURL() );

        }


        //if ( _needsUpdate )
        {
            // grab a page but don't reset 'needs update' flag until we've written it to the texture in display()
            LLMozLib::getInstance()->grabBrowserWindow( id );

            int width = LLMozLib::getInstance()->getBrowserRowSpan( id ) / LLMozLib::getInstance()->getBrowserDepth( id );
            int height = LLMozLib::getInstance()->getBrowserHeight( id );

            GLint internalFormat = LLMozLib::getInstance()->getBrowserDepth( id ) == 3 ? GL_RGB : GL_RGBA;
            GLenum pixelFormat = LLMozLib::getInstance()->getBrowserDepth( id ) == 3 ? GL_BGR_EXT : GL_BGRA_EXT;

            // osg::notify(osg::NOTICE)<<"  doing image update "<<std::endl;

            image->setImage(width,height,1, internalFormat, pixelFormat, GL_UNSIGNED_BYTE, 
                     (unsigned char*)LLMozLib::getInstance()->getBrowserWindowPixels( id ),
                     osg::Image::NO_DELETE);
                     
            image->updated();

            // _needsUpdate = false;
        }
        
        return true;
    }
};

void UBrowserManager::init(const std::string& application)
{
    if (_initialized) return;

    _application = application;
    
    setUpKeyMap();
    
    _thread = new osg::OperationThread;
    _thread->setParent(this);

    osg::ref_ptr<osg::BarrierOperation> barrier = new osg::BarrierOperation(2,osg::BarrierOperation::NO_OPERATION, false);

    _thread->startThread();
    _thread->add(new InitOperation());
    _thread->add(barrier.get());
    _thread->add(new UpdateOperation());

    // wait till the init has been run.
    barrier->block();

    _initialized = true;
}



void UBrowserManager::setUpKeyMap()
{
    _keyMap[osgGA::GUIEventAdapter::KEY_BackSpace] = nsIDOMKeyEvent::DOM_VK_BACK_SPACE;
    _keyMap[osgGA::GUIEventAdapter::KEY_Tab] = nsIDOMKeyEvent::DOM_VK_TAB;
    _keyMap[osgGA::GUIEventAdapter::KEY_Linefeed] = nsIDOMKeyEvent::DOM_VK_ENTER;
    _keyMap[osgGA::GUIEventAdapter::KEY_Clear] = nsIDOMKeyEvent::DOM_VK_CLEAR;
    _keyMap[osgGA::GUIEventAdapter::KEY_Return] = nsIDOMKeyEvent::DOM_VK_RETURN;
    _keyMap[osgGA::GUIEventAdapter::KEY_Pause] = nsIDOMKeyEvent::DOM_VK_PAUSE;
    _keyMap[osgGA::GUIEventAdapter::KEY_Scroll_Lock] = nsIDOMKeyEvent::DOM_VK_SCROLL_LOCK;
    _keyMap[osgGA::GUIEventAdapter::KEY_Escape] = nsIDOMKeyEvent::DOM_VK_ESCAPE;
    _keyMap[osgGA::GUIEventAdapter::KEY_Delete] = nsIDOMKeyEvent::DOM_VK_DELETE;

    /* Cursor control & motion */
    _keyMap[osgGA::GUIEventAdapter::KEY_Home] = nsIDOMKeyEvent::DOM_VK_HOME;
    _keyMap[osgGA::GUIEventAdapter::KEY_Left] = nsIDOMKeyEvent::DOM_VK_LEFT;
    _keyMap[osgGA::GUIEventAdapter::KEY_Up] = nsIDOMKeyEvent::DOM_VK_UP;
    _keyMap[osgGA::GUIEventAdapter::KEY_Right] = nsIDOMKeyEvent::DOM_VK_RIGHT;
    _keyMap[osgGA::GUIEventAdapter::KEY_Down] = nsIDOMKeyEvent::DOM_VK_DOWN;
    _keyMap[osgGA::GUIEventAdapter::KEY_Page_Up] = nsIDOMKeyEvent::DOM_VK_PAGE_UP;
    _keyMap[osgGA::GUIEventAdapter::KEY_Page_Down] = nsIDOMKeyEvent::DOM_VK_PAGE_DOWN;
    _keyMap[osgGA::GUIEventAdapter::KEY_End] = nsIDOMKeyEvent::DOM_VK_END;

    /* Misc Functions */
    _keyMap[osgGA::GUIEventAdapter::KEY_Print] = nsIDOMKeyEvent::DOM_VK_PRINTSCREEN;
    _keyMap[osgGA::GUIEventAdapter::KEY_Insert] = nsIDOMKeyEvent::DOM_VK_INSERT;
    _keyMap[osgGA::GUIEventAdapter::KEY_Cancel] = nsIDOMKeyEvent::DOM_VK_CANCEL;
    _keyMap[osgGA::GUIEventAdapter::KEY_Num_Lock] = nsIDOMKeyEvent::DOM_VK_NUM_LOCK;
    

    /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Space] = nsIDOMKeyEvent::DOM_VK_SPACE;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Tab] = nsIDOMKeyEvent::DOM_VK_TAB;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Enter] = nsIDOMKeyEvent::DOM_VK_ENTER;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Home] = nsIDOMKeyEvent::DOM_VK_HOME;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Left] = nsIDOMKeyEvent::DOM_VK_LEFT;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Up] = nsIDOMKeyEvent::DOM_VK_UP;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Right] = nsIDOMKeyEvent::DOM_VK_RIGHT;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Down] = nsIDOMKeyEvent::DOM_VK_DOWN;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Page_Up] = nsIDOMKeyEvent::DOM_VK_PAGE_UP;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Page_Down] = nsIDOMKeyEvent::DOM_VK_PAGE_DOWN;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_End] = nsIDOMKeyEvent::DOM_VK_END;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Insert] = nsIDOMKeyEvent::DOM_VK_INSERT;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Delete] = nsIDOMKeyEvent::DOM_VK_DELETE;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Equal] = nsIDOMKeyEvent::DOM_VK_EQUALS;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Multiply] = nsIDOMKeyEvent::DOM_VK_MULTIPLY;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Add] = nsIDOMKeyEvent::DOM_VK_ADD;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Separator] = nsIDOMKeyEvent::DOM_VK_SEPARATOR;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Subtract] = nsIDOMKeyEvent::DOM_VK_SUBTRACT;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Decimal] = nsIDOMKeyEvent::DOM_VK_DECIMAL;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_Divide] = nsIDOMKeyEvent::DOM_VK_DIVIDE;

    _keyMap[osgGA::GUIEventAdapter::KEY_KP_0] = nsIDOMKeyEvent::DOM_VK_NUMPAD0;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_1] = nsIDOMKeyEvent::DOM_VK_NUMPAD1;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_2] = nsIDOMKeyEvent::DOM_VK_NUMPAD2;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_3] = nsIDOMKeyEvent::DOM_VK_NUMPAD3;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_4] = nsIDOMKeyEvent::DOM_VK_NUMPAD4;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_5] = nsIDOMKeyEvent::DOM_VK_NUMPAD5;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_6] = nsIDOMKeyEvent::DOM_VK_NUMPAD6;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_7] = nsIDOMKeyEvent::DOM_VK_NUMPAD7;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_8] = nsIDOMKeyEvent::DOM_VK_NUMPAD8;
    _keyMap[osgGA::GUIEventAdapter::KEY_KP_9] = nsIDOMKeyEvent::DOM_VK_NUMPAD9;

    /*
     * Auxiliary Functions; note the duplicate definitions for left and right
     * function keys;  Sun keyboards and a few other manufactures have such
     * function key groups on the left and/or right sides of the keyboard.
     * We've not found a keyboard with more than 35 function keys total.
     */

    _keyMap[osgGA::GUIEventAdapter::KEY_F1] = nsIDOMKeyEvent::DOM_VK_F1;
    _keyMap[osgGA::GUIEventAdapter::KEY_F2] = nsIDOMKeyEvent::DOM_VK_F2;
    _keyMap[osgGA::GUIEventAdapter::KEY_F3] = nsIDOMKeyEvent::DOM_VK_F3;
    _keyMap[osgGA::GUIEventAdapter::KEY_F4] = nsIDOMKeyEvent::DOM_VK_F4;
    _keyMap[osgGA::GUIEventAdapter::KEY_F5] = nsIDOMKeyEvent::DOM_VK_F5;
    _keyMap[osgGA::GUIEventAdapter::KEY_F6] = nsIDOMKeyEvent::DOM_VK_F6;
    _keyMap[osgGA::GUIEventAdapter::KEY_F7] = nsIDOMKeyEvent::DOM_VK_F7;
    _keyMap[osgGA::GUIEventAdapter::KEY_F8] = nsIDOMKeyEvent::DOM_VK_F8;
    _keyMap[osgGA::GUIEventAdapter::KEY_F9] = nsIDOMKeyEvent::DOM_VK_F9;
    _keyMap[osgGA::GUIEventAdapter::KEY_F10] = nsIDOMKeyEvent::DOM_VK_F10;
    _keyMap[osgGA::GUIEventAdapter::KEY_F11] = nsIDOMKeyEvent::DOM_VK_F12;
    _keyMap[osgGA::GUIEventAdapter::KEY_F12] = nsIDOMKeyEvent::DOM_VK_F13;
    _keyMap[osgGA::GUIEventAdapter::KEY_F13] = nsIDOMKeyEvent::DOM_VK_F13;
    _keyMap[osgGA::GUIEventAdapter::KEY_F14] = nsIDOMKeyEvent::DOM_VK_F14;
    _keyMap[osgGA::GUIEventAdapter::KEY_F15] = nsIDOMKeyEvent::DOM_VK_F15;
    _keyMap[osgGA::GUIEventAdapter::KEY_F16] = nsIDOMKeyEvent::DOM_VK_F16;
    _keyMap[osgGA::GUIEventAdapter::KEY_F17] = nsIDOMKeyEvent::DOM_VK_F17;
    _keyMap[osgGA::GUIEventAdapter::KEY_F18] = nsIDOMKeyEvent::DOM_VK_F18;
    _keyMap[osgGA::GUIEventAdapter::KEY_F19] = nsIDOMKeyEvent::DOM_VK_F19;
    _keyMap[osgGA::GUIEventAdapter::KEY_F20] = nsIDOMKeyEvent::DOM_VK_F20;
    _keyMap[osgGA::GUIEventAdapter::KEY_F21] = nsIDOMKeyEvent::DOM_VK_F21;
    _keyMap[osgGA::GUIEventAdapter::KEY_F22] = nsIDOMKeyEvent::DOM_VK_F22;
    _keyMap[osgGA::GUIEventAdapter::KEY_F23] = nsIDOMKeyEvent::DOM_VK_F23;
    _keyMap[osgGA::GUIEventAdapter::KEY_F24] = nsIDOMKeyEvent::DOM_VK_F24;
 
 
    /* Modifiers */
    _keyMap[osgGA::GUIEventAdapter::KEY_Meta_L] = nsIDOMKeyEvent::DOM_VK_META;
    _keyMap[osgGA::GUIEventAdapter::KEY_Meta_R] = nsIDOMKeyEvent::DOM_VK_META;

    _keyMap[osgGA::GUIEventAdapter::KEY_Control_L] = nsIDOMKeyEvent::DOM_VK_CONTROL;
    _keyMap[osgGA::GUIEventAdapter::KEY_Control_R] = nsIDOMKeyEvent::DOM_VK_CONTROL;
    _keyMap[osgGA::GUIEventAdapter::KEY_Shift_L] = nsIDOMKeyEvent::DOM_VK_SHIFT;
    _keyMap[osgGA::GUIEventAdapter::KEY_Shift_R] = nsIDOMKeyEvent::DOM_VK_SHIFT;
    _keyMap[osgGA::GUIEventAdapter::KEY_Alt_R] = nsIDOMKeyEvent::DOM_VK_ALT;
    _keyMap[osgGA::GUIEventAdapter::KEY_Alt_L] = nsIDOMKeyEvent::DOM_VK_ALT;

    _keyMap[osgGA::GUIEventAdapter::KEY_Caps_Lock] = nsIDOMKeyEvent::DOM_VK_CAPS_LOCK;
}

int UBrowserManager::convertToXULKey(int key) const
{
    KeyMap::const_iterator itr = _keyMap.find(key);    
    if (_keyMap.find(key)==_keyMap.end()) return key;
    else return itr->second;
    
}

struct PointerEventOperation : public osg::Operation
{
    PointerEventOperation(UBrowserImage* image, int x, int y, int buttonDelta):
        osg::Operation("pointer event",false),
        _image(image),
        _x(x),
        _y(y),
        _buttonDelta(buttonDelta) {}

    virtual void operator () (osg::Object* object)
    {
        int id = _image->getBrowserWindowId();

        // send event to LLMozLib
        if (_buttonDelta>0) 
        {
            LLMozLib::getInstance()->mouseDown( id, _x, _y );
        }
        else if (_buttonDelta<0) 
        {
            LLMozLib::getInstance()->mouseUp( id, _x, _y );

            // this seems better than sending focus on mouse down (still need to improve this)
            LLMozLib::getInstance()->focusBrowser( id, true );
        }
        else
        {
            // send event to LLMozLib
            LLMozLib::getInstance()->mouseMove( id, _x, _y );
        }
        
    }
    
    osg::ref_ptr<UBrowserImage> _image;
    int _x;
    int _y;
    int _buttonDelta;
};

bool UBrowserManager::sendPointerEvent(UBrowserImage* image, int x, int y, int buttonMask)
{
    int deltaButton = (buttonMask&1) - (_previousButtonMask&1);
    _previousButtonMask = buttonMask;
    
    _thread->add(new PointerEventOperation(image, x, y, deltaButton));
    
    active(image);
    
    return true;
}


struct KeyEventOperation : public osg::Operation
{
    KeyEventOperation(UBrowserImage* image, int key, bool isUnicode):
        osg::Operation("key event",false),
        _image(image),
        _key(key),
        _isUnicode(isUnicode) {}

    virtual void operator () (osg::Object* object)
    {
        int id = _image->getBrowserWindowId();
        if (_isUnicode) LLMozLib::getInstance()->unicodeInput( id, _key );
        else LLMozLib::getInstance()->keyPress( id, _key );
    }
    
    osg::ref_ptr<UBrowserImage> _image;
    int _key;
    bool _isUnicode;
};

bool UBrowserManager::sendKeyEvent(UBrowserImage* image, int key, bool keyDown)
{
    if (!keyDown) return false;

    KeyMap::const_iterator itr = _keyMap.find(key);    
    if (_keyMap.find(key)==_keyMap.end()) _thread->add(new KeyEventOperation(image, key, true));
    else _thread->add(new KeyEventOperation(image, itr->second, false));

    active(image);
    
    return true;
}


struct NavigateToOperation : public osg::Operation
{
    NavigateToOperation(UBrowserImage* image, const std::string& url):
        osg::Operation("key event",false),
        _image(image),
        _url(url) {}

    virtual void operator () (osg::Object* object)
    {
        int id = _image->getBrowserWindowId();
        LLMozLib::getInstance()->navigateTo( id, _url );
    }
    
    osg::ref_ptr<UBrowserImage> _image;
    std::string _url;
};

void UBrowserManager::navigateTo(UBrowserImage* image, const std::string& url)
{
    _thread->add(new NavigateToOperation(image, url));

    active(image);
}

void UBrowserManager::active(UBrowserImage* image)
{
}

////////////////////////////////////////////////////////////////////////////////////
//
// UBrowser  implementation

UBrowserImage::UBrowserImage(UBrowserManager* manager, const std::string& homeURL, int width, int height):
    _browserWindowId(0),
    _needsUpdate(true),
    _timeOfLastUpdate(0.0),
    _timeOfLastRender(0.0)
{
    _manager = manager;

    GLint internalFormat = GL_RGB;
    GLenum pixelFormat = GL_BGR_EXT;

    setImage(width,height,1, internalFormat, pixelFormat, GL_UNSIGNED_BYTE, 
             0,
             osg::Image::NO_DELETE);

    setDataVariance(osg::Object::DYNAMIC);
    setOrigin(osg::Image::TOP_LEFT);
    
    _homeURL = homeURL;

    manager->registerUBrowserImage(this);

}

UBrowserImage::~UBrowserImage()
{
    _manager->unregisterUBrowserImage(this);
}

bool UBrowserImage::sendPointerEvent(int x, int y, int buttonMask)
{
    return _manager->sendPointerEvent(this, x, y, buttonMask);
}

bool UBrowserImage::sendKeyEvent(int key, bool keyDown)
{
    return _manager->sendKeyEvent(this, key, keyDown);
}

void UBrowserImage::setFrameLastRendered(const osg::FrameStamp*)
{
    _timeOfLastRender = time();
    _manager->active(this);
}

void UBrowserImage::updated()
{
    _timeOfLastUpdate = time();
}

void UBrowserImage::navigateTo(const std::string& url)
{
    _manager->navigateTo(this, url);
}

