#include <osgProducer/StatsEventHandler>

using namespace osgProducer;

bool StatsEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    if(!_cg) return false;

    if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
    {

        switch( ea.getKey() )
        {
        case 's' :
            _cg->setInstrumentationMode(!_cg->getInstrumentationMode());
            return true;

        case 'v' :
            _cg->setBlockOnVsync(!_cg->getBlockOnVsync());
            //std::cout<<"_cg->getBlockOnVsync()="<<_cg->getBlockOnVsync()<<std::endl;
            return true;

        default:
            break;

        }
    }
    return false;

}

void StatsEventHandler::accept(osgGA::GUIEventHandlerVisitor& gehv)
{
    gehv.visit(*this);
}

void StatsEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("s","Toggle intrumention");
    usage.addKeyboardMouseBinding("v","Toggle block and vsync");
}
