#include <stdio.h>
#include <osg/GeoSet>
#include "OrientationConverter.h"

using namespace osg;

OrientationConverter::OrientationConverter( void )
{
    //_mat.makeIdent();
}

void OrientationConverter::setConversion( const Vec3 &from, const Vec3 &to )
{
    Quat q;
    Matrix mat;
    // This is the default OpenGL UP vector
    Vec3 A( 0, 1, 0 );

    // if from is not equal to the default, we must first rotate beteen A and from
    if( A != from )
    {
	Vec3 aa = A;
        q.makeRot( A, from );
	q.get( mat );

	A = aa * mat;
    }

    // Now do a rotation between A and to

    q.makeRot( A, to );
    q.get( mat );
    _cv.setMatrix( mat );

}


void OrientationConverter::convert( Node &node )
{
    _cv.apply( node );
}


void OrientationConverter::ConvertVisitor::apply( Geode &geode )
{
   int numdrawables = geode.getNumDrawables();

   // We assume all Drawables are GeoSets ?!!?
   for( int i = 0; i < numdrawables; i++ )
   {
   	GeoSet *gset = dynamic_cast<GeoSet *>(geode.getDrawable(i));
	int numcoords = gset->getNumCoords();
	Vec3 *vertex = gset->getCoords();

	for( int i = 0; i < numcoords; i++ )
	{
	    Vec3 vv = vertex[i];
	    vertex[i] = vv * _mat;
	}

	int numnormals = gset->getNumNormals();
	Vec3 *normals = gset->getNormals();
	for( int i = 0; i < numnormals; i++ )
	{
	    Vec3 vv = normals[i];
	    normals[i] = vv * _mat;
	}

   }
}
