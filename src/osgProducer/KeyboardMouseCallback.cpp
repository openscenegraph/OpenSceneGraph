#include <osgProducer/KeyboardMouseCallback>

#include <osg/Math>
#include <osg/Notify>

#include <float.h>

using namespace osgProducer;

void KeyboardMouseCallback::mouseScroll( Producer::KeyboardMouseCallback::ScrollingMotion sm )
{
    osg::ref_ptr<EventAdapter> event = createEventAdapter();

    _eventQueueMutex.lock();
    event->adaptMouseScroll(getTime(), sm);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::buttonPress( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton |= (1<<(mbutton-1));
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptButtonPress(getTime(),mx,my,mbutton);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::buttonRelease( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton &= ~(1<<(mbutton-1));
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptButtonRelease(getTime(),mx,my,mbutton);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::doubleButtonPress( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton |= (1<<(mbutton-1));
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptButtonPress(getTime(),mx,my,mbutton);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::keyPress( Producer::KeyCharacter key )
{
    osg::ref_ptr<EventAdapter> event = createEventAdapter();

    _eventQueueMutex.lock();
    event->adaptKeyPress(getTime(),key);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

    // check against adapted key symbol.    
    if (_escapeKeySetsDone && 
        event->getKey()==osgGA::GUIEventAdapter::KEY_Escape) _done = true;
}

void KeyboardMouseCallback::keyRelease( Producer::KeyCharacter key )
{
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptKeyRelease(getTime(),key);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::specialKeyPress( Producer::KeyCharacter key )
{
    osg::ref_ptr<EventAdapter> event = createEventAdapter();

    _eventQueueMutex.lock();
    event->adaptKeyPress(getTime(),key);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

    // check against adapted key symbol.    
    if (_escapeKeySetsDone && 
        event->getKey()==osgGA::GUIEventAdapter::KEY_Escape) _done = true;
}

void KeyboardMouseCallback::specialKeyRelease( Producer::KeyCharacter key )
{
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptKeyRelease(getTime(),key);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::windowConfig( int x, int y, unsigned int width, unsigned int height )
{
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptResize(getTime(), x, y, x+width, y+height );
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::mouseMotion( float mx, float my) 
{
    _mx = mx;
    _my = my;
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptMouseMotion(getTime(),mx,my);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

}

void KeyboardMouseCallback::passiveMouseMotion( float mx, float my) 
{
    _mx = mx;
    _my = my;
    
    //std::cout << "mx="<<mx<<" my="<<my<<std::endl;

    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    event->adaptMouseMotion(getTime(),mx,my);
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

}

void KeyboardMouseCallback::mouseWarp( float mx, float my) 
{
    _mx = mx;
    _my = my;
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

}

double KeyboardMouseCallback::getEventQueue(EventQueue& queue)
{
    double swapTime;

    queue.clear();
    _eventQueueMutex.lock();
    _eventQueue.swap(queue);
    swapTime = getTime();
    _eventQueueMutex.unlock();
    
    return swapTime;
}

EventAdapter* KeyboardMouseCallback::createEventAdapter()
{
    EventAdapter* ea = new EventAdapter;

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

        ea->setWindowSize(minX,minY,maxX,maxY);
    }
    
    return ea;
}

void KeyboardMouseCallback::shutdown()
{
    _done = true;
    _keyboardMouse->cancel();
}
