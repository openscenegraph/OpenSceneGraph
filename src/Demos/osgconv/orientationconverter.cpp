#include <stdio.h>
#include "OrientationConverter.h"

using namespace osg;

OrientationConverter::OrientationConverter( void )
{
    _mat.makeIdent();
}

void OrientationConverter::setConversion( const Vec3 &from, const Vec3 &to )
{
    // This is the default OpenGL UP vector
    Vec3 A( 0, 1, 0 );

    // if from is not equal to the default, we must first rotate beteen A and from
    if( A != from )
    {
   	;
    }

    // Now do a rotation between A and to

    Quat q;
    q.makeRot( A, to );
    q.get( _mat );
}


void OrientationConverter::convert( Node &node )
{
    _cv.apply( node );
}


void OrientationConverter::ConvertVisitor::apply( Geode &geode )
{
   int numdrawables = geode.getNumDrawables();

   for( int i = 0; i < numdrawables; i++ )
   {
       Drawable *dbl = geode.getDrawable( i );

       if( dbl->isSameKindAs

   }
}
