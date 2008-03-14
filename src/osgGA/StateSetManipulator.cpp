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

StateSetManipulator::StateSetManipulator(osg::StateSet* stateset):
    _initialized(false),
    _backface(false),
    _lighting(false),
    _texture(false),
    _maxNumOfTextureUnits(4),
    _keyEventToggleBackfaceCulling('b'),
    _keyEventToggleLighting('l'),
    _keyEventToggleTexturing('t'),
    _keyEventCyclePolygonMode('w')
{
    setStateSet(stateset);
}

StateSetManipulator::~StateSetManipulator()
{
}

void StateSetManipulator::setStateSet(StateSet *stateset)
{
    _stateset = stateset;
#if 0
    // specify that this stateset is dynamic so it prevents 
    // the draw and update phase from overlapping - good for 
    // stability but breaks all the performance advantage of 
    // DrawThreadPerContex.
    _stateset->setDataVariance(osg::Object::DYNAMIC);
#endif
}

StateSet *StateSetManipulator::getStateSet()
{
    return _stateset.get();
}

const StateSet *StateSetManipulator::getStateSet() const
{
    return _stateset.get();
}

void StateSetManipulator::clone()
{
    if (!_stateset) return;
    
    // we clone the StateSet so that any draw traversals that might be running at the time of the
    // event traversal won't change the same StateSet that is being read.  One could just set the 
    // DataVariance to DYNAMIC to avoid this overlap, but this would introduce a performance penalty.
    
    StateSet::ParentList parents = _stateset->getParents();
    osg::ref_ptr<osg::StateSet> newStateSet = dynamic_cast<osg::StateSet*>(_stateset->clone(osg::CopyOp::SHALLOW_COPY));

    // change the parents of the original StateSet to point to the new stateset
    for(StateSet::ParentList::iterator itr = parents.begin();
        itr !=  parents.end();
        ++itr)
    {
        osg::Object* object = *itr;
        osg::Node* node = dynamic_cast<osg::Node*>(object);
        if (node) node->setStateSet(newStateSet.get());
        else
        {
            osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(object);
            if (drawable) drawable->setStateSet(newStateSet.get());
        }
    }
    
    _stateset = newStateSet;
}

bool StateSetManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    if(!_stateset.valid()) return false;

    if (!_initialized)
    {
        _initialized = true;
        _backface = (_stateset->getMode(GL_CULL_FACE)&osg::StateAttribute::ON);
        _lighting =(_stateset->getMode(GL_LIGHTING)&osg::StateAttribute::ON);

        unsigned int mode = osg::StateAttribute::INHERIT|osg::StateAttribute::ON;
        _texture = (_stateset->getTextureMode(0,GL_TEXTURE_1D)&mode) ||
                   (_stateset->getTextureMode(0,GL_TEXTURE_2D)&mode) ||
                   (_stateset->getTextureMode(0,GL_TEXTURE_3D)&mode) ||
                   (_stateset->getTextureMode(0,GL_TEXTURE_RECTANGLE)&mode) ||
                   (_stateset->getTextureMode(0,GL_TEXTURE_CUBE_MAP)&mode);
    }

    if (ea.getHandled()) return false;

    if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
    {

        if ( ea.getKey() == _keyEventToggleBackfaceCulling )
        {
            setBackfaceEnabled(!getBackfaceEnabled());
            aa.requestRedraw();
            return true;
        }
        if ( ea.getKey() == _keyEventToggleLighting )
        {
                setLightingEnabled(!getLightingEnabled());
                aa.requestRedraw();
                return true;
        }
        if ( ea.getKey() == _keyEventToggleTexturing )
        {
                setTextureEnabled(!getTextureEnabled());
                aa.requestRedraw();
                return true;
        }
        if ( ea.getKey() == _keyEventCyclePolygonMode )
        {
                cyclePolygonMode();
                aa.requestRedraw();
                return true;
        }
    }

    return false;
}

void StateSetManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding(reinterpret_cast<const char*>(&_keyEventToggleBackfaceCulling),"Toggle backface culling");
    usage.addKeyboardMouseBinding(reinterpret_cast<const char*>(&_keyEventToggleLighting),"Toggle lighting");
    usage.addKeyboardMouseBinding(reinterpret_cast<const char*>(&_keyEventToggleTexturing),"Toggle texturing");
    usage.addKeyboardMouseBinding(reinterpret_cast<const char*>(&_keyEventCyclePolygonMode),"Toggle polygon fill mode between fill, line (wire frame) and points");
}


void StateSetManipulator::setBackfaceEnabled(bool newbackface)
{
    if (_backface == newbackface) return;
    
    clone();
    
    _backface = newbackface;
    if( _backface ) _stateset->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
    else _stateset->setMode(GL_CULL_FACE,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
}

void StateSetManipulator::setLightingEnabled(bool newlighting)
{
    if (_lighting == newlighting) return;
    
    clone();

    _lighting = newlighting;
    if( _lighting ) _stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    else _stateset->setMode(GL_LIGHTING,osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
}

void StateSetManipulator::setTextureEnabled(bool newtexture)
{
    if (_texture==newtexture) return;
    
    clone();

    _texture = newtexture;
//    osg::ref_ptr< osg::Texture > tex = dynamic_cast<osg::Texture*>
//        ( _stateset->getAttribute( osg::StateAttribute::TEXTURE ) );
//    cout << tex->numTextureUnits() << endl;

    unsigned int mode = osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF;
    if ( _texture ) mode = osg::StateAttribute::INHERIT|osg::StateAttribute::ON;
    for( unsigned int ii=0; ii<_maxNumOfTextureUnits; ii++ )
    {
            _stateset->setTextureMode( ii, GL_TEXTURE_1D, mode );
            _stateset->setTextureMode( ii, GL_TEXTURE_2D, mode );
            _stateset->setTextureMode( ii, GL_TEXTURE_3D, mode );
            _stateset->setTextureMode( ii, GL_TEXTURE_RECTANGLE, mode );
            _stateset->setTextureMode( ii, GL_TEXTURE_CUBE_MAP, mode);
    }
}

void StateSetManipulator::setPolygonMode(osg::PolygonMode::Mode newpolygonmode)
{
    clone();

    osg::PolygonMode* polyModeObj = getOrCreatePolygonMode();

    polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,newpolygonmode);
}

void StateSetManipulator::cyclePolygonMode()
{
    clone();

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
    osg::PolygonMode* polyModeObj = dynamic_cast<osg::PolygonMode*>(_stateset->getAttribute(osg::StateAttribute::POLYGONMODE));
    if (polyModeObj) return polyModeObj->getMode(osg::PolygonMode::FRONT_AND_BACK);
    else return osg::PolygonMode::FILL;
}

osg::PolygonMode* StateSetManipulator::getOrCreatePolygonMode()
{
    osg::PolygonMode* polyModeObj = dynamic_cast<osg::PolygonMode*>(_stateset->getAttribute(osg::StateAttribute::POLYGONMODE));
    if (!polyModeObj) 
    {
        polyModeObj = new osg::PolygonMode;
        _stateset->setAttribute(polyModeObj);
    }
    return polyModeObj;
}
