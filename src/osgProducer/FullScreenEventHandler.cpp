#include <osgProducer/FullScreenEventHandler>

using namespace osgProducer;

bool FullScreenEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    if(!_cg) return false;

    if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
    {

        switch( ea.getKey() )
        {
          case 'f' :
          {
            Producer::CameraConfig* cfg = _cg->getCameraConfig();
            for( unsigned int i = 0; i < cfg->getNumberOfCameras(); ++i )
            {
                Producer::Camera *cam = cfg->getCamera(i);
                Producer::RenderSurface* rs = cam->getRenderSurface();
                rs->fullScreen(!rs->isFullScreen());
            }

            return true;
          }

        default:
            break;

        }
    }
    return false;

}

void FullScreenEventHandler::accept(osgGA::GUIEventHandlerVisitor& gehv)
{
    gehv.visit(*this);
}

void FullScreenEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("f","Toggle fullscreen");
}
