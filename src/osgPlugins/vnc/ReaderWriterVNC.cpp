/* -*-c++-*- OpenSceneGraph - Copyright (C) 1999-2008 Robert Osfield 
 *
 * This software is open source and may be redistributed and/or modified under  
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * include LICENSE.txt for more details.
*/

#include <osg/Timer>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <osgWidget/VncClient>

extern "C" {
#include <rfb/rfbclient.h>
}

class LibVncImage : public osgWidget::VncImage
{
    public:
    
        LibVncImage();

        bool connect(const std::string& hostname);

        void close();
        
        virtual bool sendPointerEvent(int x, int y, int buttonMask);

        double getTimeOfLastUpdate() const { return _timeOfLastUpdate; }
        double getTimeOfLastRender() const { return _timeOfLastRender; }

        double time() const { return osg::Timer::instance()->time_s(); }

        virtual bool sendKeyEvent(int key, bool keyDown);

        virtual void setFrameLastRendered(const osg::FrameStamp* frameStamp);

        void updated();

        static rfbBool resizeImage(rfbClient* client);
        
        static void updateImage(rfbClient* client,int x,int y,int w,int h);
        
        double                      _timeOfLastUpdate;
        double                      _timeOfLastRender;

        bool                        _active;
        osg::ref_ptr<osg::RefBlock> _inactiveBlock;

    protected:
    
        virtual ~LibVncImage();

        class RfbThread : public osg::Referenced, public OpenThreads::Thread
        {
        public:

            RfbThread(rfbClient* client, LibVncImage* image):
                _client(client),
                _image(image),
                _done(false) {}

            virtual ~RfbThread()
            {
                _done = true;
                while(isRunning()) 
                {
                    OpenThreads::Thread::YieldCurrentThread();
                }
            }

            virtual void run()
            {
                do
                {
                    if (_image->_active)
                    {               
                        int i=WaitForMessage(_client,5000);
                        if(i<0)
                            return;

                        if(i)
                        {
                            osg::notify(osg::NOTICE)<<"Handling "<<i<<" messages"<<std::endl;
                        
                            if(!HandleRFBServerMessage(_client))
                            return;

                            _image->updated();
                        }
                    }
                    else
                    {
                        _image->_inactiveBlock->block();
                    }
                    
                    
                    double deltaTime = _image->getTimeOfLastRender() - _image->getTimeOfLastUpdate();
                    if (deltaTime<-0.01)
                    {
                        //osg::notify(osg::NOTICE)<<"Inactive"<<std::endl;
                        //_image->_active = false;
                    }
                    else
                    {
                        _image->_active = true;
                    }

                } while (!_done && !testCancel());
            }

            rfbClient*                      _client;
            osg::observer_ptr<LibVncImage>  _image;
            bool                            _done;

        };

    public:

        rfbClient* _client;

        osg::ref_ptr<RfbThread>     _rfbThread;
      
};

LibVncImage::LibVncImage():
    _client(0)
{
    // setPixelBufferObject(new osg::PixelBufferObject(this);

    _inactiveBlock = new osg::RefBlock;

}

LibVncImage::~LibVncImage()
{
    close();
}

static rfbBool rfbInitConnection(rfbClient* client)
{
  /* Unless we accepted an incoming connection, make a TCP connection to the
     given VNC server */

  if (!client->listenSpecified) {
    if (!client->serverHost || !ConnectToRFBServer(client,client->serverHost,client->serverPort))
      return FALSE;
  }

  /* Initialise the VNC connection, including reading the password */

  if (!InitialiseRFBConnection(client))
    return FALSE;

  if (!SetFormatAndEncodings(client))
    return FALSE;

  client->width=client->si.framebufferWidth;
  client->height=client->si.framebufferHeight;
  client->MallocFrameBuffer(client);

  if (client->updateRect.x < 0) {
    client->updateRect.x = client->updateRect.y = 0;
    client->updateRect.w = client->width;
    client->updateRect.h = client->height;
  }

  if (client->appData.scaleSetting>1)
  {
      if (!SendScaleSetting(client, client->appData.scaleSetting))
          return FALSE;
      if (!SendFramebufferUpdateRequest(client,
			      client->updateRect.x / client->appData.scaleSetting,
			      client->updateRect.y / client->appData.scaleSetting,
			      client->updateRect.w / client->appData.scaleSetting,
			      client->updateRect.h / client->appData.scaleSetting,
			      FALSE))
	      return FALSE;
  }
  else
  {
      if (!SendFramebufferUpdateRequest(client,
			      client->updateRect.x, client->updateRect.y,
			      client->updateRect.w, client->updateRect.h,
			      FALSE))
      return FALSE;
  }

  return TRUE;
}


