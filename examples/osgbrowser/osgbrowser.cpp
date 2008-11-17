#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>

#include <nsGUIEvent.h>

#include "llmozlib2.h"

class UBrowserImage;

//////////////////////////////////////////////////////////////////////////
//
// UBrowserThread interface
//
class UBrowserThread : public OpenThreads::Thread, public osg::Referenced
{
    public:
    
        
        static osg::ref_ptr<UBrowserThread>& instance();

        void init(const std::string& application);

        virtual void run();
        
        void* getNativeWindowHandle();

        void registerUBrowserImage(UBrowserImage* image)
        {
            _ubrowserImageList.push_back(image);
        }

        void unregisterUBrowserImage(UBrowserImage* image)
        {
            UBrowserImageList::iterator itr = std::find(_ubrowserImageList.begin(), _ubrowserImageList.end(), image);
            if (itr != _ubrowserImageList.end()) _ubrowserImageList.erase(itr);
        }

        void sendKeyEvent(UBrowserImage* image, int key, bool keyDown);
        void sendPointerEvent(UBrowserImage* image, int x, int y, int buttonMask);

    protected:
    
        UBrowserThread();

        virtual ~UBrowserThread();

        void setUpKeyMap();
        int convertToXULKey(int key) const;

        bool        _initialized;
        bool        _done;

        std::string _application;
        void*       _nativeWindowHandle;
        
        typedef std::list< UBrowserImage* > UBrowserImageList;
        UBrowserImageList   _ubrowserImageList;
        
        typedef     std::map<int, int> KeyMap;
        KeyMap      _keyMap;

        int         _previousButtonMask;
};

////////////////////////////////////////////////////////////////////////////////////
//
// UBrowser  interface
class UBrowserImage : public osg::Image, public LLEmbeddedBrowserWindowObserver
{
    public:
    
        UBrowserImage(int width, int height);

        virtual void sendPointerEvent(int x, int y, int buttonMask);

        virtual void sendKeyEvent(int key, bool keyDown);

        void navigateTo(const std::string& page);
        

