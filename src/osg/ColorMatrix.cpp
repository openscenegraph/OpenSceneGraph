#include <osg/GL>
#include <osg/ColorMatrix>

using namespace osg;

ColorMatrix::ColorMatrix()
{
}


ColorMatrix::~ColorMatrix()
{
}

void ColorMatrix::apply(State&) const
{
//    std::cout<<"applying matrix"<<_matrix<<std::endl;

    glMatrixMode( GL_COLOR );
    glLoadMatrixf( _matrix.ptr() );
    glMatrixMode( GL_MODELVIEW );
}
