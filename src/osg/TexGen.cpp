
#include "osg/TexGen"
#include "osg/Input"
#include "osg/Output"

using namespace osg;

TexGen::TexGen( void )
{
    _mode = OBJECT_LINEAR;
}


TexGen::~TexGen( void )
{
}


TexGen* TexGen::instance()
{
    static ref_ptr<TexGen> s_texgen(new TexGen);
    return s_texgen.get();
}


bool TexGen::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;
    TexGenMode mode;
    if (fr[0].matchWord("mode") && matchModeStr(fr[1].getStr(),mode))
    {
        _mode = mode;
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool TexGen::matchModeStr(const char* str,TexGenMode& mode)
{
    if (strcmp(str,"EYE_LINEAR")==0) mode = EYE_LINEAR;
    else if (strcmp(str,"OBJECT_LINEAR")==0) mode = OBJECT_LINEAR;
    else if (strcmp(str,"SPHERE_MAP")==0) mode = SPHERE_MAP;
    else return false;
    return true;
}


const char* TexGen::getModeStr(TexGenMode mode)
{
    switch(mode)
    {
        case(EYE_LINEAR): return "EYE_LINEAR";
        case(OBJECT_LINEAR): return "OBJECT_LINEAR";
        case(SPHERE_MAP): return "SPHERE_MAP";
    }
    return "";
}


bool TexGen::writeLocalData(Output& fw)
{
    fw.indent() << "mode " << getModeStr(_mode) << endl;

    return true;
}


void TexGen::enable( void )
{
    glEnable( GL_TEXTURE_GEN_S );
    glEnable( GL_TEXTURE_GEN_T );
}


void TexGen::disable( void )
{
    glDisable( GL_TEXTURE_GEN_S );
    glDisable( GL_TEXTURE_GEN_T );
}


void TexGen::apply( void )
{
    glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, _mode );
    glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, _mode );
}
