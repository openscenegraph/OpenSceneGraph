#include "osg/OSG"
#include "osg/Transparency"

using namespace osg;

Transparency::Transparency( void )
{
    _source_factor      = SRC_ALPHA;
    _destination_factor = ONE_MINUS_SRC_ALPHA;
}


Transparency::~Transparency( void )
{
}


Transparency* Transparency::instance()
{
    static ref_ptr<Transparency> s_transparency(new Transparency);
    return s_transparency.get();
}

void Transparency::enable( void )
{
    glEnable( GL_BLEND );
}


void Transparency::disable( void )
{
    glDisable( GL_BLEND );
}


void Transparency::apply( void )
{
    glBlendFunc( (GLenum)_source_factor, (GLenum)_destination_factor );
}


void Transparency::setFunction( int source, int destination )
{
    _source_factor = source;
    _destination_factor = destination;
}
