#include <stdio.h>
#include <osg/GeoSet>
#include "OrientationConverter.h"

using namespace osg;

OrientationConverter::OrientationConverter( void )
{
}

void OrientationConverter::setConversion( const Vec3 &from, const Vec3 &to )
{
    Quat q;
    Matrix M;

    q.makeRot( from, to );
    q.get( M );

    _cv.setMatrix( M );
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

	if( gset == NULL )
	    continue;

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
