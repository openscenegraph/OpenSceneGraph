#ifdef USE_MEM_CHECK
#include <mcheck.h>
#endif

#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

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
        
        void setPacket(const osg::Camera& camera,const osg::FrameStamp* frameStamp)
        {
            _eye    = camera.getEyePoint();
            _center = camera.getCenterPoint();
            _up     = camera.getUpVector();
            if (frameStamp)
            {
                _frameStamp    = *frameStamp;
            }
        }
        
        void getCamera(osg::Camera& camera,float angle_offset=0.0f)
        {
        
            osg::Vec3 lv = _center-_eye;
            osg::Matrix matrix;
            matrix.makeIdentity();
            matrix.makeRotate(angle_offset,_up.x(),_up.y(),_up.z());
            lv = lv*matrix;
        
            camera.setLookAt(_eye,_eye+lv,_up);
                        
        }
        
        void getSceneViewUpdate(osgUtil::SceneView& sv)
        {
            // note pass a seperate reference counted FrameStamp
            // rather than this frame stamp as it can get overwritten.
            sv.setFrameStamp(new osg::FrameStamp(_frameStamp));
        }


	void checkByteOrder( void )
	{
	    if( _byte_order == 0x78563412 )  // We're backwards
	    {
	        swapBytes( _byte_order );
		swapBytes( _masterKilled );
		swapBytes( _eye[0] );
		swapBytes( _eye[1] );
		swapBytes( _eye[2] );
		swapBytes( _center[0] );
		swapBytes( _center[1] );
		swapBytes( _center[2] );
		swapBytes( _up[0] );
		swapBytes( _up[1] );
		swapBytes( _up[2] );
		swapBytes( _attachMatrix );
		for( int i = 0; i < 16; i++ )
		    swapBytes( _matrix.ptr()[i] );
	    }
	}

        
        void setMasterKilled(const bool flag) { _masterKilled = flag; }
        const bool getMasterKilled() const { return _masterKilled; }
        
	unsigned long   _byte_order;
        bool            _masterKilled;
        osg::Vec3       _eye;
        osg::Vec3       _center;
        osg::Vec3       _up;
        bool            _attachMatrix;
        osg::Matrix     _matrix;

        // note don't use a ref_ptr as used elsewhere for FrameStamp
        // since we don't want to copy the pointer - but the memory.
        // FrameStamp doesn't have a private destructor to allow
        // us to do this, even though its a reference counted object.    
        osg::FrameStamp  _frameStamp;
        
};


class MySceneView : public osgUtil::SceneView {

    public:
    
        enum ViewerMode
        {
            STAND_ALONE,
            SLAVE,
            MASTER
        };
    
        MySceneView(ViewerMode viewerMode,int socketNumber,float camera_fov, float camera_offset):
            _viewerMode(viewerMode),_socketNumber(socketNumber),
            _camera_fov(camera_fov), _camera_offset(camera_offset)
        {
            setDefaults();
            getCamera()->setAdjustAspectRatioMode(osg::Camera::ADJUST_VERTICAL);
            getCamera()->setFOV(camera_fov,camera_fov*(600.0f/800.0f),1.0f,1000.0f);
            
            _bc.setPort(socketNumber);
            _rc.setPort(socketNumber);
        };
        
        ~MySceneView()
        {
            if (_viewerMode==MASTER)
            {
                // need to broadcast my death.
                CameraPacket cp;
                cp.setPacket(*getCamera(),getFrameStamp());
                cp.setMasterKilled(true);
                
                _bc.setBuffer(&cp, sizeof( CameraPacket ));
	        _bc.sync();
                
                std::cout << "broadcasting death"<<std::endl;
                
            }
        }
        
        // override the basic SceneView::app traversal.
        virtual void app()
        {
            osgUtil::SceneView::app();
            switch (_viewerMode)
            {
            case(MASTER):
                {
                    CameraPacket cp;
                    cp.setPacket(*getCamera(),getFrameStamp());

                    _bc.setBuffer(&cp, sizeof( CameraPacket ));
	            _bc.sync();

                }
                break;
            case(SLAVE):
                {
                    CameraPacket cp;

                    _rc.setBuffer(&cp, sizeof( CameraPacket ));
	            _rc.sync();

		    cp.checkByteOrder();


                    cp.getCamera(*getCamera(),_camera_offset);
                    cp.getSceneViewUpdate(*this);
                    
                    if (cp.getMasterKilled()) 
                    {
                        std::cout << "recieved master killed"<<std::endl;
                        _viewerMode = STAND_ALONE;
                    }
                }
                break;
            default:
                // no need to anything here, just a normal interactive viewer.
                break;
            }
        }
        
    protected:
    
        ViewerMode      _viewerMode;
        int             _socketNumber;
        float           _camera_fov;
        float           _camera_offset;
	unsigned long   _byte_order;
        

