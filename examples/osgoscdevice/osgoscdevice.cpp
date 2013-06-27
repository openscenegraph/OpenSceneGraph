/* OpenSceneGraph example, osgcubemap.
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

#include <iostream>

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/PolygonOffset>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/ValueObject>
#include <osg/FrontFace>
#include <osgDB/ReadFile>

#include <osgText/Text>

#include <osgGA/Device>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>

#include <osgViewer/CompositeViewer>

#include <osgFX/Scribe>

#include <osg/io_utils>


// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public:

    PickHandler(osgGA::Device* device):
        _device(device) {}

    ~PickHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

    virtual void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea);

    void setLabel(const std::string& name, float x, float y)
    {
        osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter();
        ea->setEventType(osgGA::GUIEventAdapter::USER);
        ea->setName("pick-result");
        ea->setUserValue("name", name);
        ea->setUserValue("x", x);
        ea->setUserValue("y", y);

        _device->sendEvent(*ea);
    }

protected:

    osg::ref_ptr<osgGA::Device>  _device;
};

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::PUSH):
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            if (view) pick(view,ea);
            return false;
        }

        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey() == 't')
            {
                osg::ref_ptr<osgGA::GUIEventAdapter> user_event = new osgGA::GUIEventAdapter();
                user_event->setEventType(osgGA::GUIEventAdapter::USER);
                user_event->setUserValue("vec2f", osg::Vec2f(1.0f,2.0f));
                user_event->setUserValue("vec3f", osg::Vec3f(1.0f,2.0f, 3.0f));
                user_event->setUserValue("vec4f", osg::Vec4f(1.0f,2.0f, 3.0f, 4.0f));

                user_event->setUserValue("vec2d", osg::Vec2d(1.0,2.0));
                user_event->setUserValue("vec3d", osg::Vec3d(1.0,2.0, 3.0));
                user_event->setUserValue("vec4d", osg::Vec4d(1.0,2.0, 3.0, 4.0));

                user_event->setName("osc_test_1");

                _device->sendEvent(*user_event);
            }

        }

        default:
            return false;
    }
}

void PickHandler::pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;

    std::string gdlist="";
    float x = ea.getX();
    float y = ea.getY();
    if (view->computeIntersections(ea, intersections))
    {
        for(osgUtil::LineSegmentIntersector::Intersections::iterator hitr = intersections.begin();
            hitr != intersections.end();
            ++hitr)
        {
            std::ostringstream os;
            if (!hitr->nodePath.empty() && !(hitr->nodePath.back()->getName().empty()))
            {
                // the geodes are identified by name.
                os<<"Object \""<<hitr->nodePath.back()->getName()<<"\""<<std::endl;
            }
            else if (hitr->drawable.valid())
            {
                os<<"Object \""<<hitr->drawable->className()<<"\""<<std::endl;
            }

            os<<"        local coords vertex("<< hitr->getLocalIntersectPoint()<<")"<<"  normal("<<hitr->getLocalIntersectNormal()<<")"<<std::endl;
            os<<"        world coords vertex("<< hitr->getWorldIntersectPoint()<<")"<<"  normal("<<hitr->getWorldIntersectNormal()<<")"<<std::endl;
            const osgUtil::LineSegmentIntersector::Intersection::IndexList& vil = hitr->indexList;
            for(unsigned int i=0;i<vil.size();++i)
            {
                os<<"        vertex indices ["<<i<<"] = "<<vil[i]<<std::endl;
            }

            gdlist += os.str();
        }
    }
    setLabel(gdlist, x, y);
}


class UserEventHandler : public osgGA::GUIEventHandler {
public:

    UserEventHandler(osgText::Text* text) : osgGA::GUIEventHandler(), _text(text) {}

    ~UserEventHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);
private:
    osg::ref_ptr<osgText::Text> _text;
};


class MyValueListVisitor : public osg::ValueObject::GetValueVisitor {
public:
    virtual void apply(bool value)          { _ss << value << " (bool)"; }
    virtual void apply(char value)          { _ss << value << " (char)"; }
    virtual void apply(unsigned char value) { _ss << value << " (unsigned char)"; }
    virtual void apply(short value)         { _ss << value << " (short)"; }
    virtual void apply(unsigned short value){ _ss << value << " (unsigned short)"; }
    virtual void apply(int value)           { _ss << value << " (int)"; }
    virtual void apply(unsigned int value)  { _ss << value << " (unsigned int)"; }
    virtual void apply(float value)         { _ss << value << " (float)"; }
    virtual void apply(double value)        { _ss << value << " (double)"; }
    virtual void apply(const std::string& value)    { _ss << value << " (std::string)"; }
    virtual void apply(const osg::Vec2f& value)     { _ss << value << " (osg::Vec2f)"; }
    virtual void apply(const osg::Vec3f& value)     { _ss << value << " (osg::Vec3f)"; }
    virtual void apply(const osg::Vec4f& value)     { _ss << value << " (osg::Vec4f)"; }
    virtual void apply(const osg::Vec2d& value)     { _ss << value << " (osg::Vec2d)"; }
    virtual void apply(const osg::Vec3d& value)     { _ss << value << " (osg::Vec3d)"; }
    virtual void apply(const osg::Vec4d& value)     { _ss << value << " (osg::Vec4d)"; }
    virtual void apply(const osg::Quat& value)      { _ss << value << " (osg::Quat)"; }
    virtual void apply(const osg::Plane& value)     { _ss << value << " (osg::Plane)"; }
    virtual void apply(const osg::Matrixf& value)   { _ss << value << " (osg::Matrixf)"; }
    virtual void apply(const osg::Matrixd& value)   { _ss << value << " (osg::Matrixd)"; }
    std::string value() const { return _ss.str(); }
    void clear() {_ss.clear(); }
private:
    std::ostringstream _ss;
};

bool UserEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    if (ea.getEventType() == osgGA::GUIEventAdapter::USER) {
        OSG_ALWAYS << "handle user-event: " << ea.getName() << std::endl;

        if (ea.getName() == "/pick-result")
        {
            std::string name("");
            float x(0), y(0);
            ea.getUserValue("name", name);
            ea.getUserValue("x", x);
            ea.getUserValue("y", y);
            std::ostringstream ss;
            ss << "Name: " << std::endl << name << std::endl << std::endl;
            ss << "x: " << y << " y: " << y << std::endl;

            _text->setText(ss.str());
        }
        else if(ea.getName() == "/osgga")
        {
            osg::Vec4 rect;
            ea.getUserValue("resize", rect);
            osg::View* view = dynamic_cast<osgViewer::View*>(&aa);
            if (view && (rect[2] > 0) && (rect[3] > 0))
            {
                OSG_ALWAYS << "resizing view to " << rect << std::endl;
                osgViewer::GraphicsWindow* win = view->getCamera()->getGraphicsContext() ? dynamic_cast<osgViewer::GraphicsWindow*>(view->getCamera()->getGraphicsContext()) : NULL;
                if (win)
                    win->setWindowRectangle(rect[2] + 10 + rect[0], rect[1], rect[2], rect[3]);
            }
        }
        else {
            const osg::UserDataContainer* udc = ea.getUserDataContainer();
            if (udc)
            {
                OSG_ALWAYS << "contents of " << udc->getName() << ": " << std::endl;
                for(unsigned int i = 0; i < udc->getNumUserObjects(); ++i)
                {
                    const osg::ValueObject* vo = dynamic_cast<const osg::ValueObject*>(udc->getUserObject(i));
                    OSG_ALWAYS << "  " << vo->getName() << ": ";

                    MyValueListVisitor vlv;
                    vo->get(vlv);
                    OSG_ALWAYS << vlv.value() << std::endl;
                }
            }
        }
        return true;
    }

    return false;
}

osg::Node* createHUD()
{

    // create the hud. derived from osgHud.cpp
    // adds a set of quads, each in a separate Geode - which can be picked individually
    // eg to be used as a menuing/help system!
    // Can pick texts too!

    osg::Camera* hudCamera = new osg::Camera;
    hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    hudCamera->setProjectionMatrixAsOrtho2D(0,1280,0,1024);
    hudCamera->setViewMatrix(osg::Matrix::identity());
    hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
    hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);

    std::string timesFont("fonts/times.ttf");

    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::Vec3 position(150.0f,800.0f,0.0f);
    osg::Vec3 delta(0.0f,-60.0f,0.0f);

    {
        osg::Geode* geode = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        geode->setName("simple");
        hudCamera->addChild(geode);

        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setText("Picking in Head Up Displays is simple!");
        text->setPosition(position);

        position += delta;
    }


    for (int i=0; i<5; i++) {
        osg::Vec3 dy(0.0f,-30.0f,0.0f);
        osg::Vec3 dx(120.0f,0.0f,0.0f);
        osg::Geode* geode = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        const char *opts[]={"One", "Two", "Three", "January", "Feb", "2003"};
        osg::Geometry *quad=new osg::Geometry;
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        std::string name="subOption";
        name += " ";
        name += std::string(opts[i]);
        geode->setName(name);
        osg::Vec3Array* vertices = new osg::Vec3Array(4); // 1 quad
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(0.8-0.1*i,0.1*i,0.2*i, 1.0));
        quad->setColorArray(colors, osg::Array::BIND_OVERALL);
        (*vertices)[0]=position;
        (*vertices)[1]=position+dx;
        (*vertices)[2]=position+dx+dy;
        (*vertices)[3]=position+dy;
        quad->setVertexArray(vertices);
        quad->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
        geode->addDrawable(quad);
        hudCamera->addChild(geode);

        position += delta;
    }



    { // this displays what has been selected
        osg::Geode* geode = new osg::Geode();
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        geode->setName("The text label");
        hudCamera->addChild(geode);

        position += delta;
    }

    return hudCamera;

}


class ForwardToDeviceEventHandler : public osgGA::GUIEventHandler {
public:
    ForwardToDeviceEventHandler(osgGA::Device* device) : osgGA::GUIEventHandler(), _device(device) {}

    virtual bool handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *, osg::NodeVisitor *)
    {
        _device->sendEvent(ea);
        return false;
    }

protected:
    osg::ref_ptr<osgGA::Device> _device;
};

class OscServiceDiscoveredEventHandler: public ForwardToDeviceEventHandler {
public:
    OscServiceDiscoveredEventHandler() : ForwardToDeviceEventHandler(NULL) {}

    virtual bool handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa, osg::Object *o, osg::NodeVisitor *nv)
    {
        if (_device.valid())
            return ForwardToDeviceEventHandler::handle(ea, aa, o, nv);

        if (ea.getEventType() == osgGA::GUIEventAdapter::USER)
        {
            if (ea.getName() == "/zeroconf/service-added")
            {
                std::string host;
                unsigned int port;
                ea.getUserValue("host", host);
                ea.getUserValue("port", port);

                OSG_ALWAYS << "new osc-service discovered: " << host << ":" << port << std::endl;

                std::ostringstream ss ;
                ss << host << ":" << port << ".sender.osc";
                _device = osgDB::readFile<osgGA::Device>(ss.str());

                osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                if (view)
                    view->addEventHandler(new PickHandler(_device.get()));
                return true;
            }
        }
        return false;
    }

};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->addCommandLineOption("--zeroconf","uses zeroconf to advertise the osc-plugin and to discover it");
    arguments.getApplicationUsage()->addCommandLineOption("--sender","create a view which sends its events via osc");
    arguments.getApplicationUsage()->addCommandLineOption("--recevier","create a view which receive its events via osc");



    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);

    if (!scene)
    {
        std::cout << argv[0] << ": requires filename argument." << std::endl;
        return 1;
    }

    bool use_zeroconf(false);
    bool use_sender(false);
    bool use_receiver(false);
    if(arguments.find("--zeroconf") > 0) { use_zeroconf = true; }
    if(arguments.find("--sender") > 0) { use_sender = true; }
    if(arguments.find("--receiver") > 0) { use_receiver = true; }
    // construct the viewer.
    osgViewer::CompositeViewer viewer(arguments);

    // receiver view
    if (use_receiver) {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 600;
        traits->y = 100;
        traits->width = 400;
        traits->height = 400;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->windowName = "Receiver / view two";

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

        osgViewer::View* view = new osgViewer::View;
        view->setName("View two");
        viewer.addView(view);

        osg::Group* group = new osg::Group();
        group->addChild(scene.get());
        osg::Geode* geode = new osg::Geode();
        group->addChild(geode);

        osgText::Text* text = new osgText::Text();
        geode->addDrawable( text );

        text->setFont("Arial.ttf");
        text->setText("Waiting for data");
        text->setPosition(osg::Vec3(-50,0,30));
        text->setAxisAlignment(osgText::TextBase::SCREEN);
        text->setDataVariance(osg::Object::DYNAMIC);
        text->setCharacterSize(2.0f);
        view->setSceneData(group);
        view->getCamera()->setName("Cam two");
        view->getCamera()->setViewport(new osg::Viewport(0,0, traits->width, traits->height));
        view->getCamera()->setGraphicsContext(gc.get());

        view->addEventHandler( new osgViewer::StatsHandler );
        view->addEventHandler( new UserEventHandler(text) );

        osg::ref_ptr<osgGA::Device> device = osgDB::readFile<osgGA::Device>("0.0.0.0:9000.receiver.osc");
        if (device.valid() && (device->getCapabilities() & osgGA::Device::RECEIVE_EVENTS))
        {
            view->addDevice(device.get());

            // add a zeroconf device, advertising the osc-device
            if(use_zeroconf)
            {
                osgGA::Device* zeroconf_device = osgDB::readFile<osgGA::Device>("_osc._udp:9000.advertise.zeroconf");
                if (zeroconf_device)
                {
                    view->addDevice(zeroconf_device);
                }
            }
        }
        else {
            OSG_WARN << "could not open osc-device, receiving will not work" << std::endl;
        }
    }

    // sender view
    if(use_sender) {
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 100;
        traits->y = 100;
        traits->width = 400;
        traits->height = 400;
        traits->windowDecoration = true;
        traits->doubleBuffer = true;
        traits->sharedContext = 0;
        traits->windowName = "Sender / view one";

        osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());


        osgViewer::View* view = new osgViewer::View;
        view->setName("View one");
        viewer.addView(view);

        osg::Group* g = new osg::Group();
        g->addChild(scene.get());
        g->addChild(createHUD());
        view->setSceneData(g);
        view->getCamera()->setName("Cam one");
        view->getCamera()->setViewport(new osg::Viewport(0,0, traits->width, traits->height));
        view->getCamera()->setGraphicsContext(gc.get());
        view->setCameraManipulator(new osgGA::TrackballManipulator);

        // add the state manipulator
        osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator;
        statesetManipulator->setStateSet(view->getCamera()->getOrCreateStateSet());

        view->addEventHandler( statesetManipulator.get() );
        view->addEventHandler( new osgViewer::StatsHandler );

        if (use_zeroconf)
        {
            osgGA::Device* zeroconf_device = osgDB::readFile<osgGA::Device>("_osc._udp.discover.zeroconf");
            if(zeroconf_device) {
                view->addDevice(zeroconf_device);
                view->getEventHandlers().push_front(new OscServiceDiscoveredEventHandler());

            }
        }
        else
        {
            osg::ref_ptr<osgGA::Device> device = osgDB::readFile<osgGA::Device>("localhost:9000.sender.osc");
            if (device.valid() && (device->getCapabilities() & osgGA::Device::SEND_EVENTS))
            {
                // add as first event handler, so it gets ALL events ...
                view->getEventHandlers().push_front(new ForwardToDeviceEventHandler(device.get()));

                // add the demo-pick-event-handler
                view->addEventHandler(new PickHandler(device.get()));
            }
            else {
                OSG_WARN << "could not open osc-device, sending will not work" << std::endl;
            }
        }
    }




    while (arguments.read("-s")) { viewer.setThreadingModel(osgViewer::CompositeViewer::SingleThreaded); }
    while (arguments.read("-g")) { viewer.setThreadingModel(osgViewer::CompositeViewer::CullDrawThreadPerContext); }
    while (arguments.read("-c")) { viewer.setThreadingModel(osgViewer::CompositeViewer::CullThreadPerCameraDrawThreadPerContext); }

     // run the viewer's main frame loop
     return viewer.run();
}
