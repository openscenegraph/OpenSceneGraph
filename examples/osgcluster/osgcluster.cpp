#ifdef USE_MEM_CHECK
#include <mcheck.h>
#endif

#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>

#include <osg/Quat>

#if defined (WIN32) && !defined(__CYGWIN__)
#include <winsock.h>
#endif

#include "receiver.h"
#include "broadcaster.h"

typedef unsigned char * BytePtr;
template <class T>
inline void swapBytes(  T &s )
{
    if( sizeof( T ) == 1 ) return;

    T d = s;
    BytePtr sptr = (BytePtr)&s;
    BytePtr dptr = &(((BytePtr)&d)[sizeof(T)-1]); 

    for( unsigned int i = 0; i < sizeof(T); i++ )
        *(sptr++) = *(dptr--);
}

class PackedEvent
{
    public:
        PackedEvent():
            _eventType(osgProducer::EventAdapter::NONE),
            _key(0),
            _button(0),
            _Xmin(0),_Xmax(0),
            _Ymin(0),_Ymax(0),
            _mx(0),
            _my(0),
            _buttonMask(0),
            _modKeyMask(0),
            _time(0.0) {}


        void set(const osgProducer::EventAdapter& event)
        {
            _eventType = event._eventType;
            _key = event._key;
            _button = event._button;
            _Xmin = event._Xmin;
            _Xmax = event._Xmax;
            _Ymin = event._Ymin;
            _Ymax = event._Ymax;
            _mx = event._mx;
            _my = event._my;
            _buttonMask = event._buttonMask;
            _modKeyMask = event._modKeyMask;
            _time = event._time;
        }

        void get(osgProducer::EventAdapter& event)
        {
            event._eventType = _eventType;
            event._key = _key;
            event._button = _button;
            event._Xmin = _Xmin;
            event._Xmax = _Xmax;
            event._Ymin = _Ymin;
            event._Ymax = _Ymax;
            event._mx = _mx;
            event._my = _my;
            event._buttonMask = _buttonMask;
            event._modKeyMask = _modKeyMask;
            event._time = _time;
        }
        
        void swapBytes()
        {
        }

    protected:
    
        osgProducer::EventAdapter::EventType _eventType;
        int _key;
        int _button;
        float _Xmin,_Xmax;
        float _Ymin,_Ymax;
        float _mx;
        float _my;
        unsigned int _buttonMask;
        unsigned int _modKeyMask;
        double _time;
};

const unsigned int MAX_NUM_EVENTS = 10;

class CameraPacket {
    public:
    
    
        CameraPacket():_masterKilled(false) 
        {
            _byte_order = 0x12345678;
        }
        
        void setPacket(const osg::Matrix& matrix,const osg::FrameStamp* frameStamp)
        {
            _matrix = matrix;
            if (frameStamp)
            {
                _frameStamp    = *frameStamp;
            }
        }
        
        void getModelView(osg::Matrix& matrix,float angle_offset=0.0f)
        {
        
            matrix = _matrix * osg::Matrix::rotate(osg::DegreesToRadians(angle_offset),0.0f,1.0f,0.0f);
        }
        
        void readEventQueue(osgProducer::Viewer& viewer);
        
        void writeEventQueue(osgProducer::Viewer& viewer);
        
        void checkByteOrder( void )
        {
            if( _byte_order == 0x78563412 )  // We're backwards
            {
                swapBytes( _byte_order );
                swapBytes( _masterKilled );
                for( int i = 0; i < 16; i++ )
                swapBytes( _matrix.ptr()[i] );

                    // umm.. we should byte swap _frameStamp too...
                    
                    
                for(unsigned int ei=0; ei<_numEvents; ++ei)
                {
                    _events[ei].swapBytes();
                }
            }
        }

        
        void setMasterKilled(const bool flag) { _masterKilled = flag; }
        const bool getMasterKilled() const { return _masterKilled; }
        
        unsigned int    _byte_order;
        bool            _masterKilled;
        osg::Matrix     _matrix;

        // note don't use a ref_ptr as used elsewhere for FrameStamp
        // since we don't want to copy the pointer - but the memory.
        // FrameStamp doesn't have a private destructor to allow
        // us to do this, even though its a reference counted object.    
        osg::FrameStamp  _frameStamp;
        
        unsigned int _numEvents;
        PackedEvent  _events[MAX_NUM_EVENTS];        
        
};

void CameraPacket::readEventQueue(osgProducer::Viewer& viewer)
{
    osgProducer::KeyboardMouseCallback::EventQueue queue;
    viewer.getKeyboardMouseCallback()->copyEventQueue(queue);

    _numEvents = 0;
    for(osgProducer::KeyboardMouseCallback::EventQueue::iterator itr =queue.begin();
        itr != queue.end() && _numEvents<MAX_NUM_EVENTS;
        ++itr, ++_numEvents)
    {
        osgProducer::EventAdapter* event = itr->get();
        _events[_numEvents].set(*event);
    }
    osg::notify(osg::INFO)<<"written events = "<<_numEvents<<std::endl;
}

void CameraPacket::writeEventQueue(osgProducer::Viewer& viewer)
{
    osg::notify(osg::INFO)<<"recieved events = "<<_numEvents<<std::endl;

    // copy the packed events to osgProducer style events.
    osgProducer::KeyboardMouseCallback::EventQueue queue;
    for(unsigned int ei=0; ei<_numEvents; ++ei)
    {
        osgProducer::EventAdapter* event = new osgProducer::EventAdapter;
        _events[ei].get(*event);
        queue.push_back(event);
    }
    
    // pass them to the viewer.
    viewer.getKeyboardMouseCallback()->appendEventQueue(queue);
}



