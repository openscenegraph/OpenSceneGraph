/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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

/* Note, elements of GraphicsWindowX11 have used Prodcer/RenderSurface_X11.cpp as both
 * a guide to use of X11/GLX and copiying directly in the case of setBorder().
 * These elements are license under OSGPL as above, with Copyright (C) 2001-2004  Don Burns.
 */

#ifndef OSGVIEWER_GRAPHICSWINDOWCOCOA
#define OSGVIEWER_GRAPHICSWINDOWCOCOA 1

#ifdef __APPLE__

#ifdef __OBJC__
@class GraphicsWindowCocoaWindow;
@class GraphicsWindowCocoaGLView;
@class NSOpenGLContext;
@class NSOpenGLPixelFormat;
@class NSWindow;
@class NSView;
#else
class GraphicsWindowCocoaGLView;
class GraphicsWindowCocoaWindow;
class NSOpenGLContext;
class NSOpenGLPixelFormat;
class NSWindow;
class NSView;
#endif

#include <osgViewer/GraphicsWindow>
#include <osgViewer/api/Cocoa/GraphicsHandleCocoa>

// we may not include any cocoa-header here, because this will pollute the name-sapce and tend to compile-errors

namespace osgViewer
{

class GraphicsWindowCocoa : public osgViewer::GraphicsWindow, public osgViewer::GraphicsHandleCocoa
{
    public:
    class Implementation;

        GraphicsWindowCocoa(osg::GraphicsContext::Traits* traits):
            osgViewer::GraphicsWindow(),
            osgViewer::GraphicsHandleCocoa(),
            _valid(false),
            _initialized(false),
            _realized(false),
            _closeRequested(false),
            _checkForEvents(true),
            _ownsWindow(true),
            _currentCursor(RightArrowCursor),
            _window(NULL),
            _view(NULL),
            _context(NULL),
            _pixelformat(NULL),
            _updateContext(false),
            _multiTouchEnabled(false)
        {
            _traits = traits;

            init();
            
            if (valid())
            {
                setState( new osg::State );
                getState()->setGraphicsContext(this);

                if (_traits.valid() && _traits->sharedContext.valid())
                {
                    getState()->setContextID( _traits->sharedContext->getState()->getContextID() );
                    incrementContextIDUsageCount( getState()->getContextID() );   
                }
                else
                {
                    getState()->setContextID( osg::GraphicsContext::createNewContextID() );
                }
            }
        }
    
        virtual bool isSameKindAs(const Object* object) const { return dynamic_cast<const GraphicsWindowCocoa*>(object)!=0; }
        virtual const char* libraryName() const { return "osgViewer"; }
        virtual const char* className() const { return "GraphicsWindowCocoa"; }

        virtual bool valid() const { return _valid; }

        /** Realise the GraphicsContext.*/
        virtual bool realizeImplementation();

        /** Return true if the graphics context has been realised and is ready to use.*/
        virtual bool isRealizedImplementation() const { return _realized; }

        /** Close the graphics context.*/
        virtual void closeImplementation();

        /** Make this graphics context current.*/
        virtual bool makeCurrentImplementation();
        
        /** Release the graphics context.*/
        virtual bool releaseContextImplementation();

        /** Swap the front and back buffers.*/
        virtual void swapBuffersImplementation();
        
        /** Check to see if any events have been generated.*/
        virtual bool checkEvents();

        /** Set Window decoration.*/
        virtual bool setWindowDecorationImplementation(bool flag);

        /** Get focus.*/
        virtual void grabFocus();
        
        /** Get focus on if the pointer is in this window.*/
        virtual void grabFocusIfPointerInWindow();
        
        bool requestClose() { bool b = _closeRequested; _closeRequested = true; return b; }
        
        virtual void resizedImplementation(int x, int y, int width, int height);
        
        virtual bool setWindowRectangleImplementation(int x, int y, int width, int height);
        
        virtual void setWindowName (const std::string & name);
        virtual void requestWarpPointer(float x,float y);
        virtual void useCursor(bool cursorOn);
        virtual void setCursor(MouseCursor mouseCursor);
               
        /** WindowData is used to pass in the Cocoa window handle attached the GraphicsContext::Traits structure. */
        class WindowData : public osg::Referenced
        {
            public:
                enum Options { CreateOnlyView = 1, CheckForEvents = 2, PoseAsStandaloneApp = 4, EnableMultiTouch = 8};
                WindowData(unsigned int options)
                :   _createOnlyView(options & CreateOnlyView),
                    _checkForEvents(options & CheckForEvents),
                    _poseAsStandaloneApp(options & PoseAsStandaloneApp),
                    _multiTouchEnabled(options & EnableMultiTouch),
                    _view(NULL)
                {
                }
                            
                inline NSView* getCreatedNSView() { return _view; }
                bool createOnlyView() const { return _createOnlyView; }
                bool checkForEvents() const { return _checkForEvents; }
                bool poseAsStandaloneApp() const { return _poseAsStandaloneApp; }
                bool isMultiTouchEnabled() const { return _multiTouchEnabled; }
            
            protected:
                inline void setCreatedNSView(NSView* view) { _view = view; }
            
            private:
                bool         _createOnlyView, _checkForEvents, _poseAsStandaloneApp, _multiTouchEnabled;
                NSView*         _view;
            
            friend class GraphicsWindowCocoa;

        };
        
        NSOpenGLContext* getContext() { return _context; }
        GraphicsWindowCocoaWindow* getWindow() { return _window; }
        NSOpenGLPixelFormat* getPixelFormat() { return _pixelformat; }
                
        virtual void setSyncToVBlank(bool f);
        
        /** adapts a resize / move of the window, coords in global screen space */
        void adaptResize(int x, int y, int w, int h);
        
        bool isMultiTouchEnabled();
        void setMultiTouchEnabled(bool b);
        
    protected:
    
        void init();
        
        void transformMouseXY(float& x, float& y);
        void setupNSWindow(NSWindow* win);
        
        
        virtual ~GraphicsWindowCocoa();


        bool            _valid;
        bool            _initialized;
        bool            _realized;
        bool            _useWindowDecoration;

    
         
    private:        
       
        
        bool                            _closeRequested, _checkForEvents,_ownsWindow;
        MouseCursor                     _currentCursor;
        GraphicsWindowCocoaWindow*      _window;
        GraphicsWindowCocoaGLView*      _view;
        NSOpenGLContext*                _context;
        NSOpenGLPixelFormat*            _pixelformat;
        bool                            _updateContext, _multiTouchEnabled;
};

}

#endif
#endif
