#include "osg/GL"
#include "osg/TexMat"

using namespace osg;

TexMat::TexMat( void )
{
}


TexMat::~TexMat( void )
{
}

TexMat* TexMat::instance()
{
    static ref_ptr<TexMat> s_texmat(new TexMat);
    return s_texmat.get();
}

void TexMat::apply( void )
{
    glMatrixMode( GL_TEXTURE );
    glLoadMatrixf( (GLfloat *)_mat );
}
