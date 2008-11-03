#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <osgViewer/ViewerEventHandlers>

extern "C" {
#include <rfb/rfbclient.h>
}

class VncImage : public osg::Image
{
    public:
    
        VncImage();

        bool connect(int* argc, char** argv);
        
        bool connect(const std::string& hostname, const osgDB::ReaderWriter::Options* options);

        void close();
        
        virtual void sendPointerEvent(int x, int y, int buttonMask);

        virtual void sendKeyEvent(int key, bool keyDown);

        static rfbBool resizeImage(rfbClient* client);
        
        static void updateImage(rfbClient* client,int x,int y,int w,int h);
        
    protected:
    
        virtual ~VncImage();

        class RfbThread : public osg::Referenced, public OpenThreads::Thread
        {
        public:

            RfbThread(rfbClient* client):
                _client(client),
                _done(false) {}

            virtual ~RfbThread()
            {
                _done = true;
                cancel();
                while(isRunning()) 
                {
                    OpenThreads::Thread::YieldCurrentThread();
                }
            }

            virtual void run()
            {
                do
                {
                    int i=WaitForMessage(_client,500);
                    if(i<0)
                        return;
                    if(i)
                        if(!HandleRFBServerMessage(_client))
                        return;

                } while (!_done && !testCancel());
            }

            rfbClient*  _client;
            bool        _done;

        };

    public:

        rfbClient* _client;

        osg::ref_ptr<RfbThread> _rfbThread;
      
};

VncImage::VncImage():
    _client(0)
{
    // setPixelBufferObject(new osg::PixelBufferObject(this);

}

VncImage::~VncImage()
{
    close();
}

bool VncImage::connect(int* argc, char** argv)
{
    if (_client) close();

    _client = rfbGetClient(8,3,4);
    _client->canHandleNewFBSize = TRUE;
    _client->MallocFrameBuffer = resizeImage;
    _client->GotFrameBufferUpdate = updateImage;
    _client->HandleKeyboardLedState = 0;
    _client->HandleTextChat = 0;

    rfbClientSetClientData(_client, 0, this);
    
    if (rfbInitClient(_client,argc,argv))
    {
        _rfbThread = new RfbThread(_client);
        _rfbThread->startThread();

        return true;
    }
    else
    {
        close();
        
        return false;
    }
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


bool VncImage::connect(const std::string& hostname, const osgDB::ReaderWriter::Options* options)
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
        _rfbThread = new RfbThread(_client);
        _rfbThread->startThread();
        
        return true;
    }
    else
    {
        close();
        
        return false;
    }
}


void VncImage::close()
{
    if (_rfbThread.valid())
    {
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


rfbBool VncImage::resizeImage(rfbClient* client) 
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

void VncImage::updateImage(rfbClient* client,int x,int y,int w,int h)
{
    osg::Image* image = (osg::Image*)(rfbClientGetClientData(client, 0));
    image->dirty();
}

void VncImage::sendPointerEvent(int x, int y, int buttonMask)
{
    if (_client)
    {
        SendPointerEvent(_client ,x, y, buttonMask);
    }
}

void VncImage::sendKeyEvent(int key, bool keyDown)
{
    if (_client)
    {
        SendKeyEvent(_client, key, keyDown ? TRUE : FALSE);
    }
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

            osg::ref_ptr<VncImage> image = new VncImage;
            image->setDataVariance(osg::Object::DYNAMIC);
            
            image->setOrigin(osg::Image::TOP_LEFT);

            if (!image->connect(hostname, options))
            {
                return "Could not connect to "+hostname;
            }
            
            return image.get();
        }
        
        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = readImage(fileName, options);
            if (!result.validImage()) return result;
            
            osg::Image* image = result.getImage();
            
            bool xyPlane = false;
            bool flip = image->getOrigin()==osg::Image::TOP_LEFT;
            osg::Vec3 origin = osg::Vec3(0.0f,0.0f,0.0f);
            float width = 1.0;
            float height = float(image->t())/float(image->s());
            osg::Vec3 widthAxis = osg::Vec3(width,0.0f,0.0f);
            osg::Vec3 heightAxis = xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height);

            osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(origin, widthAxis, heightAxis,
                                               0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

            osg::Texture2D* texture = new osg::Texture2D(image);
            texture->setResizeNonPowerOfTwoHint(false);
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

            pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                        texture,
                        osg::StateAttribute::ON);

            pictureQuad->setEventCallback(new osgViewer::InteractiveImageHandler(image));

            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(pictureQuad);

            return geode;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterVNC> g_readerWriter_VNC_Proxy;

