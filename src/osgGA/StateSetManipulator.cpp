#include <osgGA/StateSetManipulator>

#include <osg/PolygonMode>

using namespace osg;
using namespace osgGA;

StateSetManipulator::StateSetManipulator(): _drawState(NULL)
{
}

StateSetManipulator::~StateSetManipulator()
{
}

void StateSetManipulator::setStateSet(StateSet *drawState)
{
    _drawState=drawState;
    if(!_drawState.valid()) return;
    _backface = (_drawState->getMode(GL_CULL_FACE)&osg::StateAttribute::ON);
    _lighting =(_drawState->getMode(GL_LIGHTING)&osg::StateAttribute::ON);
    _texture =(_drawState->getTextureMode(0,GL_TEXTURE_2D)&osg::StateAttribute::ON);
}

StateSet *StateSetManipulator::getStateSet()
{
    return _drawState.get();
}

const StateSet *StateSetManipulator::getStateSet() const
{
    return _drawState.get();
}

bool StateSetManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    if(!_drawState.valid()) return false;

    if(ea.getEventType()==GUIEventAdapter::KEYDOWN){

        switch( ea.getKey() ){

            case 'b' :
                _backface = !_backface;
                if( _backface ) _drawState->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
                else _drawState->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
                aa.requestRedraw();
                return true;
                break;

            case 'l' :
                _lighting = !_lighting ;
                if( _lighting ) _drawState->setMode(GL_LIGHTING,osg::StateAttribute::ON);
                else _drawState->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
                aa.requestRedraw();
                return true;
                break;

            case 't' :
                _texture = !_texture;
                if (_texture) _drawState->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::INHERIT);
                else _drawState->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
                aa.requestRedraw();
                return true;
                break;

        case 'w' :
            {
                osg::PolygonMode* polyModeObj = dynamic_cast<osg::PolygonMode*>(_drawState->getAttribute(osg::StateAttribute::POLYGONMODE));
                if (!polyModeObj) 
                {
                    polyModeObj = new osg::PolygonMode;
                    _drawState->setAttribute(polyModeObj);
                }
                
                // cycle through the available modes.  
                switch(polyModeObj->getMode(osg::PolygonMode::FRONT_AND_BACK))
                {
                    case osg::PolygonMode::FILL : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE); break;
                    case osg::PolygonMode::LINE : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::POINT); break;
                    case osg::PolygonMode::POINT : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL); break;
                }
            }
            break;
        }
    }
    return false;
}

void StateSetManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("b","Toggle backface culling");
    usage.addKeyboardMouseBinding("l","Toggle lighting");
    usage.addKeyboardMouseBinding("t","Toggle texturing");
    usage.addKeyboardMouseBinding("w","Toggle polygon fill mode between fill, line (wire frame) and points");
}

void StateSetManipulator::accept(GUIEventHandlerVisitor& gehv)
{
    gehv.visit(*this);
}
