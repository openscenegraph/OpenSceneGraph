#include <osgProducer/ViewerEventHandler>
#include <osgDB/WriteFile>

using namespace osgProducer;

ViewerEventHandler::ViewerEventHandler(osgProducer::OsgCameraGroup* cg):
    _cg(cg),
    _writeNodeFileName("savedmodel.osg")
{
}

bool ViewerEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
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
          case 'o' :
          {
            osg::Node* node = _cg->getSceneData();
            if (node)
            {
                std::cout<<"writing file "<<_writeNodeFileName<<std::endl;
                osgDB::writeNodeFile(*node,_writeNodeFileName.c_str());
            }
            
            return true;
          }

          case osgGA::GUIEventAdapter::KEY_Help :
          case '/' :
          case '?' :
          {
            if (_cg->getApplicationUsage())
            {
                _cg->getApplicationUsage()->write(std::cout,_cg->getApplicationUsage()->getKeyboardMouseBindings());
            }
            
            return true;
          }

        default:
            break;

        }
    }
    return false;

}

void ViewerEventHandler::accept(osgGA::GUIEventHandlerVisitor& gehv)
{
    gehv.visit(*this);
}

void ViewerEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("f","Toggle fullscreen");
}
