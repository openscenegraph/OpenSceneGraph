#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgWidget/Browser>

int main(int argc,char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);
    
    osgWidget::BrowserManager::instance()->init(arguments[0]);

    osgViewer::Viewer viewer(arguments);

    osgWidget::GeometryHints hints(osg::Vec3(0.0f,0.0f,0.0f),
                                   osg::Vec3(1.0f,0.0f,0.0f),
                                   osg::Vec3(0.0f,0.0f,1.0f),
                                   osg::Vec4(1.0f,1.0f,1.0f,1.0f),
                                   osgWidget::GeometryHints::RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO);

    osg::ref_ptr<osg::Group> group = new osg::Group;

    for(int i=1; i<arguments.argc(); ++i)
    {
        if (!arguments.isOption(i))
        {
            osg::ref_ptr<osgWidget::Browser> browser = new osgWidget::Browser;
            if (browser->open(arguments[i], hints))
            {            
                group->addChild(browser.get());
                
                hints.position.x() += 1.1f;
            }
        }
    }

    viewer.setSceneData(group.get());

    viewer.addEventHandler(new osgViewer::StatsHandler);

    return viewer.run();
}
