#include <osg/Math>
#include <osgProducer/KeyboardMouseCallback>

#include <float.h>

using namespace osgProducer;

void KeyboardMouseCallback::buttonPress( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton |= (1<<(mbutton-1));
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptButtonPress(getTime(),mx,my,mbutton);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::buttonRelease( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton &= ~(1<<(mbutton-1));
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptButtonRelease(getTime(),mx,my,mbutton);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::doubleButtonPress( float mx, float my, unsigned int mbutton ) 
{
    _mx = mx;
    _my = my;
    _mbutton |= (1<<(mbutton-1));
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptButtonPress(getTime(),mx,my,mbutton);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::keyPress( Producer::KeyCharacter key )
{

    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptKeyPress(getTime(),key);

    // check against adapted key symbol.    
    if (_escapeKeySetsDone && 
        event->getKey()==osgGA::GUIEventAdapter::KEY_Escape) _done = true;


    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::keyRelease( Producer::KeyCharacter key )
{

    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptKeyRelease(getTime(),key);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::specialKeyPress( Producer::KeyCharacter key )
{

           
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptKeyPress(getTime(),key);

    // check against adapted key symbol.    
    if (_escapeKeySetsDone && 
        event->getKey()==osgGA::GUIEventAdapter::KEY_Escape) _done = true;


    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::specialKeyRelease( Producer::KeyCharacter key )
{

    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptKeyRelease(getTime(),key);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();
}

void KeyboardMouseCallback::mouseMotion( float mx, float my) 
{
    _mx = mx;
    _my = my;
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptMouseMotion(getTime(),mx,my);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

}

void KeyboardMouseCallback::passiveMouseMotion( float mx, float my) 
{
    _mx = mx;
    _my = my;
    
    osg::ref_ptr<EventAdapter> event = createEventAdapter();
    event->adaptMouseMotion(getTime(),mx,my);
    
    _eventQueueMutex.lock();
    _eventQueue.push_back(event);
    _eventQueueMutex.unlock();

}

void KeyboardMouseCallback::getEventQueue(EventQueue& queue)
{
    queue.clear();
    _eventQueueMutex.lock();
    _eventQueue.swap(queue);
    _eventQueueMutex.unlock();
    
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
        int numInputRectangle = ia->getNumInputRectangle();
        for (int i=0;i<numInputRectangle;++i)
        {
            Producer::InputRectangle* ir = ia->getInputRectangle(i);

            minX = osg::minimum(minX,ir->left());
            minX = osg::minimum(minX,ir->left()+ir->width());
            
            minY = osg::minimum(minY,ir->bottom());
            minY = osg::minimum(minY,ir->bottom()+ir->height());

            maxX = osg::maximum(maxX,ir->left());
            maxX = osg::maximum(maxX,ir->left()+ir->width());
            
            maxY = osg::maximum(maxY,ir->bottom());
            maxY = osg::maximum(maxY,ir->bottom()+ir->height());
        }
        ea->setWindowSize(minX,minY,maxX,maxY);
    }
    else if (rs)
    {
        float xMin,yMin,xMax,yMax;
        rs->getWindowRect(xMin,xMax,yMin,yMax);
        ea->setWindowSize(xMin,yMin,xMax,yMax);
    }
    
    return ea;
}
