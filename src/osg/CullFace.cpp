#include "osg/GL"
#include "osg/CullFace"
#include "osg/Output"
#include "osg/Input"


using namespace osg;

CullFace::CullFace()
{
    _mode = BACK;
}

CullFace::~CullFace()
{
}

CullFace* CullFace::instance()
{
    static ref_ptr<CullFace> s_CullFace(new CullFace);
    return s_CullFace.get();
}

void CullFace::enable( void )
{
    glEnable( GL_CULL_FACE  );
}


void CullFace::disable( void )
{
    glDisable( GL_CULL_FACE  );
}

void CullFace::apply()
{
    glCullFace((GLenum)_mode);
}


bool CullFace::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    if (fr[0].matchWord("mode"))
    {
        if (fr[1].matchWord("FRONT"))
        {
            _mode = FRONT;
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("BACK"))
        {
            _mode = BACK;
            fr+=2;
            iteratorAdvanced = true;
        }
        else if (fr[1].matchWord("FRONT_AND_BACK"))
        {
            _mode = FRONT_AND_BACK;
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    return iteratorAdvanced;
}


bool CullFace::writeLocalData(Output& fw)
{
    switch(_mode)
    {
    case(FRONT):          fw.indent() << "mode FRONT" << endl; break;
    case(BACK):           fw.indent() << "mode BACK" << endl; break;
    case(FRONT_AND_BACK): fw.indent() << "mode FRONT_AND_BACK" << endl; break;
    }
    return true;
}