bool LibVncImage::connect(const std::string& hostname)
{
    if (hostname.empty()) return false;

    if (_client) close();

    _client = rfbGetClient(8,3,4);
    _client->canHandleNewFBSize = TRUE;
    _client->MallocFrameBuffer = resizeImage;
    _client->GotFrameBufferUpdate = updateImage;
    _client->HandleKeyboardLedState = 0;
    _client->HandleTextChat = 0;

    rfbClientSetClientData(_client, 0, this);
    
    _client->serverHost = strdup(hostname.c_str());

    // _client->serverPort = ;
    // _client->appData.qualityLevel = ;
    // _client->appData.encodings = ;
    // _client->appData.compressLevel = ;
    // _client->appData.scaleSetting = ;

    if(rfbInitConnection(_client))
    {
        _rfbThread = new RfbThread(_client, this);
        _rfbThread->startThread();
        
        return true;
    }
    else
    {
        close();
        
        return false;
    }
}


void LibVncImage::close()
{
    if (_rfbThread.valid())
    {
        _inactiveBlock->release();

        // stop the client thread
        _rfbThread = 0;
    }

    if (_client)
    {
        // close the client
        rfbClientCleanup(_client);
        _client = 0;
    }
}


rfbBool LibVncImage::resizeImage(rfbClient* client) 
{
    osg::Image* image = (osg::Image*)(rfbClientGetClientData(client, 0));
    
    int width=client->width;
    int height=client->height;
    int depth=client->format.bitsPerPixel;

    osg::notify(osg::NOTICE)<<"resize "<<width<<", "<<height<<", "<<depth<<" image = "<<image<<std::endl;

    image->allocateImage(width,height,1,GL_RGBA,GL_UNSIGNED_BYTE);
    
    client->frameBuffer= (uint8_t*)(image->data());
    
    return TRUE;
}

void LibVncImage::updateImage(rfbClient* client,int x,int y,int w,int h)
{
    osg::Image* image = (osg::Image*)(rfbClientGetClientData(client, 0));
    image->dirty();
}

bool LibVncImage::sendPointerEvent(int x, int y, int buttonMask)
{
    if (_client)
    {
        SendPointerEvent(_client ,x, y, buttonMask);
        return true;
    }
    return false;
}

bool LibVncImage::sendKeyEvent(int key, bool keyDown)
{
    if (_client)
    {
        SendKeyEvent(_client, key, keyDown ? TRUE : FALSE);
        return true;
    }
    return false;
}


void LibVncImage::setFrameLastRendered(const osg::FrameStamp*)
{
    _timeOfLastRender = time();

    if (!_active) _inactiveBlock->release();
    _active = true;
}

void LibVncImage::updated()
{
    _timeOfLastUpdate = time();
}

class ReaderWriterVNC : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterVNC()
        {
            supportsExtension("vnc","VNC plugin");
        }
        
        virtual const char* className() const { return "VNC plugin"; }

        virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            return readImage(file,options);
        }

        virtual osgDB::ReaderWriter::ReadResult readImage(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            if (!osgDB::equalCaseInsensitive(osgDB::getFileExtension(fileName),"vnc"))
            {
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }

            std::string hostname = osgDB::getNameLessExtension(fileName);
            
            osg::notify(osg::NOTICE)<<"Hostname = "<<hostname<<std::endl;

            osg::ref_ptr<LibVncImage> image = new LibVncImage;
            image->setDataVariance(osg::Object::DYNAMIC);
            
            image->setOrigin(osg::Image::TOP_LEFT);

            if (!image->connect(hostname))
            {
                return "Could not connect to "+hostname;
            }
            
            return image.get();
        }
        
        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = readImage(fileName, options);
            if (!result.validImage()) return result;
            
            osg::ref_ptr<osgWidget::VncClient> vncClient = new osgWidget::VncClient();
            if (vncClient->assign(dynamic_cast<osgWidget::VncImage*>(result.getImage())))
            {
                return vncClient.release();
            }
            else
            {
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterVNC> g_readerWriter_VNC_Proxy;

