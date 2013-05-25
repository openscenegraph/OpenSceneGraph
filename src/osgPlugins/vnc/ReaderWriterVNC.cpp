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

        double getTimeOfLastRender() const { return _timeOfLastRender; }

        double time() const { return osg::Timer::instance()->time_s(); }

        virtual bool sendKeyEvent(int key, bool keyDown);

        virtual void setFrameLastRendered(const osg::FrameStamp* frameStamp);

        static rfbBool resizeImage(rfbClient* client);

        static void updateImage(rfbClient* client,int x,int y,int w,int h);

        static void passwordCheck(rfbClient* client,const char* encryptedPassWord,int len);
        static char* getPassword(rfbClient* client);

        std::string                 _optionString;
        std::string                 _username;
        std::string                 _password;

        double                      _timeOfLastRender;

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
                    // OSG_NOTICE<<"RfThread::run()"<<std::endl;
                    int i=WaitForMessage(_client,1000000);
                    if (i)
                    {
                        if(!HandleRFBServerMessage(_client))
                        {
                            OSG_NOTICE<<"HandleRFBServerMessage returned non zero value."<<std::endl;
                        }
                        // _image->updated();
                    }
                    else
                    {
                        // OSG_NOTICE<<"Timed out"<<std::endl;
                    }
                    
                    
                    
                    double currentTime = _image->time();
                    double timeBeforeIdle = 0.1;

                    if (currentTime > _image->getTimeOfLastRender()+timeBeforeIdle)
                    {
                        //OSG_NOTICE<<"New: Time to idle"<<std::endl;
                        _image->_inactiveBlock->reset();
                        _image->_inactiveBlock->block();
                        //OSG_NOTICE<<"   Finished block."<<std::endl;
                    }
                    else
                    {
                        //OSG_NOTICE<<"New: Should still be active"<<std::endl;
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

void LibVncImage::passwordCheck(rfbClient* client,const char* encryptedPassWord,int len)
{
    OSG_NOTICE<<"LibVncImage::passwordCheck"<<std::endl;
}

char* LibVncImage::getPassword(rfbClient* client)
{
    LibVncImage* image = (LibVncImage*)(rfbClientGetClientData(client, 0));
    OSG_NOTICE<<"LibVncImage::getPassword "<<image->_password<<std::endl;
    return strdup(image->_password.c_str());
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

    // provide the password if we have one assigned
    if (!_password.empty())  _client->GetPassword = getPassword;

    rfbClientSetClientData(_client, 0, this);

    size_t pos = hostname.find(":");
    if (pos == std::string::npos)
    {
        _client->serverHost = strdup(hostname.c_str());
    }
    else
    {
        _client->serverHost = strdup(hostname.substr(0, pos).c_str());
        _client->serverPort = atoi(hostname.substr(pos+1).c_str());
    }

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
    LibVncImage* image = (LibVncImage*)(rfbClientGetClientData(client, 0));

    int width = client->width;
    int height = client->height;
    int depth = client->format.bitsPerPixel;

    OSG_NOTICE<<"resize "<<width<<", "<<height<<", "<<depth<<" image = "<<image<<std::endl;
    PrintPixelFormat(&(client->format));

    bool swap = client->format.redShift!=0;

    if (!image->_optionString.empty())
    {
        if (image->_optionString.find("swap")!=std::string::npos || image->_optionString.find("swop")!=std::string::npos) swap = true;
    }

    GLenum gl_pixelFormat = swap ? GL_BGRA : GL_RGBA;

    if (!image->_optionString.empty())
    {
        if (image->_optionString.find("RGB")!=std::string::npos) gl_pixelFormat = GL_RGBA;
        if (image->_optionString.find("RGBA")!=std::string::npos) gl_pixelFormat = GL_RGBA;
        if (image->_optionString.find("BGR")!=std::string::npos) gl_pixelFormat = GL_BGRA;
        if (image->_optionString.find("BGRA")!=std::string::npos) gl_pixelFormat = GL_BGRA;
    }

    image->allocateImage(width, height, 1, gl_pixelFormat, GL_UNSIGNED_BYTE);
    image->setInternalTextureFormat(GL_RGBA);



    client->frameBuffer= (uint8_t*)(image->data());

    return TRUE;
}

void LibVncImage::updateImage(rfbClient* client,int x,int y,int w,int h)
{
    LibVncImage* image = (LibVncImage*)(rfbClientGetClientData(client, 0));
    
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
    
    // release block
    _inactiveBlock->release();
}

class ReaderWriterVNC : public osgDB::ReaderWriter
{
    public:

        ReaderWriterVNC()
        {
            supportsExtension("vnc","VNC plugin");

            supportsOption("swap","Swaps the pixel format order, exchanging the red and blue channels.");
            supportsOption("swop","American spelling, same effect as swap.");
            supportsOption("RGB","Use RGBA pixel format for the vnc image");
            supportsOption("RGBA","Use RGBA pixel format for the vnc image");
            supportsOption("BGR","Use BGRA pixel format for the vnc image");
            supportsOption("BGRA","Use BGRA pixel format for the vnc image");
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

            OSG_NOTICE<<"Hostname = "<<hostname<<std::endl;

            osg::ref_ptr<LibVncImage> image = new LibVncImage;
            image->setDataVariance(osg::Object::DYNAMIC);
            image->setOrigin(osg::Image::TOP_LEFT);

            const osgDB::AuthenticationMap* authenticationMap = (options && options->getAuthenticationMap()) ?
                    options->getAuthenticationMap() :
                    osgDB::Registry::instance()->getAuthenticationMap();

            const osgDB::AuthenticationDetails* details = authenticationMap ?
                authenticationMap->getAuthenticationDetails(hostname) :
                0;

            // configure authentication if required.
            if (details)
            {
                OSG_NOTICE<<"Passing in password = "<<details->password<<std::endl;

                image->_username = details->username;
                image->_password = details->password;
            }

            if (options && !options->getOptionString().empty())
            {
                image->_optionString = options->getOptionString();
            }

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
REGISTER_OSGPLUGIN(vnc, ReaderWriterVNC)