        ////////////////////////////////////////////////////////////////////////////////
        // virtual
        void onPageChanged( const EventType& eventIn )
        {
            // flag that an update is required - page grab happens in idle() so we don't stall
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
            osg::notify(osg::NOTICE) << "Event: status updated to " << eventIn.getStringValue() << std::endl;
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

        void update();

        int getBrowserWindowId() const { return _browserWindowId; }

    protected:

        virtual ~UBrowserImage();
        
        int         _browserWindowId;
        
        bool        _needsUpdate;
};

//////////////////////////////////////////////////////////////////////////
//
// UBrowserThread implementation
//
UBrowserThread::UBrowserThread():
    _initialized(false),
    _done(false),
    _previousButtonMask(0)
{
}

UBrowserThread::~UBrowserThread()
{
    _done = true;
    cancel();
    while(isRunning()) 
    {
        OpenThreads::Thread::YieldCurrentThread();
    }
}

osg::ref_ptr<UBrowserThread>& UBrowserThread::instance()
{
    static osg::ref_ptr<UBrowserThread> s_ubrowserThread = new UBrowserThread;
    return s_ubrowserThread;
}

void UBrowserThread::init(const std::string& application)
{
    if (_initialized) return;

    _application = application;
    
    // create a single browser window and set things up.
    std::string applicationDir = osgDB::getFilePath(_application);
    if (applicationDir.empty()) applicationDir = osgDB::getRealPath(".");                        
    else applicationDir = osgDB::getRealPath(applicationDir);

    std::string componentDir = "/usr/lib/xulrunner";
    std::string profileDir = applicationDir + "/" + "testGL_profile";
    LLMozLib::getInstance()->init( applicationDir, componentDir, profileDir, getNativeWindowHandle() );

    // append details to agent string
    LLMozLib::getInstance()->setBrowserAgentId( _application );

    setUpKeyMap();

    _initialized = true;
    
    // startThread();
}

#ifdef _WINDOWS
void* UBrowserThread::getNativeWindowHandle()
{
    if (_nativeWindowHandle) return _nativeWindowHandle;
    
    // My implementation of the embedded browser needs a native window handle
    // Can't get this via GLUT so had to use this hack
    _nativeWindowHandle = FindWindow( NULL, _appWindowName.c_str() );

    return _nativeWindowHandle;
}

#else

#include <gtk/gtk.h>

void* UBrowserThread::getNativeWindowHandle()
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

void UBrowserThread::run()
{
    do
    {
    
        //osg::notify(osg::NOTICE)<<"UBrowserThread::run()"<<std::endl;
    
        if (_ubrowserImageList.empty())
        {
            OpenThreads::Thread::YieldCurrentThread();
            continue;
        }
        
    
#ifndef _WINDOWS
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


        for(UBrowserImageList::iterator itr = _ubrowserImageList.begin();
            itr != _ubrowserImageList.end();
            ++itr)
        {
            osg::ref_ptr<UBrowserImage> browser = *(itr);
            if (browser.valid())
            {
                browser->update();
            }
        }

    } while (!_done && !testCancel());
}

void UBrowserThread::setUpKeyMap()
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

int UBrowserThread::convertToXULKey(int key) const
{
    KeyMap::const_iterator itr = _keyMap.find(key);    
    if (_keyMap.find(key)==_keyMap.end()) return key;
    else return itr->second;
    
}

void UBrowserThread::sendPointerEvent(UBrowserImage* image, int x, int y, int buttonMask)
{
    int deltaButton = (buttonMask&1) - (_previousButtonMask&1);
    _previousButtonMask = buttonMask;
    
    int _browserWindowId = image->getBrowserWindowId();

    if (deltaButton>0)
    {
        // send event to LLMozLib
        LLMozLib::getInstance()->mouseDown( _browserWindowId, x, y );
    }
    else if (deltaButton<0)
    {
        // send event to LLMozLib
        LLMozLib::getInstance()->mouseUp( _browserWindowId, x, y );

        // this seems better than sending focus on mouse down (still need to improve this)
        LLMozLib::getInstance()->focusBrowser( _browserWindowId, true );
    } else
    {
        // send event to LLMozLib
        LLMozLib::getInstance()->mouseMove( _browserWindowId, x, y );
    }
}

void UBrowserThread::sendKeyEvent(UBrowserImage* image, int key, bool keyDown)
{
    if (!keyDown) return;

    int _browserWindowId = image->getBrowserWindowId();

    KeyMap::const_iterator itr = _keyMap.find(key);    
    if (_keyMap.find(key)==_keyMap.end()) LLMozLib::getInstance()->unicodeInput( _browserWindowId, key );
    else LLMozLib::getInstance()->keyPress( _browserWindowId, itr->second );
}


////////////////////////////////////////////////////////////////////////////////////
//
// UBrowser  implementation

UBrowserImage::UBrowserImage(int width, int height):
    _browserWindowId(0),
    _needsUpdate(true)
{
    _browserWindowId = LLMozLib::getInstance()->createBrowserWindow( width, height );

    // tell LLMozLib about the size of the browser window
    LLMozLib::getInstance()->setSize( _browserWindowId, width, height );

    // observer events that LLMozLib emits
    LLMozLib::getInstance()->addObserver( _browserWindowId, this );

    // don't flip bitmap
    LLMozLib::getInstance()->flipWindow( _browserWindowId, false );

    LLMozLib::getInstance()->setBackgroundColor( _browserWindowId, 0, 255, 0);

    GLint internalFormat = LLMozLib::getInstance()->getBrowserDepth( _browserWindowId ) == 3 ? GL_RGB : GL_RGBA;
    GLenum pixelFormat = LLMozLib::getInstance()->getBrowserDepth( _browserWindowId ) == 3 ? GL_BGR_EXT : GL_BGRA_EXT;

    setImage(width,height,1, internalFormat, pixelFormat, GL_UNSIGNED_BYTE, 
             (unsigned char*)LLMozLib::getInstance()->getBrowserWindowPixels( _browserWindowId ),
             osg::Image::NO_DELETE);
             
    setDataVariance(osg::Object::DYNAMIC);
    setOrigin(osg::Image::TOP_LEFT);
}

UBrowserImage::~UBrowserImage()
{
}

void UBrowserImage::sendPointerEvent(int x, int y, int buttonMask)
{
    UBrowserThread::instance()->sendPointerEvent(this, x, y, buttonMask);
}

void UBrowserImage::sendKeyEvent(int key, bool keyDown)
{
    UBrowserThread::instance()->sendKeyEvent(this, key, keyDown);
}

void UBrowserImage::navigateTo(const std::string& url)
{
    // go to the "home page"
    LLMozLib::getInstance()->navigateTo( _browserWindowId, url );
}


void UBrowserImage::update()
{
    //if ( _needsUpdate )
    {
        // grab a page but don't reset 'needs update' flag until we've written it to the texture in display()
        LLMozLib::getInstance()->grabBrowserWindow( _browserWindowId );

        int width = LLMozLib::getInstance()->getBrowserRowSpan( _browserWindowId ) / LLMozLib::getInstance()->getBrowserDepth( _browserWindowId );
        int height = LLMozLib::getInstance()->getBrowserHeight( _browserWindowId );
        
        GLint internalFormat = LLMozLib::getInstance()->getBrowserDepth( _browserWindowId ) == 3 ? GL_RGB : GL_RGBA;
        GLenum pixelFormat = LLMozLib::getInstance()->getBrowserDepth( _browserWindowId ) == 3 ? GL_BGR_EXT : GL_BGRA_EXT;

        setImage(width,height,1, internalFormat, pixelFormat, GL_UNSIGNED_BYTE, 
                 (unsigned char*)LLMozLib::getInstance()->getBrowserWindowPixels( _browserWindowId ),
                 osg::Image::NO_DELETE);
                 
        // osg::notify(osg::NOTICE)<<"Image updated "<<(void*)data()<<", "<<s()<<","<<t()<<std::endl;

        // _needsUpdate = false;
    }
}

osg::Node* createInteractiveQuad(const osg::Vec3& origin, osg::Vec3& widthAxis, osg::Vec3& heightAxis, 
                                 osg::Image* image)
{
    bool flip = image->getOrigin()==osg::Image::TOP_LEFT;

    osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(origin, widthAxis, heightAxis,
                                       0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

    osg::Texture2D* texture = new osg::Texture2D(image);
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    
    pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                texture,
                osg::StateAttribute::ON);
                
    pictureQuad->setEventCallback(new osgViewer::InteractiveImageHandler(image));

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(pictureQuad);
    
    return geode;
}


int main( int argc, char* argv[] )
{
    osg::ArgumentParser arguments(&argc, argv);
    
    UBrowserThread::instance()->init(arguments[0]);

    osgViewer::Viewer viewer(arguments);

    typedef std::list< osg::ref_ptr<UBrowserImage> > Images;
    Images images;

    for(int i=1; i<arguments.argc(); ++i)
    {
        if (!arguments.isOption(i))
        {
            osg::ref_ptr<UBrowserImage> browserImage= new UBrowserImage(768, 1024);
            browserImage->navigateTo(arguments[i]);
            images.push_back(browserImage.get());
        }
    }

    bool xyPlane = false;

    osg::Group* group = new osg::Group;

    osg::Vec3 origin = osg::Vec3(0.0f,0.0f,0.0f);
    for(Images::iterator itr = images.begin();
        itr != images.end();
        ++itr)
    {
        osg::Image* image = itr->get();
        float width = 1.0;
        float height = float(image->t())/float(image->s());
        
        osg::Vec3 widthAxis = osg::Vec3(width,0.0f,0.0f);
        osg::Vec3 heightAxis = xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height);
        
        group->addChild(createInteractiveQuad(origin, widthAxis, heightAxis, image));
        
        origin += widthAxis*1.1f;
    }
    
    viewer.setSceneData(group);

    viewer.addEventHandler(new osgViewer::StatsHandler);
    
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    viewer.realize();
    
    while(!viewer.done())
    {

#ifndef _WINDOWS
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


        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {
            osg::ref_ptr<UBrowserImage> browser = *(itr);
            if (browser.valid())
            {
                browser->update();
            }
        }

        viewer.frame();
    }
}
