#include <osgGA/StateSetManipulator>

#include <osg/PolygonMode>
#include <osg/ref_ptr>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/TextureCubeMap>


// #define COMPILE_TEXENVFILTER_USAGE
#if COMPILE_TEXENVFILTER_USAGE
    #include <osg/TexEnvFilter>
#endif

using namespace osg;
using namespace osgGA;

StateSetManipulator::StateSetManipulator():
    _backface(false),
    _lighting(false),
    _texture(false),
    _maxNumOfTextureUnits(4)
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
    
    unsigned int mode = osg::StateAttribute::INHERIT|osg::StateAttribute::ON;
    _texture = (_drawState->getTextureMode(0,GL_TEXTURE_1D)&mode) ||
               (_drawState->getTextureMode(0,GL_TEXTURE_2D)&mode) ||
               (_drawState->getTextureMode(0,GL_TEXTURE_3D)&mode) ||
               (_drawState->getTextureMode(0,GL_TEXTURE_RECTANGLE)&mode) ||
               (_drawState->getTextureMode(0,GL_TEXTURE_CUBE_MAP)&mode);
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

    if(ea.getEventType()==GUIEventAdapter::KEYDOWN)
    {

        switch( ea.getKey() )
        {

            case 'b' :
                setBackfaceEnabled(!getBackfaceEnabled());
                aa.requestRedraw();
                return true;
                break;

            case 'l' :
                setLightingEnabled(!getLightingEnabled());
                aa.requestRedraw();
                return true;
                break;

            case 't' :
                setTextureEnabled(!getTextureEnabled());
                aa.requestRedraw();
                return true;
            break;

            case 'w' :
                cyclePolygonMode();
                break;

#if COMPILE_TEXENVFILTER_USAGE
            case 'm' :
                {
                    osg::TexEnvFilter* texenvfilter = dynamic_cast<osg::TexEnvFilter*>(_drawState->getTextureAttribute(0,osg::StateAttribute::TEXENVFILTER));
                    if (!texenvfilter) 
                    {
                        texenvfilter = new osg::TexEnvFilter;
                        _drawState->setTextureAttribute(0,texenvfilter);
                    }

                    // cycle through the available modes.
                    texenvfilter->setLodBias(texenvfilter->getLodBias()+0.1);
                }
                break;
#endif
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


void StateSetManipulator::setBackfaceEnabled(bool newbackface)
{
    _backface = newbackface;
    if( _backface ) _drawState->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
    else _drawState->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
}

void StateSetManipulator::setLightingEnabled(bool newlighting)
{
    _lighting = newlighting;
    if( _lighting ) _drawState->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    else _drawState->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
}

void StateSetManipulator::setTextureEnabled(bool newtexture)
{
    _texture = newtexture;
//    osg::ref_ptr< osg::Texture > tex = dynamic_cast<osg::Texture*>
//        ( _drawState->getAttribute( osg::StateAttribute::TEXTURE ) );
//    cout << tex->numTextureUnits() << endl;

    unsigned int mode = osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF;
    if ( _texture ) mode = osg::StateAttribute::INHERIT|osg::StateAttribute::ON;
    for( unsigned int ii=0; ii<_maxNumOfTextureUnits; ii++ )
    {
            _drawState->setTextureMode( ii, GL_TEXTURE_1D, mode );
            _drawState->setTextureMode( ii, GL_TEXTURE_2D, mode );
            _drawState->setTextureMode( ii, GL_TEXTURE_3D, mode );
            _drawState->setTextureMode( ii, GL_TEXTURE_RECTANGLE, mode );
            _drawState->setTextureMode( ii, GL_TEXTURE_CUBE_MAP, mode);
    }
}

void StateSetManipulator::setPolygonMode(osg::PolygonMode::Mode newpolygonmode)
{
    osg::PolygonMode* polyModeObj = getOrCreatePolygonMode();

    polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,newpolygonmode);
}

void StateSetManipulator::cyclePolygonMode()
{
    osg::PolygonMode* polyModeObj = getOrCreatePolygonMode();

    osg::PolygonMode::Mode currentMode = getPolygonMode();
    // cycle through the available modes.  
    switch(currentMode)
    {
        case osg::PolygonMode::FILL : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE); break;
        case osg::PolygonMode::LINE : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::POINT); break;
        case osg::PolygonMode::POINT : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL); break;
    }
}

osg::PolygonMode::Mode StateSetManipulator::getPolygonMode() const
{
    osg::PolygonMode* polyModeObj = dynamic_cast<osg::PolygonMode*>(_drawState->getAttribute(osg::StateAttribute::POLYGONMODE));
    if (polyModeObj) return polyModeObj->getMode(osg::PolygonMode::FRONT_AND_BACK);
    else return osg::PolygonMode::FILL;
}

osg::PolygonMode* StateSetManipulator::getOrCreatePolygonMode()
{
    osg::PolygonMode* polyModeObj = dynamic_cast<osg::PolygonMode*>(_drawState->getAttribute(osg::StateAttribute::POLYGONMODE));
    if (!polyModeObj) 
    {
        polyModeObj = new osg::PolygonMode;
        _drawState->setAttribute(polyModeObj);
    }
    return polyModeObj;
}
