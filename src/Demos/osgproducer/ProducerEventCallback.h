//C++
#ifndef PRODUCEREVENTCALLBACK
#define PRODUCEREVENTCALLBACK

#include <stdio.h>
#include <Producer/RenderSurface> // For definition of KeySymbol
#include <Producer/KeyboardMouse>
#include <Producer/Mutex>

#include "ProducerEventAdapter.h"
#include <osg/ref_ptr>
#include <osg/Timer>

class ProducerEventCallback : public Producer::KeyboardMouseCallback
{
    public:
        ProducerEventCallback(bool &done) :
            Producer::KeyboardMouseCallback(),
            _mx(0.0f),_my(0.0f),_mbutton(0),
            _done(done)    
            {}

        virtual ~ProducerEventCallback();

        virtual void keyPress( Producer::KeySymbol key );
                
        virtual void keyRelease( Producer::KeySymbol key );

        virtual void mouseMotion( float mx, float my);
                
        virtual void buttonPress( float mx, float my, unsigned int mbutton );
                
        virtual void buttonRelease( float mx, float my, unsigned int mbutton );

        typedef std::vector< osg::ref_ptr<ProducerEventAdapter> > EventQueue;

        void getEventQueue(EventQueue& queue);

        bool done() { return _done; }
        float mx()  { return _mx; }
        float my()  { return _my; }
        unsigned int mbutton()  { return _mbutton; }
        
        double getTime() { return 0.0f; }
        
    private:
    
        float _mx, _my;
        unsigned int _mbutton;
        bool &_done;

        osg::Timer      _timer;        
        Producer::Mutex _eventQueueMutex;
        EventQueue      _eventQueue;
        
};
#endif
