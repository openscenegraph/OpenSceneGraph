#include <stdio.h>
#include <osg/GeoSet>
#include "OrientationConverter.h"

using namespace osg;

class TransformFunctor : public osg::Drawable::AttributeFunctor
{

    public:
    
        osg::Matrix _m;

        TransformFunctor(const osg::Matrix& m):
            AttributeFunctor(osg::Drawable::COORDS|osg::Drawable::NORMALS),
            _m(m) {}
            
        virtual ~TransformFunctor() {}

        virtual bool apply(osg::Drawable::AttributeBitMask abm,osg::Vec3* begin,osg::Vec3* end)
        {
            if (abm == osg::Drawable::COORDS)
            {
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    (*itr) = (*itr)*_m;
                }
                return true;
            }
            else if (abm == osg::Drawable::NORMALS)
            {
                for (osg::Vec3* itr=begin;itr<end;++itr)
                {
                    // note post mult rather than pre mult of value.
                    (*itr) = osg::Matrix::transform3x3(_m,(*itr));
                    (*itr).normalize();
                }
                return true;
            }
            return false;

        }

};


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
   
   TransformFunctor tf(_mat);

   // We assume all Drawables are GeoSets ?!!?
   for( int i = 0; i < numdrawables; i++ )
   {
        geode.getDrawable(i)->applyAttributeOperation(tf);

/*
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
*/
   }
}
