#include <osg/Image>
#include <osg/Geometry>
#include <osg/Texture2D>

#include <osgGA/TrackballManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>
#include <osg/io_utils>

extern "C" {
#include <rfb/rfbclient.h>
}

static rfbBool resizeImage(rfbClient* client) 
{
    osg::Image* image = (osg::Image*)(rfbClientGetClientData(client, 0));
    
    int width=client->width;
    int height=client->height;
    int depth=client->format.bitsPerPixel;

    std::cout<<"resize "<<width<<", "<<height<<", "<<depth<<" image = "<<image<<std::endl;

    image->allocateImage(width,height,1,GL_RGBA,GL_UNSIGNED_BYTE);
    
    client->frameBuffer= (uint8_t*)(image->data());
    
    return TRUE;
}

static void updateImage(rfbClient* client,int x,int y,int w,int h)
{
    osg::Image* image = (osg::Image*)(rfbClientGetClientData(client, 0));
    image->dirty();
}


class RfbEventHandler : public osgGA::GUIEventHandler
{
public:

    RfbEventHandler(rfbClient* client):
        _client(client) {}
    
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv);
    
    rfbKeySym key2rfbKeySym(int key)
    {
        return rfbKeySym(key);
    }

protected:

    virtual ~RfbEventHandler() {}
    
    bool mousePosition(osgViewer::View* view, osg::NodeVisitor* nv, const osgGA::GUIEventAdapter& ea, int& x, int &y) const;

    rfbClient*  _client;
    
};

bool RfbEventHandler::mousePosition(osgViewer::View* view, osg::NodeVisitor* nv, const osgGA::GUIEventAdapter& ea, int& x, int &y) const
{
    osgUtil::LineSegmentIntersector::Intersections intersections;
    bool foundIntersection = view==0 ? false :
        (nv==0 ? view->computeIntersections(ea.getX(), ea.getY(), intersections) :
                 view->computeIntersections(ea.getX(), ea.getY(), nv->getNodePath(), intersections));

    if (foundIntersection)
    {

        osg::Vec2 tc(0.5f,0.5f);

        // use the nearest intersection                 
        const osgUtil::LineSegmentIntersector::Intersection& intersection = *(intersections.begin());
        osg::Drawable* drawable = intersection.drawable.get();
        osg::Geometry* geometry = drawable ? drawable->asGeometry() : 0;
        osg::Vec3Array* vertices = geometry ? dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray()) : 0;
        if (vertices)
        {
            // get the vertex indices.
            const osgUtil::LineSegmentIntersector::Intersection::IndexList& indices = intersection.indexList;
            const osgUtil::LineSegmentIntersector::Intersection::RatioList& ratios = intersection.ratioList;

            if (indices.size()==3 && ratios.size()==3)
            {
                unsigned int i1 = indices[0];
                unsigned int i2 = indices[1];
                unsigned int i3 = indices[2];

                float r1 = ratios[0];
                float r2 = ratios[1];
                float r3 = ratios[2];

                osg::Array* texcoords = (geometry->getNumTexCoordArrays()>0) ? geometry->getTexCoordArray(0) : 0;
                osg::Vec2Array* texcoords_Vec2Array = dynamic_cast<osg::Vec2Array*>(texcoords);
                if (texcoords_Vec2Array)
                {
                    // we have tex coord array so now we can compute the final tex coord at the point of intersection.                                
                    osg::Vec2 tc1 = (*texcoords_Vec2Array)[i1];
                    osg::Vec2 tc2 = (*texcoords_Vec2Array)[i2];
                    osg::Vec2 tc3 = (*texcoords_Vec2Array)[i3];
                    tc = tc1*r1 + tc2*r2 + tc3*r3;
                }
            }

        }

        x = int( float(_client->width) * tc.x() );
        y = int( float(_client->height) * tc.y() );

        return true;
    }
    
    return false;
}


bool RfbEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* nv)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::MOVE):
        case(osgGA::GUIEventAdapter::DRAG):
        case(osgGA::GUIEventAdapter::PUSH):
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            int x,y;
            if (mousePosition(view, nv, ea, x, y))
            {
                SendPointerEvent(_client,x,y, ea.getButtonMask());
                return true;
            }
            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            int x,y;
            bool sendKeyEvent = mousePosition(view, nv, ea, x, y);
        
            if (sendKeyEvent)
            {
                SendKeyEvent(_client,
                    key2rfbKeySym(ea.getKey()),
                    (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)?TRUE:FALSE);

                return true;
            }
            else
            {
                if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Escape)
                {
                    osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
                    if (viewer) viewer->setDone(true);
                }
            }
            
        }

        default:
            return false;
    }
    return false;
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
    osg::ref_ptr<osg::Image> image = new osg::Image;
    // image->setPixelBufferObject(new osg::PixelBufferObject(image.get()));

    /* 16-bit: client=rfbGetClient(5,3,2); */
    rfbClient* client=rfbGetClient(8,3,4);
    client->canHandleNewFBSize = TRUE;
    client->MallocFrameBuffer = resizeImage;
    client->GotFrameBufferUpdate = updateImage;
    client->HandleKeyboardLedState = 0;
    client->HandleTextChat = 0;

    rfbClientSetClientData(client, 0, image.get());

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
    viewer.addEventHandler(new RfbEventHandler(client));
    
    RfbThread rfbThread(client);
    rfbThread.startThread();
    
    viewer.setKeyEventSetsDone(0);

    return viewer.run();

    // rfbClientCleanup(cl);
}

