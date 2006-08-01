#include <osgProducer/KeyboardMouseCallback>

#include <osg/Math>
#include <osg/Notify>

#include <float.h>

using namespace osgProducer;

KeyboardMouseCallback::KeyboardMouseCallback(Producer::KeyboardMouse* keyboardMouse, bool &done, bool escapeKeySetsDone):
    Producer::KeyboardMouseCallback(),
    _keyboardMouse(keyboardMouse),
    _mx(0.0f),_my(0.0f),_mbutton(0),
    _done(done),
    _escapeKeySetsDone(escapeKeySetsDone)            
{
    updateWindowSize();
}

void KeyboardMouseCallback::mouseScroll( Producer::KeyboardMouseCallback::ScrollingMotion sm )
{
    updateWindowSize();
    if (_eventQueue.valid()) 
    {
        switch(sm)
        {
            case(Producer::KeyboardMouseCallback::ScrollNone): break;
            case(Producer::KeyboardMouseCallback::ScrollLeft): _eventQueue->mouseScroll(osgGA::GUIEventAdapter::SCROLL_LEFT); break;
            case(Producer::KeyboardMouseCallback::ScrollRight): _eventQueue->mouseScroll(osgGA::GUIEventAdapter::SCROLL_RIGHT); break;
            case(Producer::KeyboardMouseCallback::ScrollUp): _eventQueue->mouseScroll(osgGA::GUIEventAdapter::SCROLL_UP); break;
            case(Producer::KeyboardMouseCallback::ScrollDown): _eventQueue->mouseScroll(osgGA::GUIEventAdapter::SCROLL_DOWN); break;
            case(Producer::KeyboardMouseCallback::Scroll2D): _eventQueue->mouseScroll(osgGA::GUIEventAdapter::SCROLL_2D); break;
        }
    }
}

void KeyboardMouseCallback::mouseScroll2D( float x, float y )
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->mouseScroll2D(x,y);
}

void KeyboardMouseCallback::penPressure( float pressure ) 
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->penPressure(pressure);
}

void KeyboardMouseCallback::penProximity(Producer::KeyboardMouseCallback::TabletPointerType pt, bool isEntering)
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->penProximity((osgGA::GUIEventAdapter::TabletPointerType)pt, isEntering);
}

void KeyboardMouseCallback::buttonPress( float mx, float my, unsigned int mbutton ) 
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->mouseButtonPress(mx,my,mbutton);
}

void KeyboardMouseCallback::buttonRelease( float mx, float my, unsigned int mbutton ) 
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->mouseButtonRelease(mx,my,mbutton);
}

void KeyboardMouseCallback::doubleButtonPress( float mx, float my, unsigned int mbutton ) 
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->mouseDoubleButtonPress(mx,my,mbutton);
}

void KeyboardMouseCallback::keyPress( Producer::KeyCharacter key )
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->keyPress((osgGA::GUIEventAdapter::KeySymbol)key);

    // check against adapted key symbol.    
    if (_escapeKeySetsDone && 
        (osgGA::GUIEventAdapter::KeySymbol)key==osgGA::GUIEventAdapter::KEY_Escape) _done = true;
}

void KeyboardMouseCallback::keyRelease( Producer::KeyCharacter key )
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->keyRelease((osgGA::GUIEventAdapter::KeySymbol)key);
}

void KeyboardMouseCallback::specialKeyPress( Producer::KeyCharacter key )
{
    updateWindowSize();
    keyPress(key);
}

void KeyboardMouseCallback::specialKeyRelease( Producer::KeyCharacter key )
{
    updateWindowSize();
    keyRelease(key);
}

void KeyboardMouseCallback::windowConfig( int x, int y, unsigned int width, unsigned int height )
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->windowResize(x,y,x+width,y+height);
}

void KeyboardMouseCallback::mouseMotion( float mx, float my) 
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->mouseMotion(mx,my);
}

void KeyboardMouseCallback::passiveMouseMotion( float mx, float my) 
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->mouseMotion(mx,my);
}

void KeyboardMouseCallback::mouseWarp( float mx, float my) 
{
    updateWindowSize();
    if (_eventQueue.valid()) _eventQueue->mouseWarp(mx,my); // need mouse warp??
}


void KeyboardMouseCallback::updateWindowSize()
{
    if (!_eventQueue) return;

    osgGA::GUIEventAdapter* ea = _eventQueue->getCurrentEventState();

    Producer::InputArea* ia = _keyboardMouse->getInputArea();
    Producer::RenderSurface* rs = _keyboardMouse->getRenderSurface();
    if (ia)
    {
    
        float minX = FLT_MAX;
        float minY = FLT_MAX;
        float maxX = -FLT_MAX;
        float maxY = -FLT_MAX;
        //int numInputRectangle = ia->getNumInputRectangle();
        int numRenderSurfaces = ia->getNumRenderSurfaces();
        for (int i=0;i<numRenderSurfaces;++i)
        {
            const Producer::RenderSurface::InputRectangle &ir = 
                ia->getRenderSurface(i)->getInputRectangle();

            minX = osg::minimum(minX,ir.left());
            minX = osg::minimum(minX,ir.left()+ir.width());
            
            minY = osg::minimum(minY,ir.bottom());
            minY = osg::minimum(minY,ir.bottom()+ir.height());

            maxX = osg::maximum(maxX,ir.left());
            maxX = osg::maximum(maxX,ir.left()+ir.width());
            
            maxY = osg::maximum(maxY,ir.bottom());
            maxY = osg::maximum(maxY,ir.bottom()+ir.height());
        }
        
        // osg::notify(osg::NOTICE)<<"IA ea->setWindowSize("<<minX<<","<<minY<<","<<maxX<<","<<maxY<<")"<<std::endl;
        
        ea->setWindowSize(minX,minY,maxX,maxY);
    }
    else if (rs)
    {
        //ea->setWindowSize(-1.0f,-1.0f,1.0f,1.0f);
        
        const Producer::RenderSurface::InputRectangle &ir = rs->getInputRectangle();

        float minX = osg::minimum(ir.left(),ir.left()+ir.width());
        float maxX = osg::maximum(ir.left(),ir.left()+ir.width());
        float minY = osg::minimum(ir.bottom(),ir.bottom()+ir.height());
        float maxY = osg::maximum(ir.bottom(),ir.bottom()+ir.height());

        // osg::notify(osg::NOTICE)<<"RS ea->setWindowSize("<<minX<<","<<minY<<","<<maxX<<","<<maxY<<")"<<std::endl;

        ea->setWindowSize(minX,minY,maxX,maxY);
    }
}

bool KeyboardMouseCallback::takeEventQueue(EventQueue& queue)
{
    updateWindowSize();
    return _eventQueue->takeEvents(queue);
}

bool KeyboardMouseCallback::copyEventQueue(EventQueue& queue) const
{
    return _eventQueue->copyEvents(queue);
}

void KeyboardMouseCallback::setEventQueue(EventQueue& queue)
{
    _eventQueue->setEvents(queue);
}

void KeyboardMouseCallback::appendEventQueue(EventQueue& queue)
{
    _eventQueue->appendEvents(queue);
}

void KeyboardMouseCallback::shutdown()
{
    _done = true;
    _keyboardMouse->cancel();
}

osgGA::GUIEventAdapter* KeyboardMouseCallback::createEventAdapter()
{
    return new osgGA::GUIEventAdapter(*(_eventQueue->getCurrentEventState()));
}
