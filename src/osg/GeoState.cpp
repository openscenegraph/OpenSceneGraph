#include <stdio.h>

#include "osg/GeoState"
#include "osg/Input"
#include "osg/Output"
#include "osg/Notify"

using namespace osg;

GeoState::GeoState()
{

    _transparencing     = INHERIT;
    _face_culling       = INHERIT;
    _lighting           = INHERIT;
    _texturing          = INHERIT;
    _fogging            = INHERIT;
    _texgening          = INHERIT;
    _antialiasing       = INHERIT;
    _colortable         = INHERIT;
    _pointSmoothing     = INHERIT;
    _polygonOffsetting  = INHERIT;
    _alphaTesting       = INHERIT;

    _transparency   = 0L;
    _cullFace       = 0L;
    _texture        = 0L;
    _fog            = 0L;
    _texgen         = 0L;
    _material       = 0L;
    _texenv         = 0L;
    _texmat         = 0L;
    _point          = 0L;
    _polygonOffset  = 0L;
    _alphaFunc      = 0L;
}


GeoState::~GeoState()
{
    // note, all attached state attributes will be automatically
    // unreferenced by ref_ptr<> and therefore there is now need to
    // delete the memory manually.
}


GeoState* GeoState::instance()
{
    static ref_ptr<GeoState> s_geostate(new GeoState);
    return s_geostate.get();
}

void GeoState::setGlobalDefaults()
{
    _transparencing = OFF;
    _face_culling = ON;
    _lighting = OFF;
    _texturing = OFF;
    _fogging = OFF;
    _texgening = OFF;
    _antialiasing = OFF;
    _colortable = OFF;
    _pointSmoothing = OFF;
    _polygonOffsetting = OFF;
    _alphaTesting = OFF;

    _transparency   = new Transparency;
    _cullFace       = new CullFace;
    _texture        = 0L;
    _fog            = 0L;
    _texgen         = 0L;
    _material       = new Material;
    _material->setColorMode(Material::AMBIENT_AND_DIFFUSE);
    _texenv         = 0L;
    _texmat         = 0L;
    _point          = 0L;
    _polygonOffset  = 0L;
    _alphaFunc      = new AlphaFunc;

}

void GeoState::setAllToInherit()
{
    _transparencing = INHERIT;
    _face_culling = INHERIT;
    _lighting = INHERIT;
    _texturing = INHERIT;
    _fogging = INHERIT;
    _texgening = INHERIT;
    _antialiasing = INHERIT;
    _colortable = INHERIT;
    _pointSmoothing     = INHERIT;
    _polygonOffsetting  = INHERIT;
    _alphaTesting  = INHERIT;

    _transparency   = 0L;
    _cullFace       = 0L;
    _texture        = 0L;
    _fog            = 0L;
    _texgen         = 0L;
    _material       = 0L;
    _texenv         = 0L;
    _texmat         = 0L;
    _point          = 0L;
    _polygonOffset  = 0L;
    _alphaFunc      = 0L;
}