        Broadcaster     _bc;
        Receiver        _rc;


};

/*
 * Function to read several files (typically one) as specified on the command
 * line, and return them in an osg::Node
 */
osg::Node* getNodeFromFiles(int argc,char **argv, 
                            MySceneView::ViewerMode& viewerMode, int& socketNumber,
                            float& camera_fov, float& camera_offset)
{
    osg::Node *rootnode = new osg::Node;

    int i;

    typedef std::vector<osg::Node*> NodeList;
    NodeList nodeList;
    for( i = 1; i < argc; i++ )
    {

        if (argv[i][0]=='-')
        {
            switch(argv[i][1])
            {

                case('m'):
                    viewerMode = MySceneView::MASTER;
                    break;
                case('s'):
                    viewerMode = MySceneView::SLAVE;
                    break;
                case('n'):
                    ++i;
                    if (i<argc)
                    {
                        socketNumber = atoi(argv[i]);
                    }
                    break;
                case('f'):
                    ++i;
                    if (i<argc)
                    {
                        camera_fov = atoi(argv[i]);
                    }
                    break;
                case('o'):
                    ++i;
                    if (i<argc)
                    {
                        camera_offset = atoi(argv[i]);
                    }
                    break;
                    
                case('l'):
                    ++i;
                    if (i<argc)
                    {
                        osgDB::Registry::instance()->loadLibrary(argv[i]);
                    }
                    break;
                case('e'):
                    ++i;
                    if (i<argc)
                    {
                        std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(argv[i]);
                        osgDB::Registry::instance()->loadLibrary(libName);
                    }
                    break;
            }
        } else
        {
            osg::Node *node = osgDB::readNodeFile( argv[i] );

            if( node != (osg::Node *)0L )
            {
                if (node->getName().empty()) node->setName( argv[i] );
                nodeList.push_back(node);
            }
        }

    }

    if (nodeList.size()==0)
    {
        osg::notify(osg::WARN) << "No data loaded."<<std::endl;
        exit(0);
    }
    
    
/*
    if (master) osg::notify(osg::NOTICE)<<"set to MASTER, broadcasting on socketNumber "<<socketNumber<<std::endl;
    else osg::notify(osg::NOTICE)<<"set to SLAVE, reciving on socketNumber "<<socketNumber<<std::endl;
    
*/
    

    if (nodeList.size()==1)
    {
        rootnode = nodeList.front();
    }
    else                         // size >1
    {
        osg::Group* group = new osg::Group();
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        rootnode = group;
    }

    return rootnode;
}


int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        osg::notify(osg::NOTICE)<<"usage:"<<std::endl;
        osg::notify(osg::NOTICE)<<"    osgcluster [options] infile1 [infile2 ...]"<<std::endl;
        osg::notify(osg::NOTICE)<<std::endl;
        osg::notify(osg::NOTICE)<<"options:"<<std::endl;
        osg::notify(osg::NOTICE)<<"    -m                 - set this viewer to be master"<<std::endl;
        osg::notify(osg::NOTICE)<<"    -s                 - set this viewer to be a slave"<<std::endl;
        osg::notify(osg::NOTICE)<<"    -o                 - offset the slave camera from the master position"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         by specified number of degress. A positive offset "<<std::endl;
        osg::notify(osg::NOTICE)<<"                         turns camera towards right."<<std::endl;
        osg::notify(osg::NOTICE)<<"    -f                 - set the horizontal field of view of the camera."<<std::endl;
        osg::notify(osg::NOTICE)<<"    -n SocketNumber    - set the socket number, defaults to 8100."<<std::endl;
        osg::notify(osg::NOTICE)<<"                         to broadcast on if a master"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         to reciever on if a slave"<<std::endl;
        osg::notify(osg::NOTICE)<<std::endl;
        osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<<std::endl;
        osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<<std::endl;
        osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<<std::endl;
        osg::notify(osg::NOTICE)<<std::endl;

        return 0;
    }

    osg::Timer timer;
    osg::Timer_t before_load = timer.tick();
    
    MySceneView::ViewerMode viewerMode = MySceneView::STAND_ALONE;
    int socketNumber=8100;
    float camera_fov=45.0f;
    float camera_offset=45.0f;

    osg::Node* rootnode = getNodeFromFiles( argc, argv, viewerMode, socketNumber,camera_fov,camera_offset);
    
    osg::Timer_t after_load = timer.tick();
    std::cout << "Time for load = "<<timer.delta_s(before_load,after_load)<<" seconds"<<std::endl;

    osg::ref_ptr<MySceneView> mySceneView = new MySceneView(viewerMode,socketNumber,camera_fov,osg::inDegrees(camera_offset));
    mySceneView->setSceneData(rootnode);

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);
    viewer.addViewport( mySceneView.get() );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    viewer.open();
    viewer.run();

    return 0;
}
