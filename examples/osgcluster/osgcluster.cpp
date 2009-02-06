/* OpenSceneGraph example, osgcluster.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#ifdef USE_MEM_CHECK
#include <mcheck.h>
#endif

#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include <osg/Quat>
#include <osg/io_utils>

#include <iostream>

#if defined (WIN32) && !defined(__CYGWIN__)
#include <winsock.h>
#endif

#include "receiver.h"
#include "broadcaster.h"


const unsigned int MAX_NUM_EVENTS = 10;
const unsigned int SWAP_BYTES_COMPARE = 0x12345678;
class CameraPacket {
    public:
    
    
        CameraPacket():_masterKilled(false) 
        {
            _byte_order = SWAP_BYTES_COMPARE;
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
        
        void readEventQueue(osgViewer::Viewer& viewer);
        
        void writeEventQueue(osgViewer::Viewer& viewer);

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
        
        osgGA::EventQueue::Events _events;
        
};

class DataConverter
{
    public:

        DataConverter(unsigned int numBytes):
            _startPtr(0),
            _endPtr(0),
            _swapBytes(false),
            _currentPtr(0)
        {
            _currentPtr = _startPtr = new char[numBytes];
            _endPtr = _startPtr+numBytes;
            _numBytes = numBytes;
        }

        char* _startPtr;
        char* _endPtr;
        unsigned int _numBytes;
        bool _swapBytes;

        char* _currentPtr;
        
        void reset()
        {
            _currentPtr = _startPtr;
        }

        inline void write1(char* ptr)
        {
            if (_currentPtr+1>=_endPtr) return;

            *(_currentPtr++) = *(ptr); 
        }

        inline void read1(char* ptr)
        {
            if (_currentPtr+1>=_endPtr) return;

            *(ptr) = *(_currentPtr++); 
        }

        inline void write2(char* ptr)
        {
            if (_currentPtr+2>=_endPtr) return;

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr); 
        }

        inline void read2(char* ptr)
        {
            if (_currentPtr+2>=_endPtr) return;

            if (_swapBytes)
            {
                *(ptr+1) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
            else
            {
                *(ptr++) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
        }

        inline void write4(char* ptr)
        {
            if (_currentPtr+4>=_endPtr) return;

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr); 
        }

        inline void read4(char* ptr)
        {
            if (_currentPtr+4>=_endPtr) return;

            if (_swapBytes)
            {
                *(ptr+3) = *(_currentPtr++); 
                *(ptr+2) = *(_currentPtr++); 
                *(ptr+1) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
            else
            {
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
        }

        inline void write8(char* ptr)
        {
            if (_currentPtr+8>=_endPtr) return;

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 

            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr++); 
            *(_currentPtr++) = *(ptr); 
        }

        inline void read8(char* ptr)
        {
            char* endPtr = _currentPtr+8;
            if (endPtr>=_endPtr) return;

            if (_swapBytes)
            {
                *(ptr+7) = *(_currentPtr++); 
                *(ptr+6) = *(_currentPtr++); 
                *(ptr+5) = *(_currentPtr++); 
                *(ptr+4) = *(_currentPtr++); 

                *(ptr+3) = *(_currentPtr++); 
                *(ptr+2) = *(_currentPtr++); 
                *(ptr+1) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
            else
            {
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 

                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr++) = *(_currentPtr++); 
                *(ptr) = *(_currentPtr++); 
            }
        }

        inline void writeChar(char c)               { write1(&c); }
        inline void writeUChar(unsigned char c)     { write1((char*)&c); }
        inline void writeShort(short c)             { write2((char*)&c); }
        inline void writeUShort(unsigned short c)   { write2((char*)&c); }
        inline void writeInt(int c)                 { write4((char*)&c); }
        inline void writeUInt(unsigned int c)       { write4((char*)&c); }
        inline void writeFloat(float c)             { write4((char*)&c); }
        inline void writeDouble(double c)           { write8((char*)&c); }

        inline char readChar() { char c; read1(&c); return c; }
        inline unsigned char readUChar() { unsigned char c; read1((char*)&c); return c; }
        inline short readShort() { short c; read2((char*)&c); return c; }
        inline unsigned short readUShort() { unsigned short c; read2((char*)&c); return c; }
        inline int readInt() { int c; read4((char*)&c); return c; }
        inline unsigned int readUInt() { unsigned int c; read4((char*)&c); return c; }
        inline float readFloat() { float c; read4((char*)&c); return c; }
        inline double readDouble() { double c; read8((char*)&c); return c; }

        void write(const osg::FrameStamp& fs)
        {
            osg::notify(osg::NOTICE)<<"writeFramestamp = "<<fs.getFrameNumber()<<" "<<fs.getReferenceTime()<<std::endl;

            writeUInt(fs.getFrameNumber());
            writeDouble(fs.getReferenceTime());
            writeDouble(fs.getSimulationTime());
        }

        void read(osg::FrameStamp& fs)
        {
            fs.setFrameNumber(readUInt());
            fs.setReferenceTime(readDouble());
            fs.setSimulationTime(readDouble());

            osg::notify(osg::NOTICE)<<"readFramestamp = "<<fs.getFrameNumber()<<" "<<fs.getReferenceTime()<<std::endl;
        }

        void write(const osg::Matrix& matrix)
        {
            writeDouble(matrix(0,0));
            writeDouble(matrix(0,1));
            writeDouble(matrix(0,2));
            writeDouble(matrix(0,3));

            writeDouble(matrix(1,0));
            writeDouble(matrix(1,1));
            writeDouble(matrix(1,2));
            writeDouble(matrix(1,3));

            writeDouble(matrix(2,0));
            writeDouble(matrix(2,1));
            writeDouble(matrix(2,2));
            writeDouble(matrix(2,3));

            writeDouble(matrix(3,0));
            writeDouble(matrix(3,1));
            writeDouble(matrix(3,2));
            writeDouble(matrix(3,3));

            osg::notify(osg::NOTICE)<<"writeMatrix = "<<matrix<<std::endl;

        }

        void read(osg::Matrix& matrix)
        {
            matrix(0,0) = readDouble();
            matrix(0,1) = readDouble();
            matrix(0,2) = readDouble();
            matrix(0,3) = readDouble();

            matrix(1,0) = readDouble();
            matrix(1,1) = readDouble();
            matrix(1,2) = readDouble();
            matrix(1,3) = readDouble();

            matrix(2,0) = readDouble();
            matrix(2,1) = readDouble();
            matrix(2,2) = readDouble();
            matrix(2,3) = readDouble();

            matrix(3,0) = readDouble();
            matrix(3,1) = readDouble();
            matrix(3,2) = readDouble();
            matrix(3,3) = readDouble();

            osg::notify(osg::NOTICE)<<"readMatrix = "<<matrix<<std::endl;

        }

        void write(const osgGA::GUIEventAdapter& event)
        {
            writeUInt(event.getEventType());
            writeUInt(event.getKey());
            writeUInt(event.getButton());
            writeInt(event.getWindowX());
            writeInt(event.getWindowY());
            writeUInt(event.getWindowWidth());
            writeUInt(event.getWindowHeight());
            writeFloat(event.getXmin());
            writeFloat(event.getYmin());
            writeFloat(event.getXmax());
            writeFloat(event.getYmax());
            writeFloat(event.getX());
            writeFloat(event.getY());
            writeUInt(event.getButtonMask());
            writeUInt(event.getModKeyMask());
            writeDouble(event.getTime());
        }

        void read(osgGA::GUIEventAdapter& event)
        {
            event.setEventType((osgGA::GUIEventAdapter::EventType)readUInt());
            event.setKey(readUInt());
            event.setButton(readUInt());
            int x = readInt();
            int y = readInt();
            int width = readUInt();
            int height = readUInt();
            event.setWindowRectangle(x,y,width,height);
            float xmin = readFloat();
            float ymin = readFloat();
            float xmax = readFloat();
            float ymax = readFloat();
            event.setInputRange(xmin,ymin,xmax,ymax);
            event.setX(readFloat());
            event.setY(readFloat());
            event.setButtonMask(readUInt());
            event.setModKeyMask(readUInt());
            event.setTime(readDouble());
        }
        
        void write(CameraPacket& cameraPacket)
        {
            writeUInt(cameraPacket._byte_order);
            
            writeUInt(cameraPacket._masterKilled);
            
            write(cameraPacket._matrix);
            write(cameraPacket._frameStamp);
        
            writeUInt(cameraPacket._events.size());
            for(osgGA::EventQueue::Events::iterator itr = cameraPacket._events.begin();
                itr != cameraPacket._events.end();
                ++itr)
            {
                write(*(*itr));
            }
        }

        void read(CameraPacket& cameraPacket)
        {
            cameraPacket._byte_order = readUInt();
            if (cameraPacket._byte_order != SWAP_BYTES_COMPARE)
            {
                _swapBytes = !_swapBytes;
            }
            
            cameraPacket._masterKilled = readUInt()!=0;
            
            read(cameraPacket._matrix);
            read(cameraPacket._frameStamp);
        
            cameraPacket._events.clear();
            unsigned int numEvents = readUInt();
            for(unsigned int i=0;i<numEvents;++i)
            {
                osgGA::GUIEventAdapter* event = new osgGA::GUIEventAdapter;
                read(*(event));
                cameraPacket._events.push_back(event);
            }
        }
};

void CameraPacket::readEventQueue(osgViewer::Viewer& viewer)
{
    _events.clear();

    viewer.getEventQueue()->copyEvents(_events);

    osg::notify(osg::INFO)<<"written events = "<<_events.size()<<std::endl;
}

void CameraPacket::writeEventQueue(osgViewer::Viewer& viewer)
{
    osg::notify(osg::INFO)<<"received events = "<<_events.size()<<std::endl;

    viewer.getEventQueue()->appendEvents(_events);
}



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
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates how to approach implementation of clustering.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-m","Set viewer to MASTER mode, sending view via packets.");
    arguments.getApplicationUsage()->addCommandLineOption("-s","Set viewer to SLAVE mode, receiving view via packets.");
    arguments.getApplicationUsage()->addCommandLineOption("-n <int>","Socket number to transmit packets");
    arguments.getApplicationUsage()->addCommandLineOption("-f <float>","Field of view of camera");
    arguments.getApplicationUsage()->addCommandLineOption("-o <float>","Offset angle of camera");
    
    // construct the viewer.
    osgViewer::Viewer viewer;


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

    if (camera_fov>0.0f)
    {
        double fovy, aspectRatio, zNear, zFar;
        viewer.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio,zNear, zFar);
        
        double original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;
        std::cout << "setting lens perspective : original "<<original_fov<<"  "<<fovy<<std::endl;
        
        fovy = atan(tan(osg::DegreesToRadians(camera_fov)*0.5)/aspectRatio)*2.0;
        viewer.getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio,zNear, zFar);
    
        viewer.getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio,zNear, zFar);
        original_fov = atan(tan(osg::DegreesToRadians(fovy)*0.5)*aspectRatio)*2.0;
        std::cout << "setting lens perspective : new "<<original_fov<<"  "<<fovy<<std::endl;
    }

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());


    // create the windows and run the threads.
    viewer.realize();


    CameraPacket *cp = new CameraPacket;

    // objects for managing the broadcasting and recieving of camera packets.
    Broadcaster     bc;
    Receiver        rc;

    bc.setPort(static_cast<short int>(socketNumber));
    rc.setPort(static_cast<short int>(socketNumber));

    bool masterKilled = false;
    
    DataConverter scratchPad(1024);

    while( !viewer.done() && !masterKilled )
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();
                 
        viewer.advance();

        // special handling for working as a cluster.
        switch (viewerMode)
        {
        case(MASTER):
            {
                
                // take camera zero as the guide.
                osg::Matrix modelview(viewer.getCamera()->getViewMatrix());
                
                cp->setPacket(modelview,viewer.getFrameStamp());
                
                cp->readEventQueue(viewer);

                scratchPad.reset();
                scratchPad.write(*cp);

                scratchPad.reset();
                scratchPad.read(*cp);

                bc.setBuffer(scratchPad._startPtr, scratchPad._numBytes);
                
                std::cout << "bc.sync()"<<scratchPad._numBytes<<std::endl;

                bc.sync();
                
            }
            break;
        case(SLAVE):
            {

                rc.setBuffer(scratchPad._startPtr, scratchPad._numBytes);

                rc.sync();
                
                scratchPad.reset();
                scratchPad.read(*cp);
    
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
        viewer.eventTraversal();
        viewer.updateTraversal();

        if (viewerMode==SLAVE)
        {
            osg::Matrix modelview;
            cp->getModelView(modelview,camera_offset);
        
            viewer.getCamera()->setViewMatrix(modelview);
        }

        // fire off the cull and draw traversals of the scene.
        if(!masterKilled)
            viewer.renderingTraversals();
        
    }

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
