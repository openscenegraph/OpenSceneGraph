#include <osgGA/StateSetManipulator>

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
    _backface = (_drawState->getMode(GL_CULL_FACE)==osg::StateAttribute::ON);
    _lighting =(_drawState->getMode(GL_LIGHTING)==osg::StateAttribute::ON);
    _texture =(_drawState->getMode(GL_TEXTURE_2D)==osg::StateAttribute::ON);
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

    if(ea.getEventType()==GUIEventAdapter::KEYBOARD){

        switch( ea.getKey() ){

            case 'b' :
                _backface = !_backface;
                if( _backface ) _drawState->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
                else _drawState->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE_OFF);
                aa.requestRedraw();
                return true;
                break;

            case 'l' :
                _lighting = !_lighting ;
                if( _lighting ) _drawState->setMode(GL_LIGHTING,osg::StateAttribute::ON);
                else _drawState->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE_OFF);
                aa.requestRedraw();
                return true;
                break;

            case 't' :
                _texture = !_texture;
                if (_texture) _drawState->setMode(GL_TEXTURE_2D,osg::StateAttribute::INHERIT);
                else _drawState->setMode(GL_TEXTURE_2D,osg::StateAttribute::OVERRIDE_OFF);
                aa.requestRedraw();
                return true;
                break;

        }
    }
    return false;
}

void StateSetManipulator::accept(GUIEventHandlerVisitor& gehv)
{
    gehv.visit(*this);
}
