#include <stdio.h>

#include <osg/GeoSet>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/LOD>

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
    node.accept(_cv);
}


void OrientationConverter::ConvertVisitor::apply(osg::Geode& geode)
{
    for(int i=0;i<geode.getNumDrawables();++i)
    {
        geode.getDrawable(i)->applyAttributeOperation(_tf);
    }
}

void OrientationConverter::ConvertVisitor::apply(osg::Billboard& billboard)
{
    osg::Vec3 axis = osg::Matrix::transform3x3(_tf._im,billboard.getAxis());
    billboard.setAxis(axis);

    for(int i=0;i<billboard.getNumDrawables();++i)
    {
        billboard.setPos(i,billboard.getPos(i)*_tf._m);
        billboard.getDrawable(i)->applyAttributeOperation(_tf);
    }
}

void OrientationConverter::ConvertVisitor::apply(osg::LOD& lod)
{
    lod.setCenter(lod.getCenter()*_tf._m);
    traverse(lod);
}
