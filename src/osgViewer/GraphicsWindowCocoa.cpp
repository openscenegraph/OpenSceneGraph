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

#include <osgViewer/GraphicsWindowCocoa>


using namespace osgViewer;

namespace osgViewer
{

/** This is the class we need to create for pbuffers, note its not a GraphicsWindow as it won't need any of the event handling and window mapping facilities.*/
class GraphicsContextCocoa : public osg::GraphicsContext
{
    public:

        GraphicsContextCocoa(osg::GraphicsContext::Traits* traits):
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
        virtual bool makeCurrentImplementation()  { osg::notify(osg::NOTICE)<<"GraphicsWindow::makeCurrentImplementation() not implemented."<<std::endl; return false;}
        
        /** Make this graphics context current with specified read context implementation.
          * Pure virtual - must be implemented by concrate implementations of GraphicsContext. */
        virtual bool makeContextCurrentImplementation(GraphicsContext* /*readContext*/)  { osg::notify(osg::NOTICE)<<"GraphicsWindow::makeContextCurrentImplementation(..) not implemented."<<std::endl; return false; }

        /** Release the graphics context.*/
        virtual bool releaseContextImplementation() {  osg::notify(osg::NOTICE)<<"GraphicsWindow::releaseContextImplementation(..) not implemented."<<std::endl; return false; }

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

void GraphicsWindowCocoa::setWindowDecoration(bool flag)
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::setWindowDecoration() Please implement me!"<<std::endl;
}

void GraphicsWindowCocoa::init()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::init() Please implement me!"<<std::endl;
}

bool GraphicsWindowCocoa::realizeImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::realizeImplementation() Please implement me!"<<std::endl;
    return false;
}

bool GraphicsWindowCocoa::makeCurrentImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::makeCurrentImplementation() Please implement me!"<<std::endl;
    return false;
}

bool GraphicsWindowCocoa::releaseContextImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::releaseContextImplementation() Please implement me!"<<std::endl;
    return false;
}

void GraphicsWindowCocoa::closeImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::closeImplementation() Please implement me!"<<std::endl;
}

void GraphicsWindowCocoa::swapBuffersImplementation()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::swapBuffersImplementation() Please implement me!"<<std::endl;
}

void GraphicsWindowCocoa::checkEvents()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::checkEvents() Please implement me!"<<std::endl;
}

void GraphicsWindowCocoa::grabFocus()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::grabFocus() Please implement me!"<<std::endl;
}

void GraphicsWindowCocoa::grabFocusIfPointerInWindow()
{
    osg::notify(osg::NOTICE)<<"GraphicWindowCocoa::grabFocusIfPointerInWindow() Please implement me!"<<std::endl;
}


void GraphicsWindowCocoa::transformMouseXY(float& x, float& y)
{
    if (getEventQueue()->getUseFixedMouseInputRange())
    {
        osgGA::GUIEventAdapter* eventState = getEventQueue()->getCurrentEventState();
        x = eventState->getXmin() + (eventState->getXmax()-eventState->getXmin())*x/float(_traits->width);
        y = eventState->getYmin() + (eventState->getYmax()-eventState->getYmin())*y/float(_traits->height);
    }
}

struct CocoaWindowingSystemInterface : public osg::GraphicsContext::WindowingSystemInterface
{

    CocoaWindowingSystemInterface()
    {
        
    }

    virtual unsigned int getNumScreens(const osg::GraphicsContext::ScreenIdentifier& si) 
    {
        osg::notify(osg::NOTICE)<<"CocoaWindowingSystemInterface::getNumScreens() Please implement me!"<<std::endl;
        return 1;
    }

    virtual void getScreenResolution(const osg::GraphicsContext::ScreenIdentifier& si, unsigned int& width, unsigned int& height)
    {
        osg::notify(osg::NOTICE)<<"CocoaWindowingSystemInterface::getScreenResolution() Please implement me!"<<std::endl;
    }

    virtual osg::GraphicsContext* createGraphicsContext(osg::GraphicsContext::Traits* traits)
    {
        if (traits->pbuffer)
        {
            osg::ref_ptr<osgViewer::GraphicsContextCocoa> pbuffer = new GraphicsContextCocoa(traits);
            if (pbuffer->valid()) return pbuffer.release();
            else return 0;
        }
        else
        {
            osg::ref_ptr<osgViewer::GraphicsWindowCocoa> window = new GraphicsWindowCocoa(traits);
            if (window->valid()) return window.release();
            else return 0;
        }
    }

};

struct RegisterWindowingSystemInterfaceProxy
{
    RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(new CocoaWindowingSystemInterface);
    }

    ~RegisterWindowingSystemInterfaceProxy()
    {
        osg::GraphicsContext::setWindowingSystemInterface(0);
    }
};

RegisterWindowingSystemInterfaceProxy createWindowingSystemInterfaceProxy;
