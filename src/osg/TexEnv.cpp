#include "osg/TexEnv"
#include "osg/Input"
#include "osg/Output"

using namespace osg;

TexEnv::TexEnv( void )
{
    _mode = MODULATE;
}

TexEnv::~TexEnv( void )
{
}

TexEnv* TexEnv::instance()
{
    static ref_ptr<TexEnv> s_TexEnv(new TexEnv);
    return s_TexEnv.get();
}


void TexEnv::setMode( TexEnvMode mode )
{
    _mode = (mode == DECAL ||
             mode == MODULATE ||
             mode == BLEND ) ?
             mode : MODULATE;
}

void TexEnv::apply( void )
{
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _mode);
}

bool TexEnv::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;
    TexEnvMode mode;
    if (fr[0].matchWord("mode") && matchModeStr(fr[1].getStr(),mode))
    {
        _mode = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool TexEnv::matchModeStr(const char* str,TexEnvMode& mode)
{
    if (strcmp(str,"DECAL")==0) mode = DECAL;
    else if (strcmp(str,"MODULATE")==0) mode = MODULATE;
    else if (strcmp(str,"BLEND")==0) mode = BLEND;
    else return false;
    return true;
}


const char* TexEnv::getModeStr(TexEnvMode mode)
{
    switch(mode)
    {
        case(DECAL): return "DECAL";
        case(MODULATE): return "MODULATE";
        case(BLEND): return "BLEND";
    }
    return "";
}


bool TexEnv::writeLocalData(Output& fw)
{
    fw.indent() << "mode " << getModeStr(_mode) << endl;

    return true;
}
