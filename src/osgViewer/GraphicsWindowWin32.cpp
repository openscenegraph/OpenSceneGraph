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

#include <osgViewer/GraphicsWindowWin32>


using namespace osgViewer;

namespace osgViewer
{

/** This is the class we need to create for pbuffers, note its not a GraphicsWindow as it won't need any of the event handling and window mapping facilities.*/
class GraphicsContextWin32 : public osg::GraphicsContext
{
    public:

        GraphicsContextWin32(osg::GraphicsContext::Traits* traits):
            _valid(false)
        {
            _traits = traits;
        }
    
        virtual bool valid() const { return _valid; }

        /** Realise the GraphicsContext implementation, 
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual bool realizeImplementation() { osg::notify(osg::NOTICE)<<"GraphicsWindow::realizeImplementation() not implemented."<<std::endl; return false; }

        /** Return true if the graphics context has been realised, and is ready to use, implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual bool isRealizedImplementation() const  { osg::notify(osg::NOTICE)<<"GraphicsWindow::isRealizedImplementation() not implemented."<<std::endl; return false; }

        /** Close the graphics context implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void closeImplementation()  { osg::notify(osg::NOTICE)<<"GraphicsWindow::closeImplementation() not implemented."<<std::endl; }

        /** Make this graphics context current implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void makeCurrentImplementation()  { osg::notify(osg::NOTICE)<<"GraphicsWindow::makeCurrentImplementation() not implemented."<<std::endl; }
        
        /** Make this graphics context current with specified read context implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void makeContextCurrentImplementation(GraphicsContext* /*readContext*/)  { osg::notify(osg::NOTICE)<<"GraphicsWindow::makeContextCurrentImplementation(..) not implemented."<<std::endl; }

        /** Pure virtual, Bind the graphics context to associated texture implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual void bindPBufferToTextureImplementation(GLenum /*buffer*/)  { osg::notify(osg::NOTICE)<<"GraphicsWindow::void bindPBufferToTextureImplementation(..) not implemented."<<std::endl; }

        /** Swap the front and back buffers implementation.
          * Pure virtual - must be implemented by Concrate implementations of GraphicsContext. */
        virtual void swapBuffersImplementation()  { osg::notify(osg::NOTICE)<<"GraphicsWindow:: swapBuffersImplementation() not implemented."<<std::endl; }
        
    protected:
        
        bool        _valid;

};

}

void GraphicsWindowWin32::setWindowDecoration(bool flag)
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::setWindowDecoration() Please implement me!"<<std::endl;
}

void GraphicsWindowWin32::init()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::init() Please implement me!"<<std::endl;
}

bool GraphicsWindowWin32::realizeImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::realizeImplementation() Please implement me!"<<std::endl;
    return false;
}

void GraphicsWindowWin32::makeCurrentImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::makeCurrentImplementation() Please implement me!"<<std::endl;
}

void GraphicsWindowWin32::closeImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::closeImplementation() Please implement me!"<<std::endl;
}

void GraphicsWindowWin32::swapBuffersImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::swapBuffersImplementation() Please implement me!"<<std::endl;
}

void GraphicsWindowWin32::checkEvents()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::checkEvents() Please implement me!"<<std::endl;
}

void GraphicsWindowWin32::grabFocus()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::grabFocus() Please implement me!"<<std::endl;
}

void GraphicsWindowWin32::grabFocusIfPointerInWindow()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowWin32::grabFocusIfPointerInWindow() Please implement me!"<<std::endl;
}


void GraphicsWindowWin32::transformMouseXY(float& x, float& y)
{
    if (getEventQueue()->getUseFixedMouseInputRange())
    {
        osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState();
        x = eventState->getXmin() + (eventState->getXmax()-eventState->getXmin())*x/float(_traits->width);
        y = eventState->getYmin() + (eventState->getYmax()-eventState->getYmin())*y/float(_traits->height);
    }
}

struct Win32WindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface
{

    Win32WindowingSystemInterface()
    {
        
    }

    virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) 
    {
        osg::notify(osg::NOTICE)<<"Win32WindowingSystemInterface::getNumScreens() Please implement me!"<<std::endl;
        return 1;
    }

    virtual void getScreenResolution(const osg::GraphicsContext::ScreenIdentifier& si, unsigned int& width, unsigned int& height)
    {
        osg::notify(osg::NOTICE)<<"Win32WindowingSystemInterface::getScreenResolution() Please implement me!"<<std::endl;
    }

    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
    {
        if (traits->pbuffer)
        {
            osg::ref_ptr<osgViewer::GraphicsContextWin32> pbuffer = new GraphicsContextWin32(traits);
            if (pbuffer->valid()) return pbuffer.release();
            else return 0;
        }
        else
        {
            osg::ref_ptr<osgViewer::GraphicsWindowWin32> window = new GraphicsWindowWin32(traits);
            if (window->valid()) return window.release();
            else return 0;
        }
    }

};

struct RegisterWindowingSystemInterfaceProxy
{
    RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(new Win32WindowingSystemInterface);
    }

    ~RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(0);
    }
};

RegisterWindowingSystemInterfaceProxy createWindowingSystemInterfaceProxy;