enum ViewerMode
{
    STAND_ALONE,
    SLAVE,
    MASTER
};

int main( int argc, char **argv )
{
    osg::notify(osg::INFO)<<"FrameStamp "<<sizeof(osg::FrameStamp)<<std::endl;
    osg::notify(osg::INFO)<<"osg::Matrix "<<sizeof(osg::Matrix)<<std::endl;
    osg::notify(osg::INFO)<<"PackedEvent "<<sizeof(PackedEvent)<<std::endl;


    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates how to approach implementation of clustering. Note, cluster support will soon be encompassed in Producer itself.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-m","Set viewer to MASTER mode, sending view via packets.");
    arguments.getApplicationUsage()->addCommandLineOption("-s","Set viewer to SLAVE mode, reciving view via packets.");
    arguments.getApplicationUsage()->addCommandLineOption("-n <int>","Socket number to transmit packets");
    arguments.getApplicationUsage()->addCommandLineOption("-f <float>","Field of view of camera");
    arguments.getApplicationUsage()->addCommandLineOption("-o <float>","Offset angle of camera");
    
    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());


    // read up the osgcluster specific arguments.
    ViewerMode viewerMode = STAND_ALONE;
    while (arguments.read("-m")) viewerMode = MASTER;
    while (arguments.read("-s")) viewerMode = SLAVE;
    
    int socketNumber=8100;
    while (arguments.read("-n",socketNumber)) ;

    float camera_fov=-1.0f;
    while (arguments.read("-f",camera_fov)) 
    {
    }

    float camera_offset=45.0f;
    while (arguments.read("-o",camera_offset)) ;


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }
    
    
    // load model.
    osg::ref_ptr<osg::Node> rootnode = osgDB::readNodeFiles(arguments);

    
    // set the scene to render
    viewer.setSceneData(rootnode.get());

    // create the windows and run the threads.
    viewer.realize();


    // set up the lens after realize as the Producer lens is not set up properly before this.... will need to inveestigate this at a later date.
    if (camera_fov>0.0f)
    {
        float aspectRatio = tan( osg::DegreesToRadians(viewer.getLensVerticalFov()*0.5)) / tan(osg::DegreesToRadians(viewer.getLensHorizontalFov()*0.5));
        float new_fovy = osg::RadiansToDegrees(atan( aspectRatio * tan( osg::DegreesToRadians(camera_fov*0.5))))*2.0f;
        std::cout << "setting lens perspective : original "<<viewer.getLensHorizontalFov()<<"  "<<viewer.getLensVerticalFov()<<std::endl;
        viewer.setLensPerspective(camera_fov,new_fovy,1.0f,1000.0f);
        std::cout << "setting lens perspective : new "<<viewer.getLensHorizontalFov()<<"  "<<viewer.getLensVerticalFov()<<std::endl;
    }


    CameraPacket *cp = new CameraPacket;
    // objects for managing the broadcasting and recieving of camera packets.
    Broadcaster     bc;
    Receiver        rc;

    bc.setPort(static_cast<short int>(socketNumber));
    rc.setPort(static_cast<short int>(socketNumber));

    bool masterKilled = false;

    while( !viewer.done() && !masterKilled )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        osg::Timer_t startTick = osg::Timer::instance()->tick();
                 
        // special handling for working as a cluster.
        switch (viewerMode)
        {
        case(MASTER):
            {
                
                // take camera zero as the guide.
                osg::Matrix modelview(viewer.getCameraConfig()->getCamera(0)->getViewMatrix());
                
                cp->setPacket(modelview,viewer.getFrameStamp());
                
                cp->readEventQueue(viewer);

                bc.setBuffer(cp, sizeof( CameraPacket ));
                
                std::cout << "bc.sync()"<<sizeof( CameraPacket )<<std::endl;

                bc.sync();
                
            }
            break;
        case(SLAVE):
            {

                rc.setBuffer(cp, sizeof( CameraPacket ));

                osg::notify(osg::INFO) << "rc.sync()"<<sizeof( CameraPacket )<<std::endl;

                rc.sync();
    
                cp->checkByteOrder();

                cp->writeEventQueue(viewer);

                if (cp->getMasterKilled()) 
                {
                    std::cout << "Received master killed."<<std::endl;
                    // break out of while (!done) loop since we've now want to shut down.
                    masterKilled = true;
                }
            }
            break;
        default:
            // no need to anything here, just a normal interactive viewer.
            break;
        }
         
        osg::Timer_t endTick = osg::Timer::instance()->tick();
        
        osg::notify(osg::INFO)<<"Time to do cluster sync "<<osg::Timer::instance()->delta_m(startTick,endTick)<<std::endl;

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();

        if (viewerMode==SLAVE)
        {
            osg::Matrix modelview;
            cp->getModelView(modelview,camera_offset);
        
            viewer.setView(modelview);
        }

        // fire off the cull and draw traversals of the scene.
        if(!masterKilled)
            viewer.frame();
        
    }

    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    // if we are master clean up by telling all slaves that we're going down.
    if (viewerMode==MASTER)
    {
        // need to broadcast my death.
        cp->setPacket(osg::Matrix::identity(),viewer.getFrameStamp());
        cp->setMasterKilled(true);

        bc.setBuffer(cp, sizeof( CameraPacket ));
        bc.sync();

        std::cout << "Broadcasting death."<<std::endl;

    }

    return 0;
}
