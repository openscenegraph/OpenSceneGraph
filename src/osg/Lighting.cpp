#include "osg/GL"
#include "osg/Lighting"

using namespace osg;

void Lighting::enable( void )
{
    glEnable( GL_LIGHTING );
}


void Lighting::disable( void )
{
    glDisable( GL_LIGHTING );
}
