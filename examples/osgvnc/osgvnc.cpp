#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>

#include <osgGA/TrackballManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <SDL.h>

#include <iostream>

extern "C" {
#include <rfb/rfbclient.h>
}

struct ButtonMapping { int sdl; int rfb; };

ButtonMapping buttonMapping[]={
    {1, rfbButton1Mask},
    {2, rfbButton2Mask},
    {3, rfbButton3Mask},
    {0,0}
};

static rfbBool resize(rfbClient* client) {

    static char first=TRUE;
    
    osg::Image* image = (osg::Image*)(rfbClientGetClientData(client, 0));
    
    int width=client->width;
    int height=client->height;
    int depth=client->format.bitsPerPixel;

    std::cout<<"resize "<<width<<", "<<height<<", "<<depth<<" image = "<<image<<std::endl;

    image->allocateImage(width,height,1,GL_RGBA,GL_UNSIGNED_BYTE);
    
    client->frameBuffer= (uint8_t*)(image->data());
    
    return TRUE;
}

static void update(rfbClient* client,int x,int y,int w,int h) {

    osg::Image* image = (osg::Image*)(rfbClientGetClientData(client, 0));
    image->dirty();
}

static void kbd_leds(rfbClient* client, int value, int pad) {
    printf("kbd_leds %d %d\n",value,pad);

    /* note: pad is for future expansion 0=unused */
    fprintf(stderr,"Led State= 0x%02X\n", value);
    fflush(stderr);
}

/* trivial support for textchat */
static void text_chat(rfbClient* client, int value, char *text) {
    switch(value) {
    case rfbTextChatOpen:
        fprintf(stderr,"TextChat: We should open a textchat window!\n");
        TextChatOpen(client);
        break;
    case rfbTextChatClose:
        fprintf(stderr,"TextChat: We should close our window!\n");
        break;
    case rfbTextChatFinished:
        fprintf(stderr,"TextChat: We should close our window!\n");
        break;
    default:
        fprintf(stderr,"TextChat: Received \"%s\"\n", text);
        break;
    }
    fflush(stderr);
}

class RfbThread : public OpenThreads::Thread
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

int main(int argc,char** argv)
{
    int i,buttonMask=0;
    
    osg::ref_ptr<osg::Image> image = new osg::Image;
    // image->setPixelBufferObject(new osg::PixelBufferObject(image.get()));

    osg::notify(osg::NOTICE)<<"image = "<<image.get()<<std::endl;

    /* 16-bit: client=rfbGetClient(5,3,2); */
    rfbClient* client=rfbGetClient(8,3,4);
    client->MallocFrameBuffer=resize;
    client->canHandleNewFBSize = TRUE;
    client->GotFrameBufferUpdate=update;
    client->HandleKeyboardLedState=kbd_leds;
    client->HandleTextChat=text_chat;    

    rfbClientSetClientData(client, 0, image.get());
    
    osg::notify(osg::NOTICE)<<"Before rfbInitClient"<<std::endl;

    if(!rfbInitClient(client,&argc,argv))
        return 1;

    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer;


    bool xyPlane = false;
    bool flip = true;
    float width = image->s();
    float height = image->t();

    osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f),
                                       osg::Vec3(width,0.0f,0.0f),
                                       xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height),
                                       0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

    osg::Texture2D* texture = new osg::Texture2D(image.get());
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    
    pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                texture,
                osg::StateAttribute::ON);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(pictureQuad);
    
    viewer.setSceneData(geode);

    viewer.addEventHandler(new osgViewer::StatsHandler);
    
    RfbThread rfbThread(client);
    rfbThread.startThread();

    return viewer.run();
}