bool GeoState::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    AttributeMode mode;
    if (fr[0].matchWord("transparency") && matchModeStr(fr[1].getStr(),mode))
    {
        _transparencing = mode;
        fr+=2;
        iteratorAdvanced = true;
    }
    if (Transparency* transTmp = static_cast<Transparency*>(Transparency::instance()->readClone(fr)))
    {
        _transparency = transTmp;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("antialiasing") && matchModeStr(fr[1].getStr(),mode))
    {
        _antialiasing = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("face_culling") && matchModeStr(fr[1].getStr(),mode))
    {
        _face_culling = mode;
        fr+=2;
        iteratorAdvanced = true;
    }
    if (CullFace* cullTmp = static_cast<CullFace*>(CullFace::instance()->readClone(fr)))
    {
        _cullFace = cullTmp;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("lighting") && matchModeStr(fr[1].getStr(),mode))
    {
        _lighting = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("texturing") && matchModeStr(fr[1].getStr(),mode))
    {
        _texturing = mode;
        fr+=2;
        iteratorAdvanced = true;
    }
    if (Texture* textTmp = static_cast<Texture*>(Texture::instance()->readClone(fr)))
    {
        _texture = textTmp;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("fogging") && matchModeStr(fr[1].getStr(),mode))
    {
        _fogging = mode;
        fr+=2;
        iteratorAdvanced = true;
    }
    if (Fog* fogTmp = static_cast<Fog*>(Fog::instance()->readClone(fr)))
    {
        _fog = fogTmp;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("colortable") && matchModeStr(fr[1].getStr(),mode))
    {
        _colortable = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("texgening") && matchModeStr(fr[1].getStr(),mode))
    {
        _texgening = mode;
        fr+=2;
        iteratorAdvanced = true;
    }
    if (TexGen* tmpTexgen = static_cast<TexGen*>(TexGen::instance()->readClone(fr)))
    {
        _texgen = tmpTexgen;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("point_smoothing") && matchModeStr(fr[1].getStr(),mode))
    {
        _pointSmoothing = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (Point* tmpPoint = static_cast<Point*>(Point::instance()->readClone(fr)))
    {
        _point = tmpPoint;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("polygon_offset") && matchModeStr(fr[1].getStr(),mode))
    {
        _polygonOffsetting = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (PolygonOffset* tmpPolygonOffset = static_cast<PolygonOffset*>(PolygonOffset::instance()->readClone(fr)))
    {
        _polygonOffset = tmpPolygonOffset;
        iteratorAdvanced = true;
    }


    if (fr[0].matchWord("alpha_test") && matchModeStr(fr[1].getStr(),mode))
    {
        _alphaTesting = mode;
        fr+=2;
        iteratorAdvanced = true;
    }
    if (AlphaFunc* tmpAlphaFunc = static_cast<AlphaFunc*>(AlphaFunc::instance()->readClone(fr)))
    {
        _alphaFunc = tmpAlphaFunc;
        iteratorAdvanced = true;
    }


    if (Material* tmpMaterial = static_cast<Material*>(Material::instance()->readClone(fr)))
    {
        _material = tmpMaterial;
        iteratorAdvanced = true;
    }


/*
 * Decided that texmatting does not make sense.  If a Texture matrix
 * is present, just apply it.  No need for a mode. Don.
 *
    if (fr[0].matchWord("texmating") && matchModeStr(fr[1].getStr(),mode)) {
        _texmating = mode;
        fr+=2;
        iteratorAdvanced = true;
    }
*/

    return iteratorAdvanced;
}


bool GeoState::writeLocalData(Output& fw)
{
    fw.indent() << "transparency " << getModeStr(_transparencing) << endl;
    if (_transparency.valid()) _transparency->write(fw);

    fw.indent() << "antialiasing " << getModeStr(_antialiasing) << endl;

    fw.indent() << "face_culling " << getModeStr(_face_culling) << endl;
    if (_cullFace.valid()) _cullFace->write(fw);

    fw.indent() << "lighting " << getModeStr(_lighting) << endl;

    fw.indent() << "texturing " << getModeStr(_texturing) << endl;
    if (_texture.valid()) _texture->write(fw);

    fw.indent() << "fogging " << getModeStr(_fogging) << endl;
    if (_fog.valid()) _fog->write(fw);

    fw.indent() << "colortable " << getModeStr(_colortable) << endl;

    fw.indent() << "texgening " << getModeStr(_texgening) << endl;
    if (_texgen.valid()) _texgen->write(fw);

    if (_texenv.valid()) _texenv->write(fw);

    fw.indent() << "point_smoothing " << getModeStr(_pointSmoothing) << endl;
    if (_point.valid()) _point->write(fw);

    fw.indent() << "polygon_offset " << getModeStr(_polygonOffsetting) << endl;
    if (_polygonOffset.valid()) _polygonOffset->write(fw);

    fw.indent() << "alpha_test " << getModeStr(_alphaTesting) << endl;
    if (_alphaFunc.valid()) _alphaFunc->write(fw);

    if (_material.valid()) _material->write(fw);

/*
    fw.indent() << "texmating " << getModeStr(_texmating) << endl;
*/

    return true;
}

bool GeoState::matchModeStr(const char* str,AttributeMode& mode)
{
    if (strcmp(str,"INHERIT")==0) mode = INHERIT;
    else if (strcmp(str,"ON")==0) mode = ON;
    else if (strcmp(str,"OFF")==0) mode = OFF;
    else if (strcmp(str,"OVERRIDE_ON")==0) mode = OVERRIDE_ON;
    else if (strcmp(str,"OVERRIDE_OFF")==0) mode = OVERRIDE_OFF;
    else return false;
    return true;
}


const char* GeoState::getModeStr(AttributeMode mode)
{
    switch(mode)
    {
        case(INHERIT): return "INHERIT";
        case(ON): return "ON";
        case(OFF): return "OFF";
        case(OVERRIDE_ON): return "OVERRIDE_ON";
        case(OVERRIDE_OFF): return "OVERRIDE_OFF";
    }
    return "";
}


void GeoState::setMode( AttributeType type, AttributeMode mode )
{
    switch( type )
    {
        case ANTIALIAS  : _antialiasing = mode; break;
        case FACE_CULL: _face_culling       = mode; break;
        case FOG     : _fogging   = mode; break;
        case LIGHTING: _lighting  = mode; break;
        case POINT   : _pointSmoothing = mode; break;
        case POLYGON_OFFSET : _polygonOffsetting = mode; break;
        case TEXGEN  : _texgening = mode; break;
        case TEXTURE : _texturing = mode; break;
        case TRANSPARENCY: _transparencing  = mode; break;
        case ALPHAFUNC: _alphaTesting  = mode; break;
        default : notify(WARN) << "GeoState::setMode("<<(int)type<<","<<(int)mode<<") not handled."<<endl;
    }
}

GeoState::AttributeMode GeoState::getMode( AttributeType type) const
{
    switch( type )
    {
        case ANTIALIAS  : return _antialiasing;
        case FACE_CULL: return _face_culling;
        case FOG     : return _fogging;
        case LIGHTING: return _lighting;
        case POINT   : return _pointSmoothing;
        case POLYGON_OFFSET : return _polygonOffsetting;
        case TEXTURE : return _texturing;
        case TEXGEN  : return _texgening;
        case TRANSPARENCY: return _transparencing;
        case ALPHAFUNC: return _alphaTesting;
        default : notify(WARN) << "GeoState::getMode("<<(int)type<<") not handled."<<endl;
    }
    return INHERIT;
}

void GeoState::setAttribute( AttributeType type, Object *attribute )
{
    switch( type )
    {
        case FACE_CULL : _cullFace          = dynamic_cast<CullFace*>(attribute); break;
        case FOG     : _fog                 = dynamic_cast<Fog*>(attribute); break;
        case LIGHTING:  break;            /*_light   = (Light   *)attribute;*/
        case MATERIAL: _material            = dynamic_cast<Material*>(attribute); break;
        case POINT   : _point               = dynamic_cast<Point*>(attribute); break;
        case POLYGON_OFFSET: _polygonOffset = dynamic_cast<PolygonOffset*>(attribute); break;
        case TEXENV  : _texenv              = dynamic_cast<TexEnv*>(attribute); break;
        case TEXGEN  : _texgen              = dynamic_cast<TexGen*>(attribute); break;
        case TEXMAT  : _texmat              = dynamic_cast<TexMat*>(attribute); break;
        case TEXTURE : _texture             = dynamic_cast<Texture*>(attribute); break;
        case TRANSPARENCY: _transparency    = dynamic_cast<Transparency*>(attribute); break;
        case ALPHAFUNC: _alphaFunc          = dynamic_cast<AlphaFunc*>(attribute); break;
        default : notify(WARN) << "GeoState::setAttribute("<<(int)type<<","<<attribute<<") not handled."<<endl;
    }

}

Object* GeoState::getAttribute( AttributeType type) const
{
    switch( type )
    {
        case FOG     : return _fog.get();
        case LIGHTING: return NULL; /*_light*/
        case MATERIAL: return _material.get();
        case POINT   : return _point.get();
        case POLYGON_OFFSET: return _polygonOffset.get();
        case TEXENV  : return _texenv.get();
        case TEXGEN  : return _texgen.get();
        case TEXMAT  : return _texmat.get();
        case TEXTURE : return _texture.get();
        case TRANSPARENCY: return _transparency.get();
        case ALPHAFUNC: return _alphaFunc.get();
        default : notify(WARN) << "GeoState::getAttribute("<<(int)type<<") not handled."<<endl;
    }
    return NULL;
}

void GeoState::apply()
{
    switch(_transparencing)
    {
        case(ON):
        case(OVERRIDE_ON):
            Transparency::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Transparency::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(_face_culling)
    {
        case(ON):
        case(OVERRIDE_ON):
            CullFace::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            CullFace::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(_lighting)
    {
        case(ON):
        case(OVERRIDE_ON):
            Lighting::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Lighting::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(_texturing)
    {
        case(ON):
        case(OVERRIDE_ON):
            Texture::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Texture::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(_texgening)
    {
        case(ON):
        case(OVERRIDE_ON):
            TexGen::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            TexGen::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(_fogging)
    {
        case(ON):
        case(OVERRIDE_ON):
            Fog::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Fog::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(_pointSmoothing)
    {
        case(ON):
        case(OVERRIDE_ON):
            Point::enableSmooth();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Point::disableSmooth();
            break;
        case(INHERIT):
            break;
    }

    switch(_polygonOffsetting)
    {
        case(ON):
        case(OVERRIDE_ON):
            PolygonOffset::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            PolygonOffset::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(_alphaTesting)
    {
        case(ON):
        case(OVERRIDE_ON):
            AlphaFunc::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            AlphaFunc::disable();
            break;
        case(INHERIT):
            break;
    }

    if( _transparency.valid())
        _transparency->apply();

    if( _cullFace.valid())
        _cullFace->apply();

    if( _texenv.valid())
        _texenv->apply();

    if( _texgen.valid())
        _texgen->apply();

    if( _texture.valid())
        _texture->apply();

    if( _material.valid())
        _material->apply();

    if( _fog.valid())
        _fog->apply();

    if( _texmat.valid())
        _texmat->apply();

    if( _point.valid())
        _point->apply();

    if( _polygonOffset.valid())
        _polygonOffset->apply();

    if( _alphaFunc.valid())
        _alphaFunc->apply();
}



void GeoState::apply(GeoState* global,GeoState* prev)
{
    if (global==NULL || prev==NULL)
    {
        apply();
        return;
    }

    switch(GeoState::combineMode(global->_transparencing,prev->_transparencing,_transparencing))
    {
        case(ON):
        case(OVERRIDE_ON):
            Transparency::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Transparency::disable();
            break;
        case(INHERIT):
            break;
    }


    switch(GeoState::combineMode(global->_face_culling,prev->_face_culling,_face_culling))
    {
        case(ON):
        case(OVERRIDE_ON):
            CullFace::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            CullFace::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(GeoState::combineMode(global->_lighting,prev->_lighting,_lighting))
    {
        case(ON):
        case(OVERRIDE_ON):
            Lighting::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Lighting::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(GeoState::combineMode(global->_texturing,prev->_texturing,_texturing))
    {
        case(ON):
        case(OVERRIDE_ON):
            Texture::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Texture::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(GeoState::combineMode(global->_texgening,prev->_texgening,_texgening))
    {
        case(ON):
        case(OVERRIDE_ON):
            TexGen::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            TexGen::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(GeoState::combineMode(global->_fogging,prev->_fogging,_fogging))
    {
        case(ON):
        case(OVERRIDE_ON):
            Fog::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Fog::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(GeoState::combineMode(global->_pointSmoothing,prev->_pointSmoothing,_pointSmoothing))
    {
        case(ON):
        case(OVERRIDE_ON):
            Point::enableSmooth();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            Point::disableSmooth();
            break;
        case(INHERIT):
            break;
    }

    switch(GeoState::combineMode(global->_polygonOffsetting,prev->_polygonOffsetting,_polygonOffsetting))
    {
        case(ON):
        case(OVERRIDE_ON):
            PolygonOffset::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            PolygonOffset::disable();
            break;
        case(INHERIT):
            break;
    }

    switch(GeoState::combineMode(global->_alphaTesting,prev->_alphaTesting,_alphaTesting))
    {
        case(ON):
        case(OVERRIDE_ON):
            AlphaFunc::enable();
            break;
        case(OFF):
        case(OVERRIDE_OFF):
            AlphaFunc::disable();
            break;
        case(INHERIT):
            break;
    }

    if (prev->_transparency!=_transparency)
    {
        osg::Transparency* new_transparency;
        if (_transparency.valid()) new_transparency = _transparency.get();
        else new_transparency = global->_transparency.get();
        if (new_transparency) new_transparency->apply();
    }

    if (prev->_cullFace!=_cullFace)
    {
        osg::CullFace* new_cullFace;
        if (_cullFace.valid()) new_cullFace = _cullFace.get();
        else new_cullFace = global->_cullFace.get();
        if (new_cullFace) new_cullFace->apply();
    }

    if (prev->_texenv!=_texenv)
    {
        osg::TexEnv* new_texenv;
        if (_texenv.valid()) new_texenv = _texenv.get();
        else new_texenv = global->_texenv.get();
        if (new_texenv) new_texenv->apply();
    }

    if (prev->_texgen!=_texgen)
    {
        osg::TexGen* new_texgen;
        if (_texgen.valid()) new_texgen = _texgen.get();
        else new_texgen = global->_texgen.get();
        if (new_texgen) new_texgen->apply();
    }

    if (prev->_texture!=_texture)
    {
        osg::Texture* new_texture;
        if (_texture.valid()) new_texture = _texture.get();
        else new_texture = global->_texture.get();
        if (new_texture) new_texture->apply();
    }

    if (prev->_material!=_material)
    {
        osg::Material* new_material;
        if (_material.valid()) new_material = _material.get();
        else new_material = global->_material.get();
        if (new_material) new_material->apply();
    }

    if (prev->_fog!=_fog)
    {
        osg::Fog* new_fog;
        if (_fog.valid()) new_fog = _fog.get();
        else new_fog = global->_fog.get();
        if (new_fog) new_fog->apply();
    }

    if (prev->_texmat!=_texmat)
    {
        osg::TexMat* new_texmat;
        if (_texmat.valid()) new_texmat = _texmat.get();
        else new_texmat = global->_texmat.get();
        if (new_texmat) new_texmat->apply();
    }

    if (prev->_point!=_point)
    {
        osg::Point* new_point;
        if (_point.valid()) new_point = _point.get();
        else new_point = global->_point.get();
        if (new_point) new_point->apply();
    }

    if (prev->_polygonOffset!=_polygonOffset)
    {
        osg::PolygonOffset* new_polygonOffset;
        if (_polygonOffset.valid()) new_polygonOffset = _polygonOffset.get();
        else new_polygonOffset = global->_polygonOffset.get();
        if (new_polygonOffset) new_polygonOffset->apply();
    }

    if (prev->_alphaFunc!=_alphaFunc)
    {
        osg::AlphaFunc* new_AlphaFunc;
        if (_alphaFunc.valid()) new_AlphaFunc = _alphaFunc.get();
        else new_AlphaFunc = global->_alphaFunc.get();
        if (new_AlphaFunc) new_AlphaFunc->apply();
    }
}

bool GeoState::check()
{
    return true;
}


/*
#if 0 // [  NOTES on how apply should work

Each Scene must have a global initial GeoState, current GeoState, and current request state.  The current definition of GeoState
should really become an GeoState.  The global initial State can have modes set to

    SG_ON,
    SG_OFF,

and may be or'ed with

SG_OVERRIDE.

All attributes are set to the default.  Defaults can be set at start up by querying hardware
and determining best parameters (for example, do we have hardware texture mapping?, if so enable texture and create a texture environment).

The current GeoState and the request GeoState begin as a copy of the initial  state. The request state is subsequently changed by each GeoState,
during traversal.  It is the current state that will actually issue the GL commands at apply time.

Attributes for the GeoStates  may be

SG_ON  -explicitely
SG_OFF -explicitely
SG_INHERIT

and may be or'ed with

SG_PERSIST

During traversal, each GeoState's attribute set to INHERIT does nothing. Each attribute set to ON or OFF sets the subsequent request state
to the same before drawing and unsets it after.  If the attribute is or'ed with SG_PERSIST, then the mode is not unset after drawing.
Just before drawing the request state is compared to the current state.  If an attribute or mode has changed, it is changed in the current
state then applied.  Only at this application will the actual GL calls be issued.

For exammple, if two subsequent geosets have lighting on the sequence will be as follows

geostate 1 sets lighting on in request state
at draw:
if current state has lighting off it is changed to on and applied.
geostate1 unsets lighting in request state.

geosate2 resets lighting to on in request state
at draw:
current state has lighting set to on from previous draw therefore nothing changes

geostate2

Addendum 10/22 - Use this method for traversal.  Currently we shall do dumb apply().
Upon implementation of a CULL traversal, which creates a DRAW display list, then we
shall implement the "smart" state change described above.

#endif // ]
*/
