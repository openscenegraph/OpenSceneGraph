#ifdef USE_MEM_CHECK
#include <mcheck.h>
#endif

#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgProducer/Viewer>

#include <osg/Quat>

#if defined (WIN32)
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
        
            matrix = _matrix * osg::Matrix::rotate(angle_offset,0.0f,1.0f,1.0f);
        }
        
	void checkByteOrder( void )
	{
	    if( _byte_order == 0x78563412 )  // We're backwards
	    {
	        swapBytes( _byte_order );
		swapBytes( _masterKilled );
		for( int i = 0; i < 16; i++ )
		    swapBytes( _matrix.ptr()[i] );
                    
                // umm.. we should byte swap _frameStamp too...
	    }
	}

        
        void setMasterKilled(const bool flag) { _masterKilled = flag; }
        const bool getMasterKilled() const { return _masterKilled; }
        
	unsigned long   _byte_order;
        bool            _masterKilled;
        osg::Matrix     _matrix;

        // note don't use a ref_ptr as used elsewhere for FrameStamp
        // since we don't want to copy the pointer - but the memory.
        // FrameStamp doesn't have a private destructor to allow
        // us to do this, even though its a reference counted object.    
        osg::FrameStamp  _frameStamp;
        
};

enum ViewerMode
{
    STAND_ALONE,
    SLAVE,
    MASTER
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
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
    
    float socketNumber=8100.0f;
    while (arguments.read("-n",socketNumber)) ;

    float camera_fov=45.0f;
    while (arguments.read("-f",camera_fov)) ;

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
    viewer.realize(Producer::CameraGroup::ThreadPerCamera);

    // objects for managing the broadcasting and recieving of camera packets.
    Broadcaster     bc;
    Receiver        rc;

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
         
        // special handling for working as a cluster.
        switch (viewerMode)
        {
        case(MASTER):
            {
                CameraPacket cp;
                
                // take camera zero as the guide.
                osg::Matrix modelview(viewer.getCameraConfig()->getCamera(0)->getViewMatrix());
                
                cp.setPacket(modelview,viewer.getFrameStamp());

                bc.setBuffer(&cp, sizeof( CameraPacket ));
	        bc.sync();

            }
            break;
        case(SLAVE):
            {
                CameraPacket cp;

                rc.setBuffer(&cp, sizeof( CameraPacket ));
	        rc.sync();

		cp.checkByteOrder();

                osg::Matrix modelview;
                cp.getModelView(modelview,camera_offset);
                
                viewer.setView(modelview);

                if (cp.getMasterKilled()) 
                {
                    std::cout << "recieved master killed"<<std::endl;
                    // break out of while (!done) loop since we've now want to shut down.
                    break;
                }
            }
            break;
        default:
            // no need to anything here, just a normal interactive viewer.
            break;
        }
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    // if we are master clean up by telling all slaves that we're going down.
    if (viewerMode==MASTER)
    {
        // need to broadcast my death.
        CameraPacket cp;
        cp.setPacket(osg::Matrix::identity(),viewer.getFrameStamp());
        cp.setMasterKilled(true);

        bc.setBuffer(&cp, sizeof( CameraPacket ));
	bc.sync();

        std::cout << "broadcasting death"<<std::endl;

    }

    return 0;
}
