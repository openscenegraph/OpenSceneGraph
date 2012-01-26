#include <osgWidget/VncClient>

#include <osgDB/Registry>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

class EscapeHandler : public osgGA::GUIEventHandler
{
    public:
    
        EscapeHandler() {}

        bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
        {
            if (ea.getHandled()) return false;

            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYUP):
                {
                    if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Escape)
                    {
                        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                        if (view) view->getViewerBase()->setDone(true);
                        
                        return true;
                    }
                }

                default:
                    return false;
            }
            return false;
        }
};

int main(int argc,char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);

    osgWidget::GeometryHints hints(osg::Vec3(0.0f,0.0f,0.0f),
                                   osg::Vec3(1.0f,0.0f,0.0f),
                                   osg::Vec3(0.0f,0.0f,1.0f),
                                   osg::Vec4(1.0f,1.0f,1.0f,1.0f),
                                   osgWidget::GeometryHints::RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO);

    osg::ref_ptr<osg::Group> group = new osg::Group;

    std::string password;
    while(arguments.read("--password",password))
    {
    }

    for(int i=1; i<arguments.argc(); ++i)
    {
        if (!arguments.isOption(i))
        {
            std::string hostname = arguments[i];

            if (!password.empty())
            {
                if (!osgDB::Registry::instance()->getAuthenticationMap()) osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
                osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(hostname, new osgDB::AuthenticationDetails("", password));
            }

            osg::ref_ptr<osgWidget::VncClient> vncClient = new osgWidget::VncClient;
            if (vncClient->connect(arguments[i], hints))
            {            
                group->addChild(vncClient.get());
                
                hints.position.x() += 1.1f;
            }
        }
    }

    viewer.setSceneData(group.get());

    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add a custom escape handler, but disable the standard viewer one to enable the vnc images to handle
    // the escape without it getting caught by the viewer.
    viewer.addEventHandler(new EscapeHandler);    
    viewer.setKeyEventSetsDone(0);

    return viewer.run();
}

