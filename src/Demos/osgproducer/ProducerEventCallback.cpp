#include "ProducerEventCallback.h"

#ifdef WIN32
    #include <windows.h>
#else
    #include <X11/keysym.h>
#endif


void ProducerEventCallback::keyPress( Producer::KeySymbol key )
{

    switch( key )
    {
#ifdef XK_MISCELLANY // XWindows keydefs
        case XK_Escape:
            _done = true;
            break;
#endif
#ifdef WIN32
         case VK_ESCAPE:
            _done = true;
            break;
#endif
    }
    
    osg::ref_ptr<ProducerEventAdapter> event = new ProducerEventAdapter;
    event->adaptKeyPress(getTime(),key);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void ProducerEventCallback::keyRelease( Producer::KeySymbol key )
{

    osg::ref_ptr<ProducerEventAdapter> event = new ProducerEventAdapter;
    event->adaptKeyRelease(getTime(),key);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void ProducerEventCallback::mouseMotion( float mx, float my) 
{
    _mx = mx;
    _my = my;
    
    
    osg::ref_ptr<ProducerEventAdapter> event = new ProducerEventAdapter;
    event->adaptMouseMotion(getTime(),mx,my);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

}

void ProducerEventCallback::buttonPress( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton |= (1<<(mbutton-1));
    
    
    osg::ref_ptr<ProducerEventAdapter> event = new ProducerEventAdapter;
    event->adaptButtonPress(getTime(),mx,my,mbutton);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void ProducerEventCallback::buttonRelease( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton &= ~(1<<(mbutton-1));
    
    
    osg::ref_ptr<ProducerEventAdapter> event = new ProducerEventAdapter;
    event->adaptButtonRelease(getTime(),mx,my,mbutton);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}
